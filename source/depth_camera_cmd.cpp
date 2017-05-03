#include <fstream>
#include "depth_camera_cmd.h"

#ifdef _MSC_VER
#pragma warning(disable:4996) // disable "declared deprecated" warning
#endif

DepthCameraCmdPort::DepthCameraCmdPort()
{
	mStopUpgradingFlag = false;
	mIsUpgrading = false;
	mUpgradeProgress = 0;
}

DepthCameraCmdPort::~DepthCameraCmdPort()
{

}

bool DepthCameraCmdPort::StartUpgrade(string firmware_file_name, string type)
{
	if (mIsUpgrading)
		return false;

	// check if the file is avilable
	std::ifstream fw(firmware_file_name, ios::in);
	if (!fw.is_open()) {
		mUpgradeProgress = -1;
		return false;
	}
	fw.close();

	// start upgrade file in a independent thread
	mUpgradeFuture = std::async(std::launch::async, [this, firmware_file_name, type]{
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
		uint32_t firmware_file_size = (uint32_t)fw_file.tellg();
		fw_file.seekg(0, ios::beg);

		// Set upgrade flag
		mIsUpgrading = true;

		// send erase cmd
		char cmd_str[128];
		int32_t cmd_str_len = sprintf(cmd_str, "fwu %s erase\r\n", type.c_str());
		if(SendCmdAndWaitResult(cmd_str, cmd_str_len, "success ->", 10000) == false){
			mUpgradeProgress = -2;
			return false;
		}

		// read firmware file , encode to base64, send to camera
		const int32_t SINGLE_TX_LEN = 128;
		char *buf_read = new char[SINGLE_TX_LEN];
		char *buf_send = new char[SINGLE_TX_LEN * 2];
		int32_t total_tx_len = 0;
		uint32_t file_crc = 0xFFFFFFFF;
		do{
			fw_file.read(buf_read, SINGLE_TX_LEN);
			int32_t read_len = (int32_t)fw_file.gcount();
			if(read_len > 0){
				uint32_t crc = Crc32(buf_read, read_len);
				int32_t cmd_len = sprintf(buf_send, "fwu %s write 0x%x 0x%x 0x%x ", type.c_str(), total_tx_len, read_len, crc);
				int32_t base64_len = Base64Encode(buf_read, read_len, buf_send + cmd_len, SINGLE_TX_LEN * 2 - 2 - cmd_len);
				if(base64_len > 0){
					cmd_len += base64_len;
					buf_send[cmd_len++] = '\r';
					buf_send[cmd_len++] = '\n';
					buf_send[cmd_len] = '\0';
					if(SendCmdAndWaitResult(buf_send, cmd_len, "success ->", 2000) == false){
						mUpgradeProgress = -3;
						break;
					}
					total_tx_len += read_len;
					file_crc = Crc32(buf_read, read_len, 0, file_crc);
					mUpgradeProgress = total_tx_len * 100 / firmware_file_size;
				}else{
					mUpgradeProgress = -4;
					break;
				}
			}
		}while(!fw_file.eof() && !mStopUpgradingFlag);

		delete []buf_read;
		delete []buf_send;
		fw_file.close();

		cmd_str_len = sprintf(cmd_str, "fwu %s finish 0x%x 0x%x\r\n", type.c_str(), total_tx_len, file_crc);
		if (SendCmdAndWaitResult(cmd_str, cmd_str_len, "success ->", 3000) == false) {
			mUpgradeProgress = -5;
			return false;
		}

		mIsUpgrading = false;
		return mUpgradeProgress == 100 ? true : false;
	});
	return true;
}

bool DepthCameraCmdPort::StopUpgrade()
{
	//wait for future result
	if (mUpgradeFuture.valid()) {
		mStopUpgradingFlag = false;
		mUpgradeFuture.get();
		mIsUpgrading = false;
	}
	return true;
}

int32_t DepthCameraCmdPort::GetUpgradeProgress()
{
	return mUpgradeProgress;
}

bool DepthCameraCmdPort::SetIntegrationTime(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "intg %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetExternIlluminatePower(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "isl cd 0x%x\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetInternalIlluminatePower(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "inled %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetFrameRate(uint16_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "fps %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SwitchMirror()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "mirror\r\n");
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetBinning(uint8_t rows, uint8_t columns)
{
	char cmd[32];
	rows = rows > 3 ? 3 : rows;
	columns = columns > 3 ? 3 : columns;
	int32_t len = snprintf(cmd, sizeof(cmd), "binning %d %d\r\n", rows, columns);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::RestoreFactorySettings()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "rfs\r\n");
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetHdrRatio(uint8_t value) {
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "hdr %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::GetDepthScale(float & scale)
{
	char response_buf[1024];
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "scale\r\n");
	int32_t res_len = SendCmdAndWaitResponse(cmd, len, 1000, response_buf, sizeof(response_buf) - 1);
	if (res_len > 0) {
		response_buf[res_len] = 0;
		char * str = strstr((char*)response_buf, "scale: ");
		char * endstr = strstr((char*)response_buf, "\r\nidcs>");
		if (endstr) *endstr = 0;
		if (str) {
			scale = strtof(str, NULL);
			return (scale == 0) ? false : true;
		}
	}
	return false;
}

bool DepthCameraCmdPort::Calibration(int32_t distance)
{
	char cmd[32];

	for (int i = 1; i <= 2; i++) {
		// begin test
		int32_t len = snprintf(cmd, sizeof(cmd), "cali f%d test\r\n", i);
		bool ret = SendCmdAndWaitResult(cmd, len, "success ->");
		if (!ret)
			return false;

		// wait for a second
		this_thread::sleep_for(chrono::seconds(1));

		// set data
		len = snprintf(cmd, sizeof(cmd), "cali f%d set %d\r\n", i, distance);
		ret = SendCmdAndWaitResult(cmd, len, "success ->");
		if (!ret)
			return false;
	}

	return true;
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
		char * endstr = strstr((char*)response_buf, "\r\nidcs>");
		if(endstr) *endstr = 0;
		if (str){
			status_str = str + len;
			return true;
		}
	}
	return false;
}

bool DepthCameraCmdPort::SaveConfig()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "save\r\n");
	return SendCmdAndWaitResult(cmd, len, "success ->");
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
	static const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	
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
				*out++ = base64_alphabet[a4[i]];
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
			*out++ = base64_alphabet[a4[j]];

		while ((i++ < 3))
			*out++ = '=';
	}

	return (out == (out_begin + encoded_length)) ? encoded_length : -1;
}

uint32_t DepthCameraCmdPort::Crc32(const char * input, int32_t input_len, int32_t offset, uint32_t crc)
{
	const uint32_t polynomial = 0x04c11db7;
	uint32_t len = input_len / 4;
	const uint32_t * ptr = (uint32_t *)input;
	for (uint32_t i = 0; i < len; i++)
	{
		uint32_t xbit = 1 << 31;
		uint32_t data = ptr[i];
		for (uint32_t bits = 0; bits < 32; bits++)
		{
			if (crc & 0x80000000) {
				crc <<= 1;
				crc ^= polynomial;
			}
			else
				crc <<= 1;
			if (data & xbit)
				crc ^= polynomial;

			xbit >>= 1;
		}
	}
	return crc;
}
