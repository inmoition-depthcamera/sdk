#ifndef __DEPTH_CAMERA_CMD_H__
#define __DEPTH_CAMERA_CMD_H__

#include "dshow.h"
#include "SerialPortInterface.h"
#include "CRC32.h"
#include <process.h>

#include <initguid.h>

#define _WIN32_DCOM
#include <iostream>
#include "TChar.h"
#include "conio.h"
#include <windows.h>
#include <stdio.h>

#include <comdef.h>
#include <Wbemidl.h>
#include "comutil.h"
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "wbemuuid.lib")

#define SECTION_SIZE     1024 
#define ACKBUFFER_SIZE   4096

typedef void(__stdcall *OnDepthCameraCmdCallBack)(unsigned char * rx_buffer, int rx_len);


//////////////////////////////////////   DEPTH CAMERA CMD   /////////////////////////////////////

class DepthCameraCmd 
{
public:
	DepthCameraCmd();
	~DepthCameraCmd();

	// open depth camera correspond serial port
	int startCmd(char* DepthCameraName);

	//close serial port
	void stopCmd();

	int isCmdStart() { return IsStartFlag; }

	int isCmdBusy() { return IsBusyFlag; }

	// value 0 - 55%
	// return : 0 -> OK ; -1 -> Fail
	int setIntegrationTime(uint8_t value);

	// value 0 - 255
	// return : 0 -> OK ; -1 -> Fail
	int setExLight(uint8_t value);

	// value 0 - 230mA
	// return : 0 -> OK ; 1 -> Fail
	int setInLight(uint8_t value);

	// value 0 - 500
	// return : 0 -> OK ; -1 -> Fail
	int setFrameRate(uint16_t value);

	// value 0 - 10
	// return : 0 -> OK ; -1 -> Fail
	int setMotorSpeed(uint8_t value);

	// return : 0 -> OK ; -1 -> Fail
	int switchMirror();

	// Number of rows to merge for binning = 2 ^rows.
	// Number of columns to merge for binning = 2^columns.
	// return : 0 -> OK ; -1 -> Fail
	int setBinning( uint8_t rows, uint8_t columns);

	// Restore The Factory Settings
	// return : 0 -> OK ; -1 -> Fail
	int restoreFactorySettings();

	//Save the modified configuration
	// return : 0 -> OK ; -1 -> Fail
	int saveConfig();

	// get system run status , 
	// return : 0 -> OK ; -1 -> Fail
	int getSystemStatus(char * read_buffer, int32_t buffer_size, int32_t *read_len);

	// get camera config 
	// return : 0 -> Fail ; > 1 -> OK
	int getCameraConfig(char * Config_buffer);

	int setDepthCameraCmdCallBack(OnDepthCameraCmdCallBack cb) { DepthCameraCmdCallBack = cb; return 0; }

	int writeCmd(const char * read_buffer, int32_t size);

protected:
	uint8_t  CmdBuffer[64];
	uint8_t  AckBuffer[ACKBUFFER_SIZE];
	int32_t  ReadCmdOffset;
	int     IsUpdatingFlag = 0;
	int     IsStartFlag = 0;
	int     IsBusyFlag = 0;
	// send cmd to depth camera or wait ack for depth camera
	int sendOrWaitCmd(const uint8_t *sned_buffer, int32_t sned_len, int32_t timeout, uint8_t *ack_buffer, int32_t * ack_Len);

private:
	char CmdPortName[32];
	SerialPortInterface DepthCameraCmdSpi;
    HANDLE hEventRx;
	OnDepthCameraCmdCallBack DepthCameraCmdCallBack;
  
	static void onRxSerailData(int32_t id, void *param, const uint8_t *buffer, int32_t len);

    int  getVideoCorrespondSerialPortName(char* device_name, char* serialPort_name);

	
	uint8_t * RxBufferPtr = NULL;
	int32_t * ReadLen = 0;
};

#endif