
#ifndef __DEPTH_CAMERA_CMD_PORT__
#define __DEPTH_CAMERA_CMD_PORT__

#include "inttypes.h"
#include <vector>
#include <string>

using namespace std;

class DepthCameraCmdPort
{
public:
    // Port Open&Close 
    static int32_t GetDepthCmdPortList(vector<string> *cmd_port_list);
    bool OpenCmdPort(string cmd_port_name);
    bool CloseCmdPort();

    bool IsCmdPortOpened();
    bool IsCmdPortBusy();

    // Firmware Upgrade
    bool StartUpgrade(string firmware_file_name);
    bool StopUpgrade();
    bool GetUpgradeProgress();

    // Camera param config
    bool SetIntegrationTime(uint8_t value);
	bool SetIlluminatePower(uint8_t value);
	bool SetFrameRate(uint16_t value);
	bool SwitchMirror();
	bool SetBinning(uint8_t rows, uint8_t columns);
	bool RestoreFactorySettings();
	string GetSystemStatus();
	string GetCameraConfig();

    // Low level interface to read & write reg directly
	bool ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t* reg_value);
    bool WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t reg_value);

    // Low level interface to Write Cmd to Camera
    bool WriteCmd(string cmd, string *response);

    // Save Config
    bool SaveConfig();
private:
    CmdInterface mCmdIf;
};

#endif
