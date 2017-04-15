#include <fstream>
#include "depth_camera_cmd.h"

const char Base64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

DepthCameraCmdPort::DepthCameraCmdPort()
{
	mStopUpgradingFlag = false;
	mIsUpgrading = false;
	mUpgradeProgress = 0;
}

DepthCameraCmdPort::~DepthCameraCmdPort()
{

}

bool DepthCameraCmdPort::StartUpgrade(string firmware_file_name)
{
	mUpgradeFuture = std::async(std::launch::async, [this, &firmware_file_name]{
		// reset upgrade status
		mUpgradeProgress = 0;
		mStopUpgradingFlag = false;

		// Open firmware file
		std::ifstream fw_file(firmware_file_name, ios::binary|ios::in);
		if(!fw_file.is_open()){
			mUpgradeProgress = -1;
			return false;
		}
		fw_file.seekg(0, ios::end);
		uint32_t firmware_file_size = fw_file.tellg();
		fw_file.seekg(0, ios::beg);

		// Set upgrade flag
		mIsUpgrading = true;

		// send erase cmd
		const char *erase_cmd = "fwu e\r\n";
		if(SendCmdAndWaitResult(erase_cmd, strlen(erase_cmd), "Ok", 3000) == false){
			mUpgradeProgress = -2;
			return false;
		}

		// read firmware file , encode to base64, send to camera
		const int32_t SINGLE_TX_LEN = 512;
		char *buf_raw = new char[SINGLE_TX_LEN];
		char *buf_base64 = new char[SINGLE_TX_LEN * 2];
		int32_t total_tx_len = 0;
		do{
			fw_file.read(buf_raw, SINGLE_TX_LEN);
			int32_t tx_len = fw_file.gcount();
			if(tx_len > 0){
				int32_t base64_len = Base64Encode(buf_raw, tx_len, buf_base64, SINGLE_TX_LEN * 2 - 2);
				if(base64_len > 0){
					int32_t cmd_len = sprintf(buf_raw, "fwu w 0x%x 0x%x ", total_tx_len, tx_len);
					SendCmd(buf_raw, cmd_len);
					buf_base64[base64_len] = '\r';
					buf_base64[base64_len + 1] = '\n';
					if(SendCmdAndWaitResult(buf_base64, base64_len, "Ok") == false){
						mUpgradeProgress = -3;
						break;
					}
					total_tx_len += tx_len;
					mUpgradeProgress = total_tx_len * 100 / firmware_file_size;
				}else{
					mUpgradeProgress = -4;
					break;
				}
			}
		}while(!fw_file.eof() && !mStopUpgradingFlag);

		delete []buf_raw;
		delete []buf_base64;
		fw_file.close();

		mIsUpgrading = false;
		return mUpgradeProgress == 100 ? true : false;
	});
	return true;
}

bool DepthCameraCmdPort::StopUpgrade()
{
	if(mIsUpgrading){
		mStopUpgradingFlag = false;
		//wait for future result
		return mUpgradeFuture.get();
	}
	return false;
}

int32_t DepthCameraCmdPort::GetUpgradeProgress()
{
	return mUpgradeProgress;
}

bool DepthCameraCmdPort::SetIntegrationTime(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "intg %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::SetExternIlluminatePower(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "isl cd 0x%x\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::SetInternalIlluminatePower(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "inled %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::SetFrameRate(uint16_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "fps %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::SwitchMirror()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "mirror\r\n");
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::SetBinning(uint8_t rows, uint8_t columns)
{
	char cmd[32];
	rows = rows > 3 ? 3 : rows;
	columns = columns > 3 ? 3 : columns;
	int32_t len = snprintf(cmd, sizeof(cmd), "binning %d %d\r\n", rows, columns);
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::RestoreFactorySettings()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "rfs\r\n");
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::SetHdrRatio(uint8_t value) {
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "hdr %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "Ok");
}

bool DepthCameraCmdPort::GetSystemStatus(string &status_str)
{
	char response_buf[2048];
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "show\r\n");
	int32_t res_len = SendCmdAndWaitResponse(cmd, len, 1000, response_buf, sizeof(response_buf) - 1);
	if (res_len > 0) {
		response_buf[res_len] = 0;
		char * str = strstr((char*)response_buf, "show");
		char * endstr = strstr((char*)response_buf, "\r\nINMOTION");
		if(endstr) *endstr = 0;
		if (str){
			status_str = str + len;
			return true;
		}
	}
	return false;
}

bool DepthCameraCmdPort::GetCameraConfig(string &config_str)
{
	char response_buf[2048];
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "camera\r\n");
	int32_t res_len = SendCmdAndWaitResponse(cmd, len, 1000, response_buf, sizeof(response_buf) - 1);
	if (res_len > 0) {
		response_buf[2047] = 0;
		char * str = strstr((char*)response_buf, "Send start:\r\n<");
		char * endstr = strstr((char*)response_buf, ">\r\nSend end.");
		*endstr = 0;
		if (str){
			config_str = str + len;
			return true;
		}
	}

	return false;
}

bool DepthCameraCmdPort::ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t * reg_value)
{
	return false;
}

bool DepthCameraCmdPort::WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t reg_value)
{
	return false;
}

bool DepthCameraCmdPort::SaveConfig()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "save\r\n");
	return SendCmdAndWaitResult(cmd, len, "save");
}

bool DepthCameraCmdPort::SendCmdAndWaitResult(const char * cmd, int32_t cmd_len, const char * result_ok_str, int32_t timeout)
{
	char response_buf[1024];

	int32_t res_len = SendCmdAndWaitResponse(cmd, cmd_len, timeout, response_buf, sizeof(response_buf) - 1);
	if (res_len > 0) {
		response_buf[1023] = 0;
		if (strstr(response_buf, result_ok_str) != NULL)
			return true;
	}

	return false;
}


int32_t DepthCameraCmdPort::Base64Encode(const char *input, size_t input_length, char *out, size_t out_length) {
	int32_t i = 0, j = 0;
	char *out_begin = out;
	unsigned char a3[3];
	unsigned char a4[4];

	size_t encoded_length = (input_length + 2 - ((input_length + 2) % 3)) / 3 * 4;
	if (out_length < encoded_length)
		return -1;

	while (input_length--) {
		a3[i++] = *input++;
		if (i == 3) {
			a4[0] = (a3[0] & 0xfc) >> 2;
			a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
			a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
			a4[3] = (a3[2] & 0x3f);
			for (i = 0; i < 4; i++)
				*out++ = Base64Alphabet[a4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			a3[j] = '\0';

		a4[0] = (a3[0] & 0xfc) >> 2;
		a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
		a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
		a4[3] = (a3[2] & 0x3f);

		for (j = 0; j < i + 1; j++)
			*out++ = Base64Alphabet[a4[j]];

		while ((i++ < 3))
			*out++ = '=';
	}

	return (out == (out_begin + encoded_length)) ? encoded_length : -1;
}