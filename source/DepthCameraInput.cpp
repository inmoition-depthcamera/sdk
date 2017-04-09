#include "DepthCameraInput.h"
#include "Common.h"
#include <process.h>

#include <initguid.h>


using namespace std;


static int comInitCount = 0;

#define VI_VERSION      0.1995

DEFINE_GUID(MEDIASUBTYPE_GREY, 0x59455247, 0x0000, 0x0010, 0x80, 0x00,
	0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y8, 0x20203859, 0x0000, 0x0010, 0x80, 0x00,
	0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y800, 0x30303859, 0x0000, 0x0010, 0x80, 0x00,
	0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

DEFINE_GUID(CLSID_CaptureGraphBuilder2, 0xbf87b6e1, 0x8c27, 0x11d0, 0xb3, 0xf0, 0x00, 0xaa, 0x00, 0x37, 0x61, 0xc5);
DEFINE_GUID(CLSID_FilterGraph, 0xe436ebb3, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(CLSID_NullRenderer, 0xc1f400a4, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
DEFINE_GUID(CLSID_SampleGrabber, 0xc1f400a0, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
DEFINE_GUID(CLSID_SystemDeviceEnum, 0x62be5d10, 0x60eb, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(CLSID_VideoInputDeviceCategory, 0x860bb310, 0x5d01, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(FORMAT_VideoInfo, 0x05589f80, 0xc356, 0x11ce, 0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a);
DEFINE_GUID(IID_IAMAnalogVideoDecoder, 0xc6e13350, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMCameraControl, 0xc6e13370, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMCrossbar, 0xc6e13380, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMStreamConfig, 0xc6e13340, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IAMVideoProcAmp, 0xc6e13360, 0x30ac, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56);
DEFINE_GUID(IID_IBaseFilter, 0x56a86895, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_ICaptureGraphBuilder2, 0x93e5a4e0, 0x2d50, 0x11d2, 0xab, 0xfa, 0x00, 0xa0, 0xc9, 0xc6, 0xe3, 0x8d);
DEFINE_GUID(IID_ICreateDevEnum, 0x29840822, 0x5b84, 0x11d0, 0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86);
DEFINE_GUID(IID_IGraphBuilder, 0x56a868a9, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_IMPEG2PIDMap, 0xafb6c2a1, 0x2c41, 0x11d3, 0x8a, 0x60, 0x00, 0x00, 0xf8, 0x1e, 0x0e, 0x4a);
DEFINE_GUID(IID_IMediaControl, 0x56a868b1, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_IMediaFilter, 0x56a86899, 0x0ad4, 0x11ce, 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(IID_ISampleGrabber, 0x6b652fff, 0x11fe, 0x4fce, 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f);
DEFINE_GUID(LOOK_UPSTREAM_ONLY, 0xac798be0, 0x98e3, 0x11d1, 0xb3, 0xf1, 0x00, 0xaa, 0x00, 0x37, 0x61, 0xc5);
DEFINE_GUID(MEDIASUBTYPE_AYUV, 0x56555941, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_IYUV, 0x56555949, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_RGB24, 0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB32, 0xe436eb7e, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB555, 0xe436eb7c, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_RGB565, 0xe436eb7b, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_UYVY, 0x59565955, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y211, 0x31313259, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y411, 0x31313459, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_Y41P, 0x50313459, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YUY2, 0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YUYV, 0x56595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YV12, 0x32315659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YVU9, 0x39555659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_YVYU, 0x55595659, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_MJPG, 0x47504A4D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); // MGB
DEFINE_GUID(MEDIATYPE_Interleaved, 0x73766169, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIATYPE_Video, 0x73646976, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(PIN_CATEGORY_CAPTURE, 0xfb6c4281, 0x0353, 0x11d1, 0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba);
DEFINE_GUID(PIN_CATEGORY_PREVIEW, 0xfb6c4282, 0x0353, 0x11d1, 0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba);

//defines for formats
#define VI_NTSC_M   0
#define VI_PAL_B    1
#define VI_PAL_D    2
#define VI_PAL_G    3
#define VI_PAL_H    4
#define VI_PAL_I    5
#define VI_PAL_M    6
#define VI_PAL_N    7
#define VI_PAL_NC   8
#define VI_SECAM_B  9
#define VI_SECAM_D  10
#define VI_SECAM_G  11
#define VI_SECAM_H  12
#define VI_SECAM_K  13
#define VI_SECAM_K1 14
#define VI_SECAM_L  15
#define VI_NTSC_M_J 16
#define VI_NTSC_433 17

#ifndef HEADER
#define HEADER(p) (&(((VIDEOINFOHEADER*)(p))->bmiHeader))
#endif

bool DepthCameraInput::verbose = 0;


// ----------------------------------------------------------------------
// Constructor - creates instances of VideoDevice and adds the various
// media subtypes to check.
// ----------------------------------------------------------------------

DepthCameraInput::DepthCameraInput() {
	//start com
	comInit();

	devicesFound = 0;
	callbackSetCount = 0;
	isVideoClosed = 0;
	OpenCmdFalg = 0;
	Table = NULL;
	//setup a max no of device objects
	for (int i = 0; i < VI_MAX_CAMERAS; i++)
		VDList[i] = new VideoDevice();

	if (DepthCameraInput::verbose)printf("\n***** VIDEOINPUT LIBRARY - %2.04f - TFW07 *****\n\n", VI_VERSION);

	//added for the pixelink firewire camera
	//MEDIASUBTYPE_Y800 = (GUID)FOURCCMap(FCC('Y800'));
	//MEDIASUBTYPE_Y8   = (GUID)FOURCCMap(FCC('Y8'));
	//MEDIASUBTYPE_GREY = (GUID)FOURCCMap(FCC('GREY'));

	//The video types we support
	//in order of preference

	mediaSubtypes[0] = MEDIASUBTYPE_RGB24;
	mediaSubtypes[1] = MEDIASUBTYPE_RGB32;
	mediaSubtypes[2] = MEDIASUBTYPE_RGB555;
	mediaSubtypes[3] = MEDIASUBTYPE_RGB565;
	mediaSubtypes[4] = MEDIASUBTYPE_YUY2;
	mediaSubtypes[5] = MEDIASUBTYPE_YVYU;
	mediaSubtypes[6] = MEDIASUBTYPE_YUYV;
	mediaSubtypes[7] = MEDIASUBTYPE_IYUV;
	mediaSubtypes[8] = MEDIASUBTYPE_UYVY;
	mediaSubtypes[9] = MEDIASUBTYPE_YV12;
	mediaSubtypes[10] = MEDIASUBTYPE_YVU9;
	mediaSubtypes[11] = MEDIASUBTYPE_Y411;
	mediaSubtypes[12] = MEDIASUBTYPE_Y41P;
	mediaSubtypes[13] = MEDIASUBTYPE_Y211;
	mediaSubtypes[14] = MEDIASUBTYPE_AYUV;
	mediaSubtypes[15] = MEDIASUBTYPE_MJPG; // MGB

	//non standard									   
	mediaSubtypes[16] = MEDIASUBTYPE_Y800;
	mediaSubtypes[17] = MEDIASUBTYPE_Y8;
	mediaSubtypes[18] = MEDIASUBTYPE_GREY;
	mediaSubtypes[19] = MEDIASUBTYPE_I420;

	//The video formats we support
	formatTypes[VI_NTSC_M] = AnalogVideo_NTSC_M;
	formatTypes[VI_NTSC_M_J] = AnalogVideo_NTSC_M_J;
	formatTypes[VI_NTSC_433] = AnalogVideo_NTSC_433;

	formatTypes[VI_PAL_B] = AnalogVideo_PAL_B;
	formatTypes[VI_PAL_D] = AnalogVideo_PAL_D;
	formatTypes[VI_PAL_G] = AnalogVideo_PAL_G;
	formatTypes[VI_PAL_H] = AnalogVideo_PAL_H;
	formatTypes[VI_PAL_I] = AnalogVideo_PAL_I;
	formatTypes[VI_PAL_M] = AnalogVideo_PAL_M;
	formatTypes[VI_PAL_N] = AnalogVideo_PAL_N;
	formatTypes[VI_PAL_NC] = AnalogVideo_PAL_N_COMBO;

	formatTypes[VI_SECAM_B] = AnalogVideo_SECAM_B;
	formatTypes[VI_SECAM_D] = AnalogVideo_SECAM_D;
	formatTypes[VI_SECAM_G] = AnalogVideo_SECAM_G;
	formatTypes[VI_SECAM_H] = AnalogVideo_SECAM_H;
	formatTypes[VI_SECAM_K] = AnalogVideo_SECAM_K;
	formatTypes[VI_SECAM_K1] = AnalogVideo_SECAM_K1;
	formatTypes[VI_SECAM_L] = AnalogVideo_SECAM_L;

	Common::InitColorMap();
	
}

// ----------------------------------------------------------------------
// static - set whether messages get printed to console or not
//
// ----------------------------------------------------------------------

void DepthCameraInput::setVerbose(bool _verbose) {
	verbose = _verbose;
}

// ----------------------------------------------------------------------
// Set the requested framerate - no guarantee you will get this
//
// ----------------------------------------------------------------------

void DepthCameraInput::setIdealFramerate(int deviceNumber, int idealFramerate) {
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return;

	if (idealFramerate > 0) {
		VDList[deviceNumber]->requestedFrameTime = (unsigned long)(10000000 / idealFramerate);
	}
}


// ----------------------------------------------------------------------
// Set the requested framerate - no guarantee you will get this
//
// ----------------------------------------------------------------------

void DepthCameraInput::setAutoReconnectOnFreeze(int deviceNumber, bool doReconnect, int numMissedFramesBeforeReconnect) {
	if (deviceNumber >= VI_MAX_CAMERAS) return;

	VDList[deviceNumber]->autoReconnect = doReconnect;
	VDList[deviceNumber]->nFramesForReconnect = numMissedFramesBeforeReconnect;
}


// ----------------------------------------------------------------------
// Setup a device with the default settings
//
// ----------------------------------------------------------------------

int DepthCameraInput::setupVideo(int deviceNumber) {
	if (deviceNumber >= VI_MAX_CAMERAS || VDList[deviceNumber]->readyToCapture) return 0;

	if (setup(deviceNumber))return 1;
	return 0;
}
// ----------------------------------------------------------------------
// Setup the default video format of the device
// Must be called after setup!
// See #define formats in header file (eg VI_NTSC_M )
//
// ----------------------------------------------------------------------

int DepthCameraInput::setFormat(int deviceNumber, int format) {
	if (deviceNumber >= VI_MAX_CAMERAS || !VDList[deviceNumber]->readyToCapture) return 0;

	bool returnVal = 0;

	if (format >= 0 && format < VI_NUM_FORMATS) {
		VDList[deviceNumber]->formatType = formatTypes[format];
		VDList[deviceNumber]->specificFormat = 1;

		if (VDList[deviceNumber]->specificFormat) {

			HRESULT hr = getDevice(&VDList[deviceNumber]->pVideoInputFilter, deviceNumber, VDList[deviceNumber]->wDeviceName, VDList[deviceNumber]->nDeviceName);
			if (hr != S_OK) {
				return 0;
			}

			IAMAnalogVideoDecoder *pVideoDec = NULL;
			hr = VDList[deviceNumber]->pCaptureGraph->FindInterface(NULL, &MEDIATYPE_Video, VDList[deviceNumber]->pVideoInputFilter, IID_IAMAnalogVideoDecoder, (void **)&pVideoDec);


			//in case the settings window some how freed them first
			if (VDList[deviceNumber]->pVideoInputFilter)VDList[deviceNumber]->pVideoInputFilter->Release();
			if (VDList[deviceNumber]->pVideoInputFilter)VDList[deviceNumber]->pVideoInputFilter = NULL;

			if (FAILED(hr)) {
				printf("SETUP: couldn't set requested format\n");
			}
			else {
				long lValue = 0;
				hr = pVideoDec->get_AvailableTVFormats(&lValue);
				if (SUCCEEDED(hr) && (lValue & VDList[deviceNumber]->formatType))
				{
					hr = pVideoDec->put_TVFormat(VDList[deviceNumber]->formatType);
					if (FAILED(hr)) {
						printf("SETUP: couldn't set requested format\n");
					}
					else {
						returnVal = 1;
					}
				}

				pVideoDec->Release();
				pVideoDec = NULL;
			}
		}
	}

	return returnVal;
}

// ----------------------------------------------------------------------
// Our static function for returning device names - thanks Peter!
// Must call listDevices first.
//
// ----------------------------------------------------------------------
char DepthCameraInput::deviceNames[VI_MAX_CAMERAS][255] = { { 0 } };
char DepthCameraInput::deviceMonikerString[VI_MAX_CAMERAS][255] = { { 0 } };

char * DepthCameraInput::getVideoName(int deviceID) {
	if (deviceID >= VI_MAX_CAMERAS) {
		return NULL;
	}
	return deviceNames[deviceID];
}

int DepthCameraInput::getCurrentFrameRate(int deviceID)
{
	if (deviceID >= VI_MAX_CAMERAS)
		return 0;
	return (int)(VDList[deviceID]->sgCallback->fps.GetFps() + 0.5f);
}


int DepthCameraInput::setDepthCameraFrameCallBack(int deviceID, OnDepthCameraFrameCallBack userCallback)
{
	if (deviceID >= VI_MAX_CAMERAS)
		return 0;
	VDList[deviceID]->sgCallback->SetDepthCameraFrameCallBack(userCallback);
	return 1;
}

// ----------------------------------------------------------------------
// Our static function for finding num devices available etc
//
// ----------------------------------------------------------------------

int DepthCameraInput::listVideos(int silent) {

	//COM Library Intialization
	comInit();

	if (!silent)printf("\nVIDEOINPUT SPY MODE!\n\n");


	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK) {

			if (!silent)printf("SETUP: Looking For Capture Devices\n");
			IMoniker *pMoniker = NULL;

			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)) {
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}
				
				//Read Moniker String 
				LPOLESTR str = NULL;
				IBindCtx* bindCtx = NULL;
				CreateBindCtx(0, &bindCtx);

				hr = pMoniker->GetDisplayName(bindCtx, NULL, &str);
				if (SUCCEEDED(hr))
				{
					int len = (int)wcslen(str);
					WideCharToMultiByte(CP_ACP, 0, str, -1, deviceMonikerString[deviceCounter], len, NULL, NULL);
					IMalloc *pMalloc;
					hr = CoGetMalloc(1, &pMalloc);
					if (SUCCEEDED(hr))
						pMalloc->Free(str);
				}

				bindCtx->Release();

				//Check if the camera is from inmotion depth
				if (strstr(deviceMonikerString[deviceCounter], USB_VID_PID) != NULL)
				{
					// Find the description or friendly name.
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"Description", &varName, 0);

					if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					if (SUCCEEDED(hr)) {

						hr = pPropBag->Read(L"FriendlyName", &varName, 0);

						int count = 0;
						int maxLen = sizeof(deviceNames[0]) / sizeof(deviceNames[0][0]) - 2;
						while (varName.bstrVal[count] != 0x00 && count < maxLen) {
							deviceNames[deviceCounter][count] = (char)varName.bstrVal[count];
							count++;
						}
						deviceNames[deviceCounter][count] = 0;

						if (!silent)printf("SETUP: %i) %s \n", deviceCounter, deviceNames[deviceCounter]);
					}

					pPropBag->Release();
					pPropBag = NULL;

					pMoniker->Release();
					pMoniker = NULL;

					deviceCounter++;
				}
			}

			pDevEnum->Release();
			pDevEnum = NULL;

			pEnum->Release();
			pEnum = NULL;
		}

		if (!silent)printf("SETUP: %i Device(s) found\n\n", deviceCounter);
	}

	comUnInit();

	return deviceCounter;
}


// ----------------------------------------------------------------------
// Do we need this?
//
// Enumerate all of the video input devices
// Return the filter with a matching friendly name
// ----------------------------------------------------------------------

HRESULT DepthCameraInput::getDevice(IBaseFilter** gottaFilter, int deviceId, WCHAR * wDeviceName, char * nDeviceName) {
	BOOL done = 0;
	int deviceCounter = 0;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the video input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK)
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{

			LPOLESTR str = NULL;
			IBindCtx* bindCtx = NULL;
			CreateBindCtx(0, &bindCtx);

			hr = pMoniker->GetDisplayName(bindCtx, NULL, &str);
			if (SUCCEEDED(hr))
			{
				WideCharToMultiByte(CP_ACP, 0, str, -1, deviceMonikerString[deviceCounter], (int)wcslen(str), NULL, NULL);
				IMalloc *pMalloc;
				hr = CoGetMalloc(1, &pMalloc);
				if (SUCCEEDED(hr))
					pMalloc->Free(str);
			}

			bindCtx->Release();
			if (strstr(deviceMonikerString[deviceCounter], USB_VID_PID) != NULL)
			{
				if (deviceCounter == deviceId)
				{
					// Bind the first moniker to an object
					IPropertyBag *pPropBag;
					hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
					if (SUCCEEDED(hr))
					{
						// To retrieve the filter's friendly name, do the following:
						VARIANT varName;
						VariantInit(&varName);
						hr = pPropBag->Read(L"FriendlyName", &varName, 0);
						if (SUCCEEDED(hr))
						{

							//copy the name to nDeviceName & wDeviceName
							int count = 0;
							while (varName.bstrVal[count] != 0x00) {
								wDeviceName[count] = varName.bstrVal[count];
								nDeviceName[count] = (char)varName.bstrVal[count];
								count++;
							}

							// We found it, so send it back to the caller
							hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)gottaFilter);
							done = 1;
						}
						VariantClear(&varName);
						pPropBag->Release();
						pPropBag = NULL;
						pMoniker->Release();
						pMoniker = NULL;
					}
				}
				deviceCounter++;
			}
		}
		pEnumCat->Release();
		pEnumCat = NULL;
	}
	pSysDevEnum->Release();
	pSysDevEnum = NULL;

	if (done) {
		return hr;    // found it, return native error
	}
	else {
		return VFW_E_NOT_FOUND;    // didn't find it error
	}
}


// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------

int DepthCameraInput::getFrameWidth(int id) {

	if (isVideoSetup(id))
	{
		int w = (VDList[id]->width / 2);
		if (VDList[id]->height % 10 > 0)
			w = (VDList[id]->height - VDList[id]->height % 10);
		return w;
	}

	return 0;

}


// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------

int DepthCameraInput::getFrameHeight(int id) {
	if (isVideoSetup(id))
	{
		int h = VDList[id]->height;
		if (VDList[id]->height % 10 > 0)
			h = (VDList[id]->width / 2);
		return h;
	}
	return 0;
}

// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
int DepthCameraInput::getFourcc(int id) {
	if (isVideoSetup(id))
		return getFourccFromMediaSubtype(VDList[id]->videoType);
	return 0;
}



// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------

int DepthCameraInput::getFrameSize(int id) {

	if (isVideoSetup(id))
	{
		int h = VDList[id]->height;
		int size = (h - h % 10) *(VDList[id]->width / 2);
		return size;
	}

	return 0;

}


// ----------------------------------------------------------------------
// Uses a supplied buffer
// ----------------------------------------------------------------------

int DepthCameraInput::getPixels(int id, float * anlge, unsigned short * depth_Buffer, unsigned short *  amplitude_Buffer, unsigned char * ambient_Buffer, unsigned char * flags_Buffer) {

	bool success = 0;

	if (isVideoSetup(id)) {
		DWORD result = WaitForSingleObject(VDList[id]->sgCallback->hEvent, 1000);
		if (result != WAIT_OBJECT_0) return 0;

		int w = VDList[id]->width;
		int h = VDList[id]->height;
		int size = w / 2 * (h - h % 10);
		int out = (w / 2);
		//double paranoia - mutexing with both event and critical section
		EnterCriticalSection(&VDList[id]->sgCallback->critSection);
		if (depth_Buffer)
		{
			memcpy(depth_Buffer, VDList[id]->sgCallback->PhaseBuffer, size * 2);
			unsigned short * depth_ptr = depth_Buffer;
			float *table_ptr = Table;
			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < out; x++)
				{
					*depth_ptr = (unsigned short)((*depth_ptr) * (*table_ptr) * (7.5 / 3.072));//z
					depth_ptr++;
					table_ptr++;
				}
			}
		}
		if (amplitude_Buffer)
			memcpy(amplitude_Buffer, VDList[id]->sgCallback->AmplitudeBuffer, size * 2);
		if (ambient_Buffer)
			memcpy(ambient_Buffer, VDList[id]->sgCallback->AmbientBuffer, size);
		if (flags_Buffer)
			memcpy(flags_Buffer, VDList[id]->sgCallback->FlagsBuffer, size);
		if (anlge)
			*anlge = VDList[id]->sgCallback->FrameAngle;
		//processPixels(src, dst, width, height, flipRedAndBlue, flipImage);
		VDList[id]->sgCallback->newFrame = 0;

		LeaveCriticalSection(&VDList[id]->sgCallback->critSection);

		ResetEvent(VDList[id]->sgCallback->hEvent);

		success = 1;

	}

	return success;
}





// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------
int DepthCameraInput::isFrameNew(int id) {
	if (!isVideoSetup(id)) return 0;

	bool result = 0;
	bool freeze = 0;

	//again super paranoia!
	EnterCriticalSection(&VDList[id]->sgCallback->critSection);
	result = VDList[id]->sgCallback->newFrame;

	//we need to give it some time at the begining to start up so lets check after 400 frames
	if (VDList[id]->nFramesRunning > 400 && VDList[id]->sgCallback->freezeCheck > VDList[id]->nFramesForReconnect) {
		freeze = 1;
	}

	//we increment the freezeCheck var here - the callback resets it to 1
	//so as long as the callback is running this var should never get too high.
	//if the callback is not running then this number will get high and trigger the freeze action below
	VDList[id]->sgCallback->freezeCheck++;
	LeaveCriticalSection(&VDList[id]->sgCallback->critSection);

	VDList[id]->nFramesRunning++;

	if (freeze && VDList[id]->autoReconnect) {
		if (DepthCameraInput::verbose)printf("ERROR: Device seems frozen - attempting to reconnect\n");
		if (!restartVideo(VDList[id]->myID)) {
			if (DepthCameraInput::verbose)printf("ERROR: Unable to reconnect to device\n");
		}
		else {
			if (DepthCameraInput::verbose)printf("SUCCESS: Able to reconnect to device\n");
		}
	}

	return result;
}


