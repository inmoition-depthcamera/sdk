
#ifndef __GRABBER_CB_H__
#define __GRABBER_CB_H__

#include "dshow.h"
#include "FrameRate.h"

typedef void(__stdcall *OnDepthCameraFrameCallBack)(float angle, unsigned short * phase_buffer, unsigned short * amplitude_buffer, unsigned char * ambient_buffer, unsigned char * flags_buffer);

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

	//------------------------------------------------
	GrabberCB();
	//------------------------------------------------
	virtual ~GrabberCB();
	//------------------------------------------------
	bool setupBuffer(int w, int h, int numBytesIn);
	//------------------------------------------------
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	//------------------------------------------------
	STDMETHODIMP QueryInterface(REFIID, void **ppvObject);
	//This method is meant to have less overhead
	//------------------------------------------------
	STDMETHODIMP SampleCB(double, IMediaSample *pSample);
	//This method is meant to have more overhead
	STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, LONG BufferLen);


	void SetDepthCameraFrameCallBack(OnDepthCameraFrameCallBack cb) {
		userDepthCallBackFunc = cb;
	}

	int FrameWidth;
	int FrameHeight;
	float FrameAngle;
	unsigned short *PhaseBuffer;
	unsigned short *AmplitudeBuffer;
	unsigned char *AmbientBuffer;
	unsigned char *FlagsBuffer;

	int freezeCheck;

	int latestBufferLength;
	int numBytes;
	bool newFrame;
	bool bufferSetup;
	unsigned char * pixels;
	unsigned char * ptrBuffer;
	CRITICAL_SECTION critSection;
	HANDLE hEvent;

	OnDepthCameraFrameCallBack userDepthCallBackFunc;

	FrameRate fps;
};

#endif