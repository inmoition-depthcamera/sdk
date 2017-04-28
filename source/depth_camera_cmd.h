
/// Copyright (C) 2017 by INMOTION
/// \file depth_camera_cmd.h
/// \brief This file is used to connect INMOTION depth camera's cmd port
/// 
/// Inmotion depth camera's cmd port is base on a USB "Virtual Serial Port",
/// in linux the device name is like "/dev/ttyACM0", in Windows, the device 
/// name is like "COM4".
//
/// \author Gavin
/// \date 2017-04-18


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

/// \brief INMOTION depth camera cmd interface.
///
/// This class is use to connect depth camera usb cmd
/// port, and give user many mothed to set the camera's
/// parameters, upgrade the firmware, and calibration
class DepthCameraCmdPort : public CMD_INTERFACE_DRIVER
{
public:

	/// \brief The constructor
    DepthCameraCmdPort();
	/// \brief The destructor 
    ~DepthCameraCmdPort();
	
	/// \brief Start Upgrade the depth camera's firmware
	/// 
	/// This function will create an independence thread to download 
	/// specified firmware file, and will return immediately.
	///
	/// \param firmware_file_name the firmware file path name.
	/// \return If last upgrading process has not finished, will 
	///         return false, else will return true
    bool StartUpgrade(string firmware_file_name, string type);

	/// \brief Start Stop the upgrading process
	/// 
	/// This function will set a flag and wait last upgrading thread
	/// to exit.
	///
	/// \return If last upgrading process has not finished, will 
	///         return true, else will return false
	/// @warning This function will block to wait for upgrading thread exit.
    bool StopUpgrade();

	/// \brief Get Upgrade progress
	/// \return Return the detail progrss in percent(0~100)
    int32_t GetUpgradeProgress();

	/// \brief Set the integration time of camera
	/// \param value The intgration time of camera in percent(0~100)
	/// \return Return ture if successed
	/// \note High integration time will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
    bool SetIntegrationTime(uint8_t value);

	/// \brief Set the extern illuminate power
	/// \param value The value range is [0~0xFF]
	/// \return Return ture if successed
	/// \note Some depth camera do not have external illumination.
	///       High illumiate power will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
	bool SetExternIlluminatePower(uint8_t value);

	/// \brief Set the internal illuminate power
	/// \param value The value range is [0~0xFF]
	/// \return Return ture if successed
	/// \note Some depth camera do not have external illumination.
	///       High illumiate power will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
	bool SetInternalIlluminatePower(uint8_t value);

	/// \brief Set the frame rate of the camera
	/// \param value The value range depends on camera spec.
	/// \return Return ture if successed
	/// \note Some depth camera will not work, in a wrong frame
	///       rate value. High frame rate will reduce the detect
	///       range of camera.
	bool SetFrameRate(uint16_t value);

	/// \brief Mirror the output by 180 degree
	/// \return Return ture if successed
	bool SwitchMirror();

	/// \brief Set the Binning of the depth camera
	/// \param rows Binning 
	/// \return Return ture if successed
	/// \note Some depth camera will not work, in a wrong frame
	///       rate value. High frame rate will reduce the detect
	///       range of camera.
	bool SetBinning(uint8_t rows, uint8_t columns);
	bool RestoreFactorySettings();
	bool GetSystemStatus(string &status_str);
	bool GetCameraConfig(string &config_str);
	bool SetHdrRatio(uint8_t value);
	
	// Camera Calibration
	bool Calibration(int32_t distance);

    // Low level interface to read & write reg directly
	bool ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t* reg_value);
    bool WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint32_t reg_value);

    // Save Config
    bool SaveConfig();
private:

	bool SendCmdAndWaitResult(const char * cmd, int32_t cmd_len, const char * result_ok_str, int32_t timeout = 1000);
	static int32_t Base64Encode(const char *input, size_t input_length, char *out, size_t out_length);
	static uint32_t Crc32(const char *input, int32_t input_len, int32_t offset = 0, uint32_t crc = 0xFFFFFFFF);

	atomic_bool mIsUpgrading, mStopUpgradingFlag;
	atomic<int32_t> mUpgradeProgress;
	future<bool> mUpgradeFuture;
};

#endif
