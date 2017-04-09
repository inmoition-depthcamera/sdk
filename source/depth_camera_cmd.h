
#ifndef __DEPTH_CAMERA_CMD_PORT__
#define __DEPTH_CAMERA_CMD_PORT__

#include "inttypes.h"
#include <vector>
#include <string>

using namespace std;

#ifdef WIN32
#include "os/cmd_interface_win.h"
#else
#include "os/cmd_interface_linux.h"
#endif



#ifdef WIN32
class DepthCameraCmdPort : public CmdInterfaceWin
#else
class DepthCameraCmdPort : public CmdInterfaceLinux
#endif
{
public:
    DepthCameraCmdPort();
    ~DepthCameraCmdPort();

    // Port Open&Close 
    static int32_t GetDepthCmdPortList(vector<string> *cmd_port_list);

    // Firmware Upgrade
    bool StartUpgrade(string firmware_file_name);
    bool StopUpgrade();
    bool GetUpgradeProgress();

    // Camera param config
    bool SetIntegrationTime(uint8_t value);
	bool SetExternIlluminatePower(uint8_t value);
	bool SetInternalIlluminatePower(uint8_t value);
	bool SetFrameRate(uint16_t value);
	bool SwitchMirror();
	bool SetBinning(uint8_t rows, uint8_t columns);
	bool RestoreFactorySettings();
	bool GetSystemStatus(char * status_buf, int32_t status_buf_len);
	bool GetCameraConfig(char * config_buf, int32_t config_buf_len);

    // Low level interface to read & write reg directly
	bool ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t* reg_value);
    bool WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t reg_value);

    // Save Config
    bool SaveConfig();
private:

	bool SendCmdAndWaitResult(const char * cmd, int32_t cmd_len, const char * result_ok_str);
};

#endif
