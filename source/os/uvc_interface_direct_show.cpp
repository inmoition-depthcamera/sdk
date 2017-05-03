
#include "uvc_interface_direct_show.h"

int32_t UvcInterfaceDirectShow::mComInitCount = 0;

#ifdef DEBUG
#pragma comment(lib,"strmbasd.lib")
#else
#pragma comment(lib,"strmbase.lib")
#endif // DEBUG

DEFINE_GUID(CLSID_SampleGrabber, 0xc1f400a0, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
DEFINE_GUID(IID_ISampleGrabber,  0x6b652fff, 0x11fe, 0x4fce, 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f);
DEFINE_GUID(CLSID_NullRenderer,  0xc1f400a4, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);

UvcInterfaceDirectShow::UvcInterfaceDirectShow()
{
	mCaptureGraph = NULL;    // Capture graph builder object
	mGraph = NULL;    // Graph builder object
	mControl = NULL;    // Media control object
	mGrabber = NULL; // Grabs frame

	mGrabberCB = new GrabberCB();
	mGrabberCB->mUvcIf = this;
}

UvcInterfaceDirectShow::~UvcInterfaceDirectShow()
{
	Close();

	if (mGrabberCB) {
		mGrabberCB->Release();
		delete mGrabberCB;
	}
}

bool UvcInterfaceDirectShow::GetUvcCameraList(std::vector<std::string> &camera_list, const char *filter)
{
	ComInit();

	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));

	if (SUCCEEDED(hr)){
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK) {
			IMoniker *pMoniker = NULL;

			char *name_buf = new char[1024];
			camera_list.clear();

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
				if (SUCCEEDED(hr)){
					WideCharToMultiByte(CP_ACP, 0, str, -1, name_buf, 1024, NULL, NULL);

					IMalloc *pMalloc;
					hr = CoGetMalloc(1, &pMalloc);
					if (SUCCEEDED(hr))
						pMalloc->Free(str);
				}else {
					pMoniker->Release();
					bindCtx->Release();
					continue;
				}

				bindCtx->Release();

				int len = strlen(name_buf);
				name_buf[len++] = '_';
				name_buf[len++] = '_';
				// Find the description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr)) {
					int count = len;
					while (varName.bstrVal[count - len] != 0x00 && count < 1024) {
						name_buf[count] = (char)varName.bstrVal[count - len];
						count++;
					}

					if(count < 1024)
						name_buf[count] = 0;
					
					if(filter && strstr(name_buf, filter))
						camera_list.push_back(name_buf);
				}
				pPropBag->Release();
				pPropBag = NULL;
				pMoniker->Release();
				pMoniker = NULL;
			}
			delete[] name_buf;
			pEnum->Release();
			pEnum = NULL;
		}
	}

	pDevEnum->Release();
	pDevEnum = NULL;

	ComUnInit();

	return true;
}

bool UvcInterfaceDirectShow::Open(std::string &camera_name)
{
	HRESULT hr = NOERROR;

	ComInit();

#define CHECK_HR(x) do{ hr = (x); if (FAILED(hr)){ Close(); return false;}}while(0)

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&mCaptureGraph);
	if (FAILED(hr))    // FAILED is a macro that tests the return value
		return false;

	CHECK_HR(CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&mGraph));
	CHECK_HR(mCaptureGraph->SetFiltergraph(mGraph));
	CHECK_HR(mGraph->QueryInterface(IID_IMediaControl, (void **)&mControl));

	WCHAR w_device_name[256];
	IAMStreamConfig *stream_conf;
	AM_MEDIA_TYPE *pmt;
	IBaseFilter *video_input_filter, *grabber_filter, *dest_filter;          // Video Capture filter

	CHECK_HR(BindDevice(&video_input_filter, camera_name.c_str(), w_device_name));
	CHECK_HR(mGraph->AddFilter(video_input_filter, w_device_name));
	CHECK_HR(mCaptureGraph->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, video_input_filter, IID_IAMStreamConfig, (void **)&stream_conf));
	CHECK_HR(stream_conf->GetFormat(&pmt));

	VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
	mUvcWidth = HEADER(pVih)->biWidth;
	mUvcHeight = HEADER(pVih)->biHeight;

	pmt->formattype = FORMAT_VideoInfo;
	pmt->majortype = MEDIATYPE_Video;
	pmt->subtype = MEDIASUBTYPE_YUY2;
	pmt->lSampleSize = mUvcWidth * mUvcHeight * 2;
		
	CHECK_HR(stream_conf->SetFormat(pmt));

	if (pmt->cbFormat != 0) {
		CoTaskMemFree((PVOID)pmt->pbFormat);
		pmt->cbFormat = NULL;
		pmt->pbFormat = NULL;
	}
	if (pmt->pUnk) {
		pmt->pUnk->Release();
		pmt->pUnk = NULL;
	}

	CHECK_HR(CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&grabber_filter));
	CHECK_HR(mGraph->AddFilter(grabber_filter, L"Sample Grabber"));
	CHECK_HR(grabber_filter->QueryInterface(IID_ISampleGrabber, (void**)&mGrabber));

	//Set Params - One Shot should be 0 unless you want to capture just one buffer
	CHECK_HR(mGrabber->SetOneShot(0));
	CHECK_HR(mGrabber->SetBufferSamples(0));
	CHECK_HR(mGrabber->SetCallback(mGrabberCB, 1));

	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_YUY2;
	mt.formattype = FORMAT_VideoInfo;
	CHECK_HR(mGrabber->SetMediaType(&mt));

	CHECK_HR(CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)(&dest_filter)));
	CHECK_HR(mGraph->AddFilter(dest_filter, L"NullRenderer"));
	CHECK_HR(mCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, video_input_filter, grabber_filter, dest_filter));

	IMediaFilter *media_filter = NULL;
	hr = mGraph->QueryInterface(IID_IMediaFilter, (void**)&media_filter);
	if (SUCCEEDED(hr)) {
		media_filter->SetSyncSource(NULL);
		media_filter->Release();
		media_filter = NULL;
	}

	//Setup User Call Back
	mGrabberCB->mOnNewDataCallBack = mFrameCallBack;
	mGrabberCB->mOnNewDataCallBackUserParam = mFrameCallBackParam;

	CHECK_HR(mControl->Run());

	mIsOpened = true;

	stream_conf->Release();
	stream_conf = NULL;

	video_input_filter->Release();
	video_input_filter = NULL;

	grabber_filter->Release();
	grabber_filter = NULL;

	dest_filter->Release();
	dest_filter = NULL;

	return true;
}

bool UvcInterfaceDirectShow::Close()
{
	HRESULT HR = NOERROR;
#define SAFE_CHECK_RELEASE(x) if(x){(x)->Release(); (x) = NULL;}

	//Stop the callback and free it
	if (mGrabber){
		mGrabber->SetCallback(NULL, 1);
	}

	//Check to see if the graph is running, if so stop it.
	if (mControl){
		HR = mControl->Pause();
		HR = mControl->Stop();
	}

	SAFE_CHECK_RELEASE(mGrabber);
	SAFE_CHECK_RELEASE(mControl);

	//Release and zero our capture graph and our main graph
	SAFE_CHECK_RELEASE(mCaptureGraph);
	SAFE_CHECK_RELEASE(mGraph);

	mIsOpened = false;

	return true;
}

HRESULT UvcInterfaceDirectShow::BindDevice(IBaseFilter** gottaFilter, const char * device_name, WCHAR *w_device_name) {
	bool done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
		return hr;

	char *string_buf = new char[1024];

	// Obtain a class enumerator for the video input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
	if (hr == S_OK){
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done)){
			LPOLESTR str = NULL;
			IBindCtx* bindCtx = NULL;
			CreateBindCtx(0, &bindCtx);
			hr = pMoniker->GetDisplayName(bindCtx, NULL, &str);
			if (SUCCEEDED(hr)){
				WideCharToMultiByte(CP_ACP, 0, str, -1, string_buf, 1024, NULL, NULL);
				IMalloc *pMalloc;
				hr = CoGetMalloc(1, &pMalloc);
				if (SUCCEEDED(hr))
					pMalloc->Free(str);
				if (strstr(device_name, string_buf)) {
					IPropertyBag *pPropBag;
					hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr)) {
						int count = 0;
						while (varName.bstrVal[count] != 0x00) {
							w_device_name[count] = varName.bstrVal[count];
							count++;
						}
						w_device_name[count] = 0;

						// We found it, so send it back to the caller
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)gottaFilter);
						done = 1;
					}
					VariantClear(&varName);
					pPropBag->Release();
					pPropBag = NULL;

					done = true;
				}
			}
			bindCtx->Release();
		}
		pMoniker->Release();
		pMoniker = NULL;
		pEnumCat->Release();
		pEnumCat = NULL;
	}
	pSysDevEnum->Release();
	pSysDevEnum = NULL;
	delete [] string_buf;
	return done ? hr : VFW_E_NOT_FOUND;
}

bool UvcInterfaceDirectShow::ComInit()
{
	if (mComInitCount == 0) {
#ifdef VI_COM_MULTI_THREADED
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
		CoInitialize(NULL);
#endif
	}

	mComInitCount++;
	return true;
}

bool UvcInterfaceDirectShow::ComUnInit()
{
	if (mComInitCount > 0)
		mComInitCount--;        //decrease the count of instances using com

	if (mComInitCount == 0) {
		CoUninitialize();    //if there are no instances left - uninitialize com
		return true;
	}

	return false;
}

///////////////////////// Class for GrabberCB
UvcInterfaceDirectShow::GrabberCB::GrabberCB()
{
	mOnNewDataCallBack = NULL;
	mOnNewDataCallBackUserParam = NULL;
}

STDMETHODIMP UvcInterfaceDirectShow::GrabberCB::QueryInterface(REFIID, void ** ppv_object)
{
	*ppv_object = static_cast<ISampleGrabberCB*>(this);
	return S_OK;
}

STDMETHODIMP UvcInterfaceDirectShow::GrabberCB::BufferCB(double sample_time, BYTE * pbuffer, LONG buffer_len)
{
	if (mOnNewDataCallBack)
		mOnNewDataCallBack(sample_time, pbuffer, buffer_len, mOnNewDataCallBackUserParam);
	return S_OK;
}
