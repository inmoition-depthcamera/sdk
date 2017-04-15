
#ifndef __DEPTH_CAMERA_CMD_PORT__
#define __DEPTH_CAMERA_CMD_PORT__

#include "inttypes.h"
#include <vector>
#include <string>
#include <future>

using namespace std;

#ifdef WIN32
	#include "os/cmd_interface_win.h"
	#define CMD_INTERFACE_DRIVER CmdInterfaceWin
#else
	#include "os/cmd_interface_linux.h"
	#define CMD_INTERFACE_DRIVER CmdInterfaceLinux
#endif


class DepthCameraCmdPort : public CMD_INTERFACE_DRIVER
{
public:
    DepthCameraCmdPort();
    ~DepthCameraCmdPort();
	
    // Firmware Upgrade
    bool StartUpgrade(string firmware_file_name);
    bool StopUpgrade();
    int32_t GetUpgradeProgress();

    // Camera param config
    bool SetIntegrationTime(uint8_t value);
	bool SetExternIlluminatePower(uint8_t value);
	bool SetInternalIlluminatePower(uint8_t value);
	bool SetFrameRate(uint16_t value);
	bool SwitchMirror();
	bool SetBinning(uint8_t rows, uint8_t columns);
	bool RestoreFactorySettings();
	bool GetSystemStatus(string &status_str);
	bool GetCameraConfig(string &config_str);
	bool SetHdrRatio(uint8_t value);

    // Low level interface to read & write reg directly
	bool ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t* reg_value);
    bool WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t reg_value);

    // Save Config
    bool SaveConfig();
private:

	bool SendCmdAndWaitResult(const char * cmd, int32_t cmd_len, const char * result_ok_str, int32_t timeout = 1000);
	int32_t Base64Encode(const char *input, size_t input_length, char *out, size_t out_length);

	atomic_bool mIsUpgrading, mStopUpgradingFlag;
	atomic<int32_t> mUpgradeProgress;
	future<bool> mUpgradeFuture;
};

#endif