// ----------------------------------------------------------------------
//
//
// ----------------------------------------------------------------------

int DepthCameraInput::isVideoSetup(int id) {

	if (id < devicesFound && VDList[id]->readyToCapture)return 1;
	else return 0;

}


// ----------------------------------------------------------------------
// Gives us a little pop up window to adjust settings
// We do this in a seperate thread now!
// ----------------------------------------------------------------------


void __cdecl DepthCameraInput::basicThread(void * objPtr) {

	//get a reference to the video device
	//not a copy as we need to free the filter
	VideoDevice * vd = *((VideoDevice **)(objPtr));
	ShowFilterPropertyPages(vd->pVideoInputFilter);



	//now we free the filter and make sure it set to NULL
	if (vd->pVideoInputFilter)vd->pVideoInputFilter->Release();
	if (vd->pVideoInputFilter)vd->pVideoInputFilter = NULL;

	return;
}

void DepthCameraInput::showSettingsWindow(int id) {

	if (isVideoSetup(id)) {
		//HANDLE myTempThread;

		//we reconnect to the device as we have freed our reference to it
		//why have we freed our reference? because there seemed to be an issue
		//with some mpeg devices if we didn't
		HRESULT hr = getDevice(&VDList[id]->pVideoInputFilter, id, VDList[id]->wDeviceName, VDList[id]->nDeviceName);
		if (hr == S_OK) {
			//myTempThread = (HANDLE)
			_beginthread(basicThread, 0, (void *)&VDList[id]);
		}
	}
}

// ----------------------------------------------------------------------
// Shutsdown the device, deletes the object and creates a new object
// so it is ready to be setup again
// ----------------------------------------------------------------------

void DepthCameraInput::stopVideo(int id) {
	if (id < VI_MAX_CAMERAS)
	{
		delete VDList[id];
		VDList[id] = new VideoDevice();
	}

}

// ----------------------------------------------------------------------
// Restarts the device with the same settings it was using
//
// ----------------------------------------------------------------------

bool DepthCameraInput::restartVideo(int id) {
	if (isVideoSetup(id))
	{
		int tmpW = VDList[id]->width;
		int tmpH = VDList[id]->height;

		bool bFormat = VDList[id]->specificFormat;
		long format = VDList[id]->formatType;

		int nReconnect = VDList[id]->nFramesForReconnect;
		bool bReconnect = VDList[id]->autoReconnect;

		unsigned long avgFrameTime = VDList[id]->requestedFrameTime;

		stopVideo(id);

		//set our fps if needed
		if (avgFrameTime != (unsigned long)-1) {
			VDList[id]->requestedFrameTime = avgFrameTime;
		}

		if (setupVideo(id)) {
			//reapply the format - ntsc / pal etc
			if (bFormat) {
				setFormat(id, format);
			}
			if (bReconnect) {
				setAutoReconnectOnFreeze(id, 1, nReconnect);
			}
			return 1;
		}
	}
	return 0;
}

// ----------------------------------------------------------------------
// Shuts down all devices, deletes objects and unitializes com if needed
//
// ----------------------------------------------------------------------
DepthCameraInput::~DepthCameraInput() {
	
	if (Table)
		delete Table;

	for (int i = 0; i < VI_MAX_CAMERAS; i++)
	{
		delete VDList[i];
	}
	//Unitialize com
	comUnInit();
}


//////////////////////////////  VIDEO INPUT  ////////////////////////////////
////////////////////////////  PRIVATE METHODS  //////////////////////////////

// ----------------------------------------------------------------------
// We only should init com if it hasn't been done so by our apps thread
// Use a static counter to keep track of other times it has been inited
// (do we need to worry about multithreaded apps?)
// ----------------------------------------------------------------------

bool DepthCameraInput::comInit() {
	HRESULT hr = NOERROR;

	//no need for us to start com more than once
	if (comInitCount == 0) {

		// Initialize the COM library.
		//CoInitializeEx so VideoInput can run in another thread
#ifdef VI_COM_MULTI_THREADED
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
		hr = CoInitialize(NULL);
#endif
		//this is the only case where there might be a problem
		//if another library has started com as single threaded
		//and we need it multi-threaded - send warning but don't fail
		if (hr == RPC_E_CHANGED_MODE) {
			if (DepthCameraInput::verbose)printf("SETUP - COM already setup - threaded VI might not be possible\n");
		}
	}

	comInitCount++;
	return 1;
}


// ----------------------------------------------------------------------
// Same as above but to unitialize com, decreases counter and frees com
// if no one else is using it
// ----------------------------------------------------------------------

bool DepthCameraInput::comUnInit() {
	if (comInitCount > 0)comInitCount--;        //decrease the count of instances using com

	if (comInitCount == 0) {
		CoUninitialize();    //if there are no instances left - uninitialize com
		return 1;
	}

	return 0;
	return 1;
}


// ----------------------------------------------------------------------
// This is the size we ask for - we might not get it though :)
//
// ----------------------------------------------------------------------
void DepthCameraInput::setAttemptCaptureSize(int id, int w, int h) {
	VDList[id]->tryWidth = w;
	VDList[id]->tryHeight = h;
	VDList[id]->tryDiffSize = 1;
}

// ----------------------------------------------------------------------
// This is the videoType we ask for - we might not get it though :)
//
// ----------------------------------------------------------------------
void DepthCameraInput::setAttemptCaptureType(int id, GUID mediaType) {
	VDList[id]->tryDiffVideoType = 1;
	VDList[id]->tryVideoType = mediaType;
}

// ----------------------------------------------------------------------
// Check that we are not trying to setup a non-existant device
// Then start the graph building!
// ----------------------------------------------------------------------

bool DepthCameraInput::setup(int deviceNumber) {


	devicesFound = getDeviceCount();

	if (deviceNumber > devicesFound - 1)
	{
		if (DepthCameraInput::verbose)printf("SETUP: device[%i] not found - you have %i devices available\n", deviceNumber, devicesFound);
		if (devicesFound >= 0) if (DepthCameraInput::verbose)printf("SETUP: this means that the last device you can use is device[%i] \n", devicesFound - 1);
		return 0;
	}

	if (VDList[deviceNumber]->readyToCapture)
	{
		if (DepthCameraInput::verbose)printf("SETUP: can't setup, device %i is currently being used\n", VDList[deviceNumber]->myID);
		return 0;
	}
	

	HRESULT hr = start(deviceNumber, VDList[deviceNumber]);
	if (hr == S_OK)return 1;
	else return 0;
}

//------------------------------------------------------------------------------------------
void DepthCameraInput::getMediaSubtypeAsString(GUID type, char * typeAsString) {

	char tmpStr[8];
	if (type == MEDIASUBTYPE_RGB24)     sprintf_s(tmpStr, "RGB24");
	else if (type == MEDIASUBTYPE_RGB32) sprintf_s(tmpStr, "RGB32");
	else if (type == MEDIASUBTYPE_RGB555)sprintf_s(tmpStr, "RGB555");
	else if (type == MEDIASUBTYPE_RGB565)sprintf_s(tmpStr, "RGB565");
	else if (type == MEDIASUBTYPE_YUY2)  sprintf_s(tmpStr, "YUY2");
	else if (type == MEDIASUBTYPE_YVYU)  sprintf_s(tmpStr, "YVYU");
	else if (type == MEDIASUBTYPE_YUYV)  sprintf_s(tmpStr, "YUYV");
	else if (type == MEDIASUBTYPE_IYUV)  sprintf_s(tmpStr, "IYUV");
	else if (type == MEDIASUBTYPE_UYVY)  sprintf_s(tmpStr, "UYVY");
	else if (type == MEDIASUBTYPE_YV12)  sprintf_s(tmpStr, "YV12");
	else if (type == MEDIASUBTYPE_YVU9)  sprintf_s(tmpStr, "YVU9");
	else if (type == MEDIASUBTYPE_Y411)  sprintf_s(tmpStr, "Y411");
	else if (type == MEDIASUBTYPE_Y41P)  sprintf_s(tmpStr, "Y41P");
	else if (type == MEDIASUBTYPE_Y211)  sprintf_s(tmpStr, "Y211");
	else if (type == MEDIASUBTYPE_AYUV)  sprintf_s(tmpStr, "AYUV");
	else if (type == MEDIASUBTYPE_MJPG)  sprintf_s(tmpStr, "MJPG");
	else if (type == MEDIASUBTYPE_Y800)  sprintf_s(tmpStr, "Y800");
	else if (type == MEDIASUBTYPE_Y8)    sprintf_s(tmpStr, "Y8");
	else if (type == MEDIASUBTYPE_GREY)  sprintf_s(tmpStr, "GREY");
	else if (type == MEDIASUBTYPE_I420)  sprintf_s(tmpStr, "I420");
	else sprintf_s(tmpStr, "OTHER");

	memcpy(typeAsString, tmpStr, sizeof(char) * 8);
}

int DepthCameraInput::getFourccFromMediaSubtype(GUID type) {
	return type.Data1;
}

GUID *DepthCameraInput::getMediaSubtypeFromFourcc(int fourcc) {

	for (int i = 0; i < VI_NUM_TYPES; i++) {
		if ((unsigned long)(unsigned)fourcc == mediaSubtypes[i].Data1) {
			return &mediaSubtypes[i];
		}
	}

	return NULL;
}

GUID *DepthCameraInput::getMediaSubtypeFromID(int id) {
	switch (id) {
	case VTID_RGB24:
	case VTID_RGB32:
	case VTID_RGB565:
	case VTID_YUY2:
	case VTID_YVYU:
	case VTID_YUYV:
	case VTID_YV12:
		return &mediaSubtypes[id];
	}
	return NULL;
}

void DepthCameraInput::getVideoPropertyAsString(int prop, char * propertyAsString) {

	char tmpStr[16];

	if (prop == VideoProcAmp_Brightness) sprintf_s(tmpStr, "Brightness");
	else if (prop == VideoProcAmp_Contrast) sprintf_s(tmpStr, "Contrast");
	else if (prop == VideoProcAmp_Saturation) sprintf_s(tmpStr, "Saturation");
	else if (prop == VideoProcAmp_Hue) sprintf_s(tmpStr, "Hue");
	else if (prop == VideoProcAmp_Gain) sprintf_s(tmpStr, "Gain");
	else if (prop == VideoProcAmp_Gamma) sprintf_s(tmpStr, "Gamma");
	else if (prop == VideoProcAmp_ColorEnable) sprintf_s(tmpStr, "ColorEnable");
	else if (prop == VideoProcAmp_Sharpness) sprintf_s(tmpStr, "Sharpness");
	else sprintf_s(tmpStr, "%u", prop);

	memcpy(propertyAsString, tmpStr, sizeof(char) * 16);
}

void DepthCameraInput::getCameraPropertyAsString(int prop, char * propertyAsString) {

	char tmpStr[16];

	if (prop == CameraControl_Pan) sprintf_s(tmpStr, "Pan");
	else if (prop == CameraControl_Tilt) sprintf_s(tmpStr, "Tilt");
	else if (prop == CameraControl_Roll) sprintf_s(tmpStr, "Roll");
	else if (prop == CameraControl_Zoom) sprintf_s(tmpStr, "Zoom");
	else if (prop == CameraControl_Exposure) sprintf_s(tmpStr, "Exposure");
	else if (prop == CameraControl_Iris) sprintf_s(tmpStr, "Iris");
	else if (prop == CameraControl_Focus) sprintf_s(tmpStr, "Focus");
	else sprintf_s(tmpStr, "%u", prop);

	memcpy(propertyAsString, tmpStr, sizeof(char) * 16);
}


//-------------------------------------------------------------------------------------------
static void findClosestSizeAndSubtype(VideoDevice * VD, int widthIn, int heightIn, int &widthOut, int &heightOut, GUID & mediatypeOut) {
	HRESULT hr;

	//find perfect match or closest size
	int nearW = 9999999;
	int nearH = 9999999;
	//bool foundClosestMatch     = 1;

	int iCount = 0;
	int iSize = 0;
	hr = VD->streamConf->GetNumberOfCapabilities(&iCount, &iSize);

	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		//For each format type RGB24 YUV2 etc
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *pmtConfig;
			hr = VD->streamConf->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);

			if (SUCCEEDED(hr)) {

				//his is how many diff sizes are available for the format
				int stepX = scc.OutputGranularityX;
				int stepY = scc.OutputGranularityY;

				int tempW = 999999;
				int tempH = 999999;

				//Don't want to get stuck in a loop
				if (stepX < 1 || stepY < 1) continue;

				//if(DepthCameraInput::verbose)printf("min is %i %i max is %i %i - res is %i %i \n", scc.MinOutputSize.cx, scc.MinOutputSize.cy,  scc.MaxOutputSize.cx,  scc.MaxOutputSize.cy, stepX, stepY);
				//if(DepthCameraInput::verbose)printf("min frame duration is %i  max duration is %i\n", scc.MinFrameInterval, scc.MaxFrameInterval);

				bool exactMatch = 0;
				bool exactMatchX = 0;
				bool exactMatchY = 0;

				for (int x = scc.MinOutputSize.cx; x <= scc.MaxOutputSize.cx; x += stepX) {
					//If we find an exact match
					if (widthIn == x) {
						exactMatchX = 1;
						tempW = x;
					}
					//Otherwise lets find the closest match based on width
					else if (abs(widthIn - x) < abs(widthIn - tempW)) {
						tempW = x;
					}
				}

				for (int y = scc.MinOutputSize.cy; y <= scc.MaxOutputSize.cy; y += stepY) {
					//If we find an exact match
					if (heightIn == y) {
						exactMatchY = 1;
						tempH = y;
					}
					//Otherwise lets find the closest match based on height
					else if (abs(heightIn - y) < abs(heightIn - tempH)) {
						tempH = y;
					}
				}

				//see if we have an exact match!
				if (exactMatchX && exactMatchY) {
					//foundClosestMatch = 0;
					exactMatch = 1;

					widthOut = widthIn;
					heightOut = heightIn;
					mediatypeOut = pmtConfig->subtype;
				}

				//otherwise lets see if this filters closest size is the closest
				//available. the closest size is determined by the sum difference
				//of the widths and heights
				else if (abs(widthIn - tempW) + abs(heightIn - tempH) < abs(widthIn - nearW) + abs(heightIn - nearH))
				{
					nearW = tempW;
					nearH = tempH;

					widthOut = nearW;
					heightOut = nearH;
					mediatypeOut = pmtConfig->subtype;
				}

				Common::DeleteMediaType(pmtConfig);

				//If we have found an exact match no need to search anymore
				if (exactMatch)break;
			}
		}
	}

}


