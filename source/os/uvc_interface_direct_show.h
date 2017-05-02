
#ifndef __UVC_INTERFACE_DIRECT_SHOW_H__
#define __UVC_INTERFACE_DIRECT_SHOW_H__

#include "uvc_interface.h"

#include "dshow.h"
#include <windows.h>

class UvcInterfaceDirectShow:public UvcInterface
{
public:
	UvcInterfaceDirectShow();
	~UvcInterfaceDirectShow();

	// Inherited via UvcInterface
	virtual bool GetUvcCameraList(std::vector<std::string> &camera_list, const char *filter) override;
	virtual bool Open(std::string &camera_name) override;
	virtual bool Close() override;

private:

	interface ISampleGrabberCB : public IUnknown
	{
		virtual HRESULT STDMETHODCALLTYPE SampleCB(
			double SampleTime,
			IMediaSample *pSample) = 0;

		virtual HRESULT STDMETHODCALLTYPE BufferCB(
			double SampleTime,
			BYTE *pBuffer,
			LONG BufferLen) = 0;

		virtual ~ISampleGrabberCB() {}
	};

	interface ISampleGrabber : public IUnknown
	{
		virtual HRESULT STDMETHODCALLTYPE SetOneShot(
			BOOL OneShot) = 0;

		virtual HRESULT STDMETHODCALLTYPE SetMediaType(
			const AM_MEDIA_TYPE *pType) = 0;

		virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(
			AM_MEDIA_TYPE *pType) = 0;

		virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(
			BOOL BufferThem) = 0;

		virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(
			LONG *pBufferSize,
			LONG *pBuffer) = 0;

		virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(
			IMediaSample **ppSample) = 0;

		virtual HRESULT STDMETHODCALLTYPE SetCallback(
			ISampleGrabberCB *pCallback,
			LONG WhichMethodToCallback) = 0;

		virtual ~ISampleGrabber() {}
	};

	//Callback class
	class GrabberCB : public ISampleGrabberCB {
	public:

		GrabberCB();
		virtual ~GrabberCB() {}
		STDMETHODIMP_(ULONG) AddRef() { return 1; }
		STDMETHODIMP_(ULONG) Release() { return 2; }
		STDMETHODIMP QueryInterface(REFIID, void **ppvObject);
		STDMETHODIMP SampleCB(double, IMediaSample *pSample) { return E_NOTIMPL; }
		STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, LONG BufferLen);

		void *mOnNewDataCallBackUserParam;
		std::function<void(double, BYTE *, LONG, void *)> mOnNewDataCallBack;

		UvcInterfaceDirectShow *mUvcIf;

	};

	static bool ComInit();
	static bool ComUnInit();
	static int32_t mComInitCount;

	HRESULT BindDevice(IBaseFilter** gottaFilter, const char * device_name, WCHAR *w_device_name);
	
	ICaptureGraphBuilder2 *mCaptureGraph;    // Capture graph builder object
	IGraphBuilder *mGraph;                    // Graph builder object
	IMediaControl *mControl;                // Media control object


	ISampleGrabber * mGrabber;                // Grabs frame
	GrabberCB * mGrabberCB;
};

#endif