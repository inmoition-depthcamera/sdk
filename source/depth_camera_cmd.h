
/// Copyright (C) 2017 by INMOTION
/// @file depth_camera_cmd.h
/// @brief This file is used to connect INMOTION depth camera's cmd port
/// 
/// Inmotion depth camera's cmd port is base on a USB "Virtual Serial Port",
/// in linux the device name is like "/dev/ttyACM0", in Windows, the device 
/// name is like "COM4".
//
/// @author Gavin
/// @date 2017-04-18


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

/// @brief INMOTION depth camera cmd interface.
///
/// This class is use to connect depth camera usb cmd
/// port, and give user many mothed to set the camera's
/// parameters, upgrade the firmware, and calibration
class DepthCameraCmdPort : public CMD_INTERFACE_DRIVER
{
public:

	/// @brief The constructor
    DepthCameraCmdPort();
	/// @brief The destructor 
    ~DepthCameraCmdPort();
	
	/// @brief Start Upgrade the depth camera's firmware
	/// 
	/// This function will create an independence thread to download 
	/// specified firmware file, and will return immediately.
	///
	/// @param firmware_file_name the firmware file path name.
	/// @return If last upgrading process has not finished, will 
	///         return false, else will return true
    bool StartUpgrade(string firmware_file_name, string type);

	/// @brief Start Stop the upgrading process
	/// 
	/// This function will set a flag and wait last upgrading thread
	/// to exit.
	///
	/// @return If last upgrading process has not finished, will 
	///         return true, else will return false
	/// @warning This function will block to wait for upgrading thread exit.
    bool StopUpgrade();

	/// @brief Get Upgrade progress
	/// @return Return the detail progrss in percent(0~100)
    int32_t GetUpgradeProgress();

	/// @brief Set the integration time of camera
	/// @param value The intgration time of camera in percent(0~100)
	/// @return Return ture if successed
	/// @note High integration time will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
    bool SetIntegrationTime(uint8_t value);

	/// @brief Set the extern illuminate power
	/// @param value The value range is [0~0xFF]
	/// @return Return ture if successed
	/// @note Some depth camera do not have external illumination.
	///       High illumiate power will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
	bool SetExternIlluminatePower(uint8_t value);

	/// @brief Set the internal illuminate power
	/// @param value The value range is [0~0xFF]
	/// @return Return ture if successed
	/// @note Some depth camera do not have external illumination.
	///       High illumiate power will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
	bool SetInternalIlluminatePower(uint8_t value);

	/// @brief Set the frame rate of the camera
	/// @param value The value range depends on camera spec.
	/// @return Return ture if successed
	/// @note Some depth camera will not work, in a wrong frame
	///       rate value. High frame rate will reduce the detect
	///       range of camera.
	bool SetFrameRate(uint16_t value);

	/// @brief Mirror the output by 180 degree
	/// @return Return ture if successed
	bool SwitchMirror();

	/// @brief Set the Binning of the depth camera
	/// @param rows Binning 
	/// @return Return ture if successed
	/// @note Some depth camera will not work, in a wrong frame
	///       rate value. High frame rate will reduce the detect
	///       range of camera.
	bool SetBinning(uint8_t rows, uint8_t columns);

	/// @brief Restore depth camera setting to factory setting.
	/// @return Return ture if successed
	bool RestoreFactorySettings();

	/// @brief Get current status information of depth camera.
	///
	/// This function return the status information of current depth camera.
	/// The output of this function is like following:
	///	    Video Frame Rate:  33.3 FPS
	///	    Integration Time : 59 %
	///	    Illumination:      255
	///	    Center(6 * 6) Value: Phase = 536974412, Amplitude = 536974420
	///	    Frequence(MHz) : Freq1 = 40.000, Freq2 = 60.000,
	///	    VCO_Freq1 = 480.000, VCO_Freq2 = 360.000
	///	    Others:   Max Distance = 7.50 m, dealiased_ph_mask = 2,
	///	    		  Max Avilable Phase Value = 3072,
	///	    	      GCD = 20000000, ma = 2, mb = 3,
	///
	/// @param status_str The output result will store into this parameter
	/// @return Return ture if successed
	bool GetSystemStatus(string &status_str);

	/// @brief Set the camera's hdr ratio.
	///
	/// A nonzero value that will allow the depth camera to enter the HDR mode. 
	/// In HDR mode, the camera output two kinds of frames : the normal frame and HDR frame, 
	/// the output form : NORMAL_FRAME, HDR_FRAME, NORMAL_FRAME, HDR_FRAME, NORMAL_FRAME, HDR_FRAME...
	/// The integration time of NORMAL_FRAME: `normal_intg` depending on the SetIntegrationTime function.
	/// The integration time of HDR_FRAME: `hdr_intg` = normal_intg >> hdr_ratio.The hdr_ratio is setted by SetHdrRatio function.
	/// The hdr_ratio value range is[0~7].
	///
	/// Users will call SetHdrMode method of DepthCameraUvcPort to process hdr frame, or not call to process the hdr frame themself.
	///
	/// @param hdr_ratio The retio of normal_intg
	/// @return Return ture if successed
	bool SetHdrRatio(uint8_t hdr_ratio);

	/// @brief Get the depth data to distance scale
	/// @param scale the detph data to distance scale. distance = scale * phase
	/// @return Return ture if successed
	bool GetDepthScale(float &scale);

	
	/// @brief Calibrate the device with a given distance
	///
	/// The camera will use the center 6*6 rectange's average value to calibrate the camera params.
	/// The calibration process takes about 2~3 seconds, 
	/// in the calibration process, can not move the calibration target and camera.
	///
	/// @param distance The distance of the center object(center 6*6 rectange). The unit of this parameter is mm.
	/// @return Return ture if successed
	bool Calibration(int32_t distance);
	
	/// @brief Save modified settings to internal FLASH
	///
	/// If you do not call SaveConfig, all settings(like change the hdr ratio or integration time) 
	/// will not be saved, and will only be valid for the current power cycle, after you are re powered,
	/// all settings will be lost. 
	/// After setting up the camera, you need to call the SaveConfig function 
	/// when you need to save the settings
	/// @return Return ture if successed
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