//---------------------------------------------------------------------------------------------------
static bool setSizeAndSubtype(VideoDevice * VD, int attemptWidth, int attemptHeight, GUID mediatype) {
	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(VD->pAmMediaType->pbFormat);

	AM_MEDIA_TYPE * tmpType = NULL;

	HRESULT    hr = VD->streamConf->GetFormat(&tmpType);
	if (hr != S_OK)return 0;

	//set new size:
	//width and height
	HEADER(pVih)->biWidth = attemptWidth;
	HEADER(pVih)->biHeight = attemptHeight;

	VD->pAmMediaType->formattype = FORMAT_VideoInfo;
	VD->pAmMediaType->majortype = MEDIATYPE_Video;
	VD->pAmMediaType->subtype = mediatype;
	VD->pAmMediaType->lSampleSize = attemptWidth * attemptHeight * 2;

	//set fps if requested
	if (VD->requestedFrameTime != -1) {
		pVih->AvgTimePerFrame = VD->requestedFrameTime;
	}

	//okay lets try new size
	hr = VD->streamConf->SetFormat(VD->pAmMediaType);
	if (hr == S_OK) {
		if (tmpType != NULL)Common::DeleteMediaType(tmpType);
		return 1;
	}
	else {
		VD->streamConf->SetFormat(tmpType);
		if (tmpType != NULL)Common::DeleteMediaType(tmpType);
	}

	return 0;
}

// ----------------------------------------------------------------------
// Where all the work happens!
// Attempts to build a graph for the specified device
// ----------------------------------------------------------------------

int DepthCameraInput::start(int deviceID, VideoDevice *VD) {

	HRESULT hr = NOERROR;
	VD->myID = deviceID;
	VD->setupStarted = 1;
	CAPTURE_MODE = PIN_CATEGORY_CAPTURE; //Don't worry - it ends up being preview (which is faster)
	callbackSetCount = 1;  //make sure callback method is not changed after setup called

	if (DepthCameraInput::verbose)printf("SETUP: Setting up device %i\n", deviceID);

	// CREATE THE GRAPH BUILDER //
	// Create the filter graph manager and query for interfaces.
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&VD->pCaptureGraph);
	if (FAILED(hr))    // FAILED is a macro that tests the return value
	{
		if (DepthCameraInput::verbose)printf("ERROR - Could not create the Filter Graph Manager\n");
		return hr;
	}

	//FITLER GRAPH MANAGER//
	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&VD->pGraph);
	if (FAILED(hr))
	{
		if (DepthCameraInput::verbose)printf("ERROR - Could not add the graph builder!\n");
		stopVideo(deviceID);
		return hr;
	}

	//SET THE FILTERGRAPH//
	hr = VD->pCaptureGraph->SetFiltergraph(VD->pGraph);
	if (FAILED(hr))
	{
		if (DepthCameraInput::verbose)printf("ERROR - Could not set filtergraph\n");
		stopVideo(deviceID);
		return hr;
	}

	//MEDIA CONTROL (START/STOPS STREAM)//
	// Using QueryInterface on the graph builder,
	// Get the Media Control object.
	hr = VD->pGraph->QueryInterface(IID_IMediaControl, (void **)&VD->pControl);
	if (FAILED(hr))
	{
		if (DepthCameraInput::verbose)printf("ERROR - Could not create the Media Control object\n");
		stopVideo(deviceID);
		return hr;
	}


	//FIND VIDEO DEVICE AND ADD TO GRAPH//
	//gets the device specified by the second argument.
	hr = getDevice(&VD->pVideoInputFilter, deviceID, VD->wDeviceName, VD->nDeviceName);

	if (SUCCEEDED(hr)) {
		if (DepthCameraInput::verbose)printf("SETUP: %s\n", VD->nDeviceName);
		hr = VD->pGraph->AddFilter(VD->pVideoInputFilter, VD->wDeviceName);
	}
	else {
		if (DepthCameraInput::verbose)printf("ERROR - Could not find specified video device\n");
		stopVideo(deviceID);
		return hr;
	}

	//we do this because webcams don't have a preview mode
	hr = VD->pCaptureGraph->FindInterface(&CAPTURE_MODE, &MEDIATYPE_Video, VD->pVideoInputFilter, IID_IAMStreamConfig, (void **)&VD->streamConf);
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: Couldn't config the stream!\n");
		stopVideo(deviceID);
		return hr;
	}

	//NOW LETS DEAL WITH GETTING THE RIGHT SIZE
	hr = VD->streamConf->GetFormat(&VD->pAmMediaType);
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: Couldn't getFormat for pAmMediaType!\n");
		stopVideo(deviceID);
		return hr;
	}

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(VD->pAmMediaType->pbFormat);

	VD->tryWidth = HEADER(pVih)->biWidth;
	VD->tryHeight = HEADER(pVih)->biHeight;
	VD->tryVideoType = MEDIASUBTYPE_YUY2;

	bool foundSize = 0;
	char guidStr[8];

	// try specified format and size
	getMediaSubtypeAsString(VD->tryVideoType, guidStr);
	if (DepthCameraInput::verbose)printf("SETUP: trying specified format %s @ %i by %i\n", guidStr, VD->tryWidth, VD->tryHeight);
	if (setSizeAndSubtype(VD, VD->tryWidth, VD->tryHeight, VD->tryVideoType)) {
		VD->videoType = VD->tryVideoType;
		VD->setSize(VD->tryWidth, VD->tryHeight, 2);
		foundSize = 1;

	}

	//if we didn't specify a custom size or if we did but couldn't find it lets setup with the default settings
	if (foundSize == 0)
		return -1;

	//SAMPLE GRABBER (ALLOWS US TO GRAB THE BUFFER)//
	// Create the Sample Grabber.
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&VD->pGrabberF);
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("Could not Create Sample Grabber - CoCreateInstance()\n");
		stopVideo(deviceID);
		return hr;
	}

	hr = VD->pGraph->AddFilter(VD->pGrabberF, L"Sample Grabber");
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("Could not add Sample Grabber - AddFilter()\n");
		stopVideo(deviceID);
		return hr;
	}

	hr = VD->pGrabberF->QueryInterface(IID_ISampleGrabber, (void**)&VD->pGrabber);
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: Could not query SampleGrabber\n");
		stopVideo(deviceID);
		return hr;
	}

	//Set Params - One Shot should be 0 unless you want to capture just one buffer
	hr = VD->pGrabber->SetOneShot(0);
	hr = VD->pGrabber->SetBufferSamples(0);

	//Tell the grabber to use our callback function - 0 is for SampleCB and 1 for BufferCB
	//We use SampleCB
	hr = VD->pGrabber->SetCallback(VD->sgCallback, 1);
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: problem setting callback\n");
		stopVideo(deviceID);
		return hr;
	}
	else {
		if (DepthCameraInput::verbose)printf("SETUP: Capture callback set\n");
	}

	//MEDIA CONVERSION
	//Get video properties from the stream's mediatype and apply to the grabber (otherwise we don't get an RGB image)
	//zero the media type - lets try this :) - maybe this works?
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));

	mt.majortype = MEDIATYPE_Video;
	mt.subtype = VD->tryVideoType;
	mt.formattype = FORMAT_VideoInfo;

	//VD->pAmMediaType->subtype = VD->videoType;
	hr = VD->pGrabber->SetMediaType(&mt);

	//lets try freeing our stream conf here too
	//this will fail if the device is already running
	if (VD->streamConf) {
		VD->streamConf->Release();
		VD->streamConf = NULL;
	}
	else {
		if (DepthCameraInput::verbose)printf("ERROR: connecting device - prehaps it is already being used?\n");
		stopVideo(deviceID);
		return S_FALSE;
	}


	//NULL RENDERER//
	//used to give the video stream somewhere to go to.
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)(&VD->pDestFilter));
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: Could not create filter - NullRenderer\n");
		stopVideo(deviceID);
		return hr;
	}

	hr = VD->pGraph->AddFilter(VD->pDestFilter, L"NullRenderer");
	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: Could not add filter - NullRenderer\n");
		stopVideo(deviceID);
		return hr;
	}

	//RENDER STREAM//
	//This is where the stream gets put together.
	hr = VD->pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, VD->pVideoInputFilter, VD->pGrabberF, VD->pDestFilter);

	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: Could not connect pins - RenderStream()\n");
		stopVideo(deviceID);
		return hr;
	}


	//EXP - lets try setting the sync source to null - and make it run as fast as possible
	{
		IMediaFilter *pMediaFilter = 0;
		hr = VD->pGraph->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
		if (FAILED(hr)) {
			if (DepthCameraInput::verbose)printf("ERROR: Could not get IID_IMediaFilter interface\n");
		}
		else {
			pMediaFilter->SetSyncSource(NULL);
			pMediaFilter->Release();
		}
	}


	//LETS RUN THE STREAM!
	hr = VD->pControl->Run();

	if (FAILED(hr)) {
		if (DepthCameraInput::verbose)printf("ERROR: Could not start graph\n");
		stopVideo(deviceID);
		return hr;
	}

	if (DepthCameraInput::verbose)printf("SETUP: Device is setup and ready to capture.\n\n");
	VD->readyToCapture = 1;

	//Release filters - seen someone else do this
	//looks like it solved the freezes

	//if we release this then we don't have access to the settings
	//we release our video input filter but then reconnect with it
	//each time we need to use it
	VD->pVideoInputFilter->Release();
	VD->pVideoInputFilter = NULL;

	VD->pGrabberF->Release();
	VD->pGrabberF = NULL;

	VD->pDestFilter->Release();
	VD->pDestFilter = NULL;

	return S_OK;
}

