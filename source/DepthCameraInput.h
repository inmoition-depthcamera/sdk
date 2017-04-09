#ifndef __DEPTH_CAMERA_INPUT_H__
#define __DEPTH_CAMERA_INPUT_H__

#include "dshow.h"
#include "GrabberCB.h"
#include "VideoDevice.h"
#include "DepthCameraCmd.h"
#include "DepthDenoise.h"

#define VI_MAX_CAMERAS  20
#define VI_NUM_TYPES    20 //MGB
#define VI_NUM_FORMATS  18 //DON'T TOUCH

#define VTID_RGB24  0
#define VTID_RGB32  1
#define VTID_RGB565 3
#define VTID_YUY2   4
#define VTID_YVYU   5
#define VTID_YUYV   6
#define VTID_YV12   9
#define CONGIF_SIZE     512

#define USB_VID_PID     "vid_0483&pid_5760"


//////////////////////////////////////   VIDEO INPUT   /////////////////////////////////////
class DepthCameraInput: public DepthCameraCmd
{

public:
	DepthCameraInput();
	~DepthCameraInput();

	int OpenById(int id, int openCmdFalg);

	void Close(int id);

	// return download firmware status
	//int updateFirmwareStartUnicode(int deviceID, WCHAR * path);
	int updateFirmwareStart(int deviceID, char* path);

	void updateFirmwareStop(int deviceID);

	int getUpdateProgress() { return UpdateProgress; }

	int isUpdating() { return IsUpdatingFlag; }

	//turns off console messages - default is to print messages
	static void setVerbose(bool _verbose);

	//Functions in rough order they should be used.
	static int listVideos(int silent = 0);

	//needs to be called after listDevices - otherwise returns NULL
	static char * getVideoName(int deviceID);

	//get video Current Frame Rate
	int getCurrentFrameRate(int deviceID);

	//choose to use callback based capture - or single threaded
	int setDepthCameraFrameCallBack(int deviceID, OnDepthCameraFrameCallBack userCallback);

	//call before setupDevice
	//directshow will try and get the closest possible framerate to what is requested
	void setIdealFramerate(int deviceID, int idealFramerate);

	//some devices will stop delivering frames after a while - this method gives you the option to try and reconnect
	//to a device if videoInput detects that a device has stopped delivering frames.
	//you MUST CALL isFrameNew every app loop for this to have any effect
	void setAutoReconnectOnFreeze(int deviceNumber, bool doReconnect, int numMissedFramesBeforeReconnect);

	//If you need to you can set your NTSC/PAL/SECAM
	//preference here. if it is available it will be used.
	//see #defines above for available formats - eg VI_NTSC_M or VI_PAL_B
	//should be called after setupDevice
	//can be called multiple times
	int setFormat(int deviceNumber, int format);

	//Tells you when a new frame has arrived - you should call this if you have specified setAutoReconnectOnFreeze to true
	int isFrameNew(int deviceID);

	// get device runing status
	int isVideoSetup(int deviceID);

	//Or pass in a buffer for getPixels to fill returns true if successful.
	int getPixels(int id, float * anlge, unsigned short * depth_Buffer, unsigned short *  amplitude_Buffer, unsigned char * ambient_Buffer, unsigned char * flags_Buffer);

    void DepthToPointCloud(int id, unsigned short *phase, float *pc);

	int DepthToPointCloud(int id, unsigned short *phase, unsigned short *amplitude, unsigned short phaseMax, unsigned short amplitudeMin, float *pc, float *pcc);

	void PhaseDenoise(unsigned short *phase, unsigned short *amplitude, unsigned char *flags, unsigned short* DstFrame, int Amp_Thr);

	//Launches a pop up settings window
	//For some reason in GLUT you have to call it twice each time.
	void showSettingsWindow(int deviceID);

	//get width of Frame
	int  getFrameWidth(int deviceID);

	//get height of Frame
	int  getFrameHeight(int deviceID);

	//get size of Frame
	int  getFrameSize(int deviceID);


	int  getFourcc(int deviceID);

	//number of devices available
	int devicesFound;
	


	static bool verbose;
	
private:
	//Choose one of these five to setup your device
	int setupVideo(int deviceID);

	//completely stops and frees a device
	void stopVideo(int deviceID);

	//as above but then sets it up with same settings
	bool restartVideo(int deviceID);

	void setAttemptCaptureSize(int deviceID, int w, int h);
	void setAttemptCaptureType(int deviceID, GUID mediaType = MEDIASUBTYPE_RGB24);

	bool setup(int deviceID);
	//void processPixels(unsigned char * src, unsigned char * dst, int width, int height, bool bRGB, bool bFlip);
	int  start(int deviceID, VideoDevice * VD);
	int  getDeviceCount();
	void getMediaSubtypeAsString(GUID type, char * typeAsString);
	GUID *getMediaSubtypeFromFourcc(int fourcc);
	GUID *getMediaSubtypeFromID(int id);
	int    getFourccFromMediaSubtype(GUID type);

	void getVideoPropertyAsString(int prop, char * propertyAsString);
	void getCameraPropertyAsString(int prop, char * propertyAsString);

	HRESULT getDevice(IBaseFilter **pSrcFilter, int deviceID, WCHAR * wDeviceName, char * nDeviceName);
	static HRESULT ShowFilterPropertyPages(IBaseFilter *pFilter);

	HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);

	int UpdateVideoID = -1;
	char     FilePath[256];
	uint32_t SendFileSize = 0;
	uint8_t  SendDataBuffer[2048];
	uint32_t SendOffset = 0;
	int    UpdateProgress = 0;
	uint32_t LoadSendData(int end);
	void UpdateFirmwareLoop();
	HANDLE hDownloadFirmwareThread;

	static DWORD WINAPI UpdateFirmwareThread(LPVOID pM) {
		DepthCameraInput *self = (DepthCameraInput *)pM;
		self->UpdateFirmwareLoop();
		return 0;
	}

	//don't touch
	static bool comInit();
	static bool comUnInit();

	int FrameWidth;
	int FrameHeight;
	float W_focal;
	float H_focal;
	float PrincipalPoint_X;
	float PrincipalPoint_Y;
	int  connection;
	int  callbackSetCount;
	int  isVideoClosed;
	GUID CAPTURE_MODE;

	VideoDevice * VDList[VI_MAX_CAMERAS];
	GUID mediaSubtypes[VI_NUM_TYPES];
	long formatTypes[VI_NUM_FORMATS];

	int OpenCmdFalg;
	static void __cdecl basicThread(void * objPtr);
	static char deviceMonikerString[VI_MAX_CAMERAS][255];
	static char deviceNames[VI_MAX_CAMERAS][255];
	float* Table;
	char ConfigBuffer[CONGIF_SIZE];
	DepthDenoise DDI;

};

#endif