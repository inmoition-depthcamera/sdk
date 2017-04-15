#include "depth_camera_cmd.h"
#include "string.h"

DepthCameraCmdPort::DepthCameraCmdPort()
{
}

DepthCameraCmdPort::~DepthCameraCmdPort()
{

}

bool DepthCameraCmdPort::StartUpgrade(string firmware_file_name)
{
	return false;
}

bool DepthCameraCmdPort::StopUpgrade()
{
	return false;
}

bool DepthCameraCmdPort::GetUpgradeProgress()
{
	return false;
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

bool DepthCameraCmdPort::SendCmdAndWaitResult(const char * cmd, int32_t cmd_len, const char * result_ok_str)
{
	char response_buf[1024];

	int32_t res_len = SendCmdAndWaitResponse(cmd, cmd_len, 1000, response_buf, sizeof(response_buf) - 1);
	if (res_len > 0) {
		response_buf[1023] = 0;
		if (strstr(response_buf, result_ok_str) != NULL)
			return true;
	}

	return false;
}