// ----------------------------------------------------------------------
// Returns number of good devices
//
// ----------------------------------------------------------------------

int DepthCameraInput::getDeviceCount() {

	return listVideos(1);
}

// ----------------------------------------------------------------------
// Show the property pages for a filter
// This is stolen from the DX9 SDK
// ----------------------------------------------------------------------

HRESULT DepthCameraInput::ShowFilterPropertyPages(IBaseFilter *pFilter) {

	ISpecifyPropertyPages *pProp;

	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr))
	{
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo);
		IUnknown *pFilterUnk;
		pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

		// Show the page.
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		pProp->Release();
		OleCreatePropertyFrame(
			NULL,                   // Parent window
			0, 0,                   // Reserved
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			&pFilterUnk,            // Array of object pointers.
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL                 // Reserved
			);

		// Clean up.
		if (pFilterUnk)pFilterUnk->Release();
		if (FilterInfo.pGraph)FilterInfo.pGraph->Release();
		CoTaskMemFree(caGUID.pElems);
	}
	return hr;
}

// ----------------------------------------------------------------------
// This code was also brazenly stolen from the DX9 SDK
// Pass it a file name in wszPath, and it will save the filter graph to that file.
// ----------------------------------------------------------------------

HRESULT DepthCameraInput::SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) {
	const WCHAR wszStreamName[] = L"ActiveMovieGraph";
	HRESULT hr;
	IStorage *pStorage = NULL;

	// First, create a document file which will hold the GRF file
	hr = StgCreateDocfile(
		wszPath,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, &pStorage);
	if (FAILED(hr))
	{
		return hr;
	}

	// Next, create a stream to store.
	IStream *pStream;
	hr = pStorage->CreateStream(
		wszStreamName,
		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStream);
	if (FAILED(hr))
	{
		pStorage->Release();
		return hr;
	}

	// The IPersistStream converts a stream into a persistent object.
	IPersistStream *pPersist = NULL;
	pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
	hr = pPersist->Save(pStream, 1);
	pStream->Release();
	pPersist->Release();
	if (SUCCEEDED(hr))
	{
		hr = pStorage->Commit(STGC_DEFAULT);
	}
	pStorage->Release();
	return hr;
}

int DepthCameraInput::OpenById(int id, int openCmdFalg)
{
	int n = listVideos(1);
	if (n == 0)
		return 0;
	if (id < 0 || id > n - 1)
		return 0;
	OpenCmdFalg = openCmdFalg;
    char * name = getVideoName(id);
	if (openCmdFalg && name != NULL)
	{
		startCmd(name);
		IsUpdatingFlag = 0;
	}
	/*
	TOF 3D雷达             : ILD38TOF
	低分辨率小型化3D摄像头 : IDC8060S
	高分辨率小型化3D摄像头 : IDC3224S
	RGBD摄像头             : IRDC3224 
	*/
	if (strstr(name, "IDC8060S") != NULL)
	{
		W_focal = 1.62f / 0.03f;  // Focal Length 1.62mm / pixel size 0.03mm
		H_focal = W_focal;
	}
	else if(strstr(name, "ILD38TOF") != NULL)
	{
		W_focal = 6.0f / 0.03f;  // Focal Length 6mm / pixel size 0.03mm
		H_focal = W_focal;
	}
	else if (strstr(name, "IDC3224S") != NULL || strstr(name, "IRDC3224") != NULL)
	{
		W_focal = 3.6f / 0.015f;  // Focal Length 3.6mm / pixel size 0.015mm
		H_focal = W_focal;
	}
	else if (strstr(name, "IDC3224L") != NULL)
	{
		W_focal = 3.6f / 0.015f;  // Focal Length 3.6mm / pixel size 0.015mm    //W_focal = 2.3f / 0.015f;  // Focal Length 2.3mm / pixel size 0.015mm
		H_focal = W_focal;
	}
	else  //default value
	{
		W_focal = 247.0f;
		H_focal = W_focal;
	}
	int res = setupVideo(id);
	if (res == 1)
	{
		int len = DepthCameraCmd::getCameraConfig(ConfigBuffer);
		if (len > 100)
		{
			char *str;

			char * FocalLengthX_Str = strstr((char*)ConfigBuffer, "FocalLength_X ");
			char * FocalLengthY_Str = strstr((char*)ConfigBuffer, "FocalLength_Y ");
			char * PrincipalPointX_Str = strstr((char*)ConfigBuffer, "PrincipalPoint_X ");
			char * PrincipalPointY_Str = strstr((char*)ConfigBuffer, "PrincipalPoint_Y ");
			char * RadialDistortion_K1 = strstr((char*)ConfigBuffer, "RadialDistortion_K1 ");
			char * RadialDistortion_K2 = strstr((char*)ConfigBuffer, "RadialDistortion_K2 ");

			W_focal = strtof((FocalLengthX_Str + sizeof("FocalLength_X")), &str);
			H_focal = strtof((FocalLengthY_Str + sizeof("FocalLength_Y")), &str);
			PrincipalPoint_X = strtof((PrincipalPointX_Str + sizeof("PrincipalPoint_X")), &str);
			PrincipalPoint_Y = strtof((PrincipalPointY_Str + sizeof("PrincipalPoint_Y")), &str);
		}


		FrameHeight = getFrameHeight(id);
		FrameWidth = getFrameWidth(id);

		if (len <= 100)
		{
			PrincipalPoint_X = FrameWidth / 2.0f;
			PrincipalPoint_Y = FrameHeight / 2.0f;
		}



		Table = Common::CreateD2PTable(FrameWidth, FrameHeight, W_focal, H_focal, PrincipalPoint_X, PrincipalPoint_Y);
		DDI.InitDepthDenoise(FrameWidth,FrameHeight);
	}
	return res;
}

void DepthCameraInput::Close(int id)
{
	stopVideo(id);
	if (isCmdStart())
		stopCmd();
	if (Table)
		delete Table;
}

