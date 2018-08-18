
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
/// port, and give user many method to set the camera's
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

	/// @brief Stop the upgrading process
	/// 
	/// This function will set a flag and wait last upgrading thread
	/// to exit.
	///
	/// @return If last upgrading process has not finished, will 
	///         return true, else will return false
	/// @warning This function will block to wait for upgrading thread exit.
    bool StopUpgrade();

	/// @brief Get Upgrade progress
	/// @return Return the detail progress in percent(0~100)
    int32_t GetUpgradeProgress();

	/// @brief Get Upgrade progress
	/// @return Return the detail progress in percent(0~100)
	int32_t IsUpgrading();

	/// @brief Set the integration time of camera
	/// @param value The intgration time of camera in percent(0~100)
	/// @return Return true if successed
	/// @note High integration time will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
    bool SetIntegrationTime(uint8_t value);

	/// @brief Set the extern illuminate power
	/// @param value The value range is [0~0xFF]
	/// @return Return true if successed
	/// @note Some depth camera do not have external illumination.
	///       High illuminate power will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
	bool SetExternIlluminatePower(uint8_t value);

	/// @brief Set the internal illuminate power
	/// @param value The value range is [0~0xFF]
	/// @return Return true if successed
	/// @note Some depth camera do not have external illumination.
	///       High illuminate power will reduce the noise and 
	///       increase detect range, but also will increase the 
	///       system power and increase the blind range.
	bool SetInternalIlluminatePower(uint8_t value);

	/// @brief Set the frame rate of the camera
	/// @param value The value range depends on camera spec.
	/// @return Return true if successed
	/// @note Some depth camera will not work, in a not suitable frame
	///       rate value, the available fps values are different from different devices. 
	///       High frame rate will reduce the detect range of camera. 
	bool SetFrameRate(uint16_t value);

	/// @brief Mirror the output by 180 degree
	/// @return Return true if successed
	bool SwitchMirror();

	/// @brief Set the Binning of the depth camera
	/// @param rows Binning rows
	/// @param columns Binning columns
	/// @return Return true if successed
	/// @note Some depth camera will not support binning.
	bool SetBinning(uint8_t rows, uint8_t columns);

	/// @brief Restore depth camera setting to factory setting.
	/// @return Return true if successed
	bool RestoreFactorySettings();

	/// @brief Get current status information of depth camera.
	///
	/// This function return the status information of current depth camera.
	/// The output of this function is like following:
	///	    Product: INMOTION IDC8060R-7C0F80E7
	///     Device ID : 00007C0F80E7
	///	    Firmware Version : 2.2.1
	///	    Video Channel : cdc
	///	    Video Width : 160
	///	    Video Height : 60
	///	    FPS : 32.0
	///     FrameRate Set : 40.0
	/// 	Integration Time(%) : 29
	/// 	Center Phase Value : 1170
	/// 	Center Amplitude Value : 37
	/// 	Freq1(MHz) : 24.000
	/// 	Max Distance(m) : 6.25
	/// @param status_str The output result will store into this parameter
	/// @return Return true if successed
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
	/// @param hdr_ratio The ratio of normal_intg
	/// @return Return true if successed
	bool SetHdrRatio(uint8_t hdr_ratio);

	/// @brief Get the depth data to distance scale
	/// @param scale the detph data to distance scale. distance = scale * phase
	/// @return Return true if successed
	bool GetDepthScale(float &scale);
	
	/// @brief Calibrate the device with a given distance
	///
	/// The camera will use the center 6*6 rectange's average value to calibrate the camera params.
	/// The calibration process takes about 2~3 seconds, 
	/// in the calibration process, can not move the calibration target and camera.
	///
	/// @param distance The distance of the center object(center 6*6 rectange). The unit of this parameter is mm.
	/// @param freq_cnt The Calibration frequency count, 1 -> calibrate freq1, 2 -> calibrate freq1 and freq2
	/// @return Return true if successed
	bool Calibration(int32_t distance, int32_t freq_cnt);
	
	/// @brief Save modified settings to internal FLASH
	///
	/// If you do not call SaveConfig, all settings(like change the hdr ratio or integration time) 
	/// will not be saved, and will only be valid for the current power cycle, after you are re powered,
	/// all settings will be lost. 
	/// After setting up the camera, you need to call the SaveConfig function 
	/// when you need to save the settings
	///
	/// @return Return true if successed
	bool SaveConfig();

	/// @brief Enable/Disable video stream from cdc port
	///
	/// Some low resolution camera support video from cdc.
	///
	/// @param enable_disable true -> enable, false -> disable
	/// @return Return true if successed
	bool CdcVideoControl(bool enable_disable);

	/// @brief Set the operation mode of the device
	///
	/// Setup operation mode: master(default), or slave.
	/// On Master mode, fps is controlled by fps cmd.
	/// On Slave mode, fps is controlled by slave_fps param.
	/// The slave_fps param range is 1~33 
	/// This operation is added in/after firmware version: 2.4.1
	///
	/// @param mode 0 -> master, 1 -> slave
	/// @param slave_fps set the fps in slave mode.(master mode will ignore this param)
	/// @return Return true if successed
	bool SetOperationMode(int32_t mode, int32_t slave_fps);

	/// @brief Reboot depth camera device
	///
	/// This operation will make a reset to depth camera device
	/// and the usb connection will lost after this cmd
	///
	/// @return Return true if successed
	void SystemReboot();

protected:

	bool SendCmdAndWaitResult(const char * cmd, int32_t cmd_len, const char * result_ok_str, int32_t timeout = 3000);
	static int32_t Base64Encode(const char *input, size_t input_length, char *out, size_t out_length);
	static uint32_t Crc32(const uint8_t *p, int len, int32_t offset = 0, uint32_t crc = 0xFFFFFFFF);

	atomic_bool mIsUpgrading, mStopUpgradingFlag;
	atomic<int32_t> mUpgradeProgress;
	future<bool> mUpgradeFuture;
};

#endif