int DepthCameraInput::updateFirmwareStart(int deviceID, char* path)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	IsUpdatingFlag = 1;
	int res = -1;
	if (isVideoSetup(deviceID))
	{
		isVideoClosed = 1;
		stopVideo(deviceID);
	}
	if (isCmdStart() == 0)
	{
		int n = listVideos(1);
		if (n == 0)
			return 0;
		if (deviceID < 0 || deviceID > n - 1)
			return 0;
		char * name = getVideoName(deviceID);
		startCmd(name);
	}
	if (isCmdStart() == 1)
	{
		UpdateProgress = 0; 
		
		if (path != NULL)
		{
			FILE * readFile = NULL;
			SendFileSize = 0;
			SendOffset = 0;
			memset(FilePath, 0, sizeof(FilePath));
			strcpy_s(FilePath, (char*)path);
			Common::WriteToLogFile("Self : %s\r\n", FilePath);
			if (fopen_s(&readFile, FilePath, "rb") == 0)
			{
				fseek(readFile, 0, SEEK_END);
				SendFileSize = ftell(readFile);
				fclose(readFile);
				if (SendFileSize > 0 && (SendFileSize % 4) == 0)
				{
					//提取文件名
					char * Extension = strrchr(FilePath, '.');
					uint32_t CmdLen = 0;
					int ack_len = 0;
					if (memcmp(Extension, ".ifw", 4) == 0)
						CmdLen = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "wf a 0x%x\r\n", SendFileSize);
					else if (memcmp(Extension, ".mfw", 4) == 0)
						CmdLen = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "wf o 0x%x\r\n", SendFileSize);
					else if (memcmp(Extension, ".cfg", 4) == 0)
						CmdLen = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "wf c 0x%x\r\n", SendFileSize);
					else if (memcmp(Extension, ".dfg", 4) == 0)
						CmdLen = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "wf d 0x%x\r\n", SendFileSize);
					else
						res = -2;   //文件名不对

					int timeout = 0;
					
					while (1)
					{
						sendOrWaitCmd(CmdBuffer, CmdLen, 5000, AckBuffer, &ack_len);
						if (ack_len > 0 && strstr((char*)AckBuffer, "Ready") != NULL)
						{
							res = 0;
							break;
						}
						else {
							timeout++;
							if (timeout >= 3) {
								res = -3;   //超时
								break;
							}
						}
					}
				}
				else
					res = -4;  //文件内容为空
			}
			else
				res = -5;  //打开文件失败
		}
		else
			res = -6;  //文件路口为空
		if (res == 0)
		{
			UpdateVideoID = deviceID;
			hDownloadFirmwareThread = CreateThread(NULL, 0, UpdateFirmwareThread, this, 0, NULL);
			return 0;
		}
	}
	IsUpdatingFlag = 0;
	if (isVideoClosed == 1)
	{
		isVideoClosed = 0;
		OpenById(deviceID, OpenCmdFalg);
	}
	return res;
}


uint32_t DepthCameraInput::LoadSendData(int end)
{
	CRC32 myCRC32;
	uint32_t CRC32Value = 0;
	uint32_t Offset = 0xFFFFFFFF;
	uint32_t SendSize = 0;
	uint32_t SendDataLen = 0;
	uint32_t *iptr = (uint32_t*)SendDataBuffer;
	char* filePath = FilePath;
	if (end == 0)
	{
		FILE *readFile = NULL;
		if (fopen_s(&readFile, filePath, "rb") == 0)
		{
			memset(SendDataBuffer, 0xFF, sizeof(SendDataBuffer));
			SendDataLen = (SendFileSize - SendOffset > SECTION_SIZE) ? SECTION_SIZE : (SendFileSize - SendOffset);
			fseek(readFile, SendOffset, SEEK_SET);
			fread((uint8_t *)(SendDataBuffer + 8), SendDataLen, 1, readFile);
			fclose(readFile);
			CRC32Value = myCRC32.CalcStm32CRC(SendDataBuffer, 8, SendDataLen);
			Offset = SendOffset;
			SendOffset += SendDataLen;
			SendSize = SendDataLen;
		}
		else
			return 0;

	}
	*iptr = Offset;
	*(iptr + 1) = SendSize;
	*(iptr + (SendSize / 4) + 2) = CRC32Value;
	return (SendDataLen + 12);

}

void DepthCameraInput::UpdateFirmwareLoop()
{
	uint32_t SendOkSize = 0, LoadSendSize = 0;
	uint8_t TimeOut = 0;
	uint8_t  Error = 0, SendEndFlag = 0, LoadSendFlag = 0;
	int ack_len = 0;
	IsUpdatingFlag = 1;
	while (IsUpdatingFlag)
	{
		if (LoadSendFlag == 0)
		{
			LoadSendSize = LoadSendData(SendEndFlag);
			if (LoadSendSize >= 12)
			{
				LoadSendFlag = 1;
				sendOrWaitCmd(SendDataBuffer, LoadSendSize, 1000, AckBuffer, &ack_len);
				AckBuffer[ack_len] = 0;
			}
			else
				LoadSendFlag = 0;
		}
		if (ack_len > 0)
		{
			TimeOut = 0;
			if (strstr((char*)AckBuffer, "Ok") != NULL && strstr((char*)AckBuffer, "Fail") == NULL)
			{
				Error = 0;
				if (SendEndFlag == 0)
				{
					SendOkSize += (LoadSendSize - 12);
					if (SendOkSize != SendFileSize)
						SendEndFlag = 0;
					else
						SendEndFlag = 1;
					LoadSendFlag = 0;
					UpdateProgress = ((int)((float)SendOkSize / SendFileSize * 100.0) - 1);
				}
				else
				{
					UpdateProgress = 100;
					break;
				}
			}
			else
			{
				Error++;
				if (Error > 5)
					break;
				if (LoadSendFlag)
					sendOrWaitCmd(SendDataBuffer, LoadSendSize, 1000, AckBuffer, &ack_len);

			}
		}
		else
		{
			TimeOut++;
			if (TimeOut > 5)
				break;
			if (LoadSendFlag)
				sendOrWaitCmd(SendDataBuffer, LoadSendSize, 1000, AckBuffer, &ack_len);
		}
	}
	if (IsUpdatingFlag)
		updateFirmwareStop(UpdateVideoID);
}


void DepthCameraInput::updateFirmwareStop(int deviceID)
{ 
	IsBusyFlag = 0;
	IsUpdatingFlag = 0;
	if (isVideoClosed == 1)
	{
		isVideoClosed = 0;
		OpenById(deviceID, OpenCmdFalg);
	}
	if (hDownloadFirmwareThread != NULL)
	{
		WaitForSingleObject(hDownloadFirmwareThread, INFINITE);
		CloseHandle(hDownloadFirmwareThread);
		hDownloadFirmwareThread = NULL;
	}
}

void DepthCameraInput::DepthToPointCloud(int id, unsigned short * phase, float * pc)
{
	Common::DepthToPointCloud(phase, pc, FrameWidth, FrameHeight, W_focal, H_focal, PrincipalPoint_X, PrincipalPoint_Y);
}

int DepthCameraInput::DepthToPointCloud(int id,  unsigned short *phase, unsigned short *amplitude, unsigned short phaseMax, unsigned short amplitudeMin, float *pc, float *pcc)
{
	return Common::DepthToPointCloud(phase, amplitude, phaseMax, amplitudeMin, pc, pcc, FrameWidth, FrameHeight, W_focal, H_focal,PrincipalPoint_X, PrincipalPoint_Y);
}

void DepthCameraInput::PhaseDenoise(unsigned short * phase, unsigned short * amplitude, unsigned char * flags, unsigned short * DstFrame, int Amp_Thr)
{
	DDI.Depth_Denoise(FrameWidth, FrameHeight, phase, amplitude, flags, DstFrame, Amp_Thr);
}
