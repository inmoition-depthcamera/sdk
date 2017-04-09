#include "GrabberCB.h"
#include "Common.h"

GrabberCB::GrabberCB() {
	InitializeCriticalSectionAndSpinCount(&critSection, 4000);
	freezeCheck = 0;

	bufferSetup = false;
	newFrame = false;
	latestBufferLength = 0;

	hEvent = CreateEvent(NULL, true, false, NULL);

	userDepthCallBackFunc = NULL;

    PhaseBuffer = NULL;
	AmplitudeBuffer = NULL;
	AmbientBuffer = NULL;
	FlagsBuffer = NULL;
}


//------------------------------------------------
GrabberCB::~GrabberCB() {
	ptrBuffer = NULL;
	DeleteCriticalSection(&critSection);
	CloseHandle(hEvent);
	if (bufferSetup) {
		delete[] pixels;
		
		delete[] PhaseBuffer;
		delete[] AmplitudeBuffer;
		delete[] AmbientBuffer;
		delete[] FlagsBuffer;
	}
}


//------------------------------------------------
bool GrabberCB::setupBuffer(int w , int h, int numBytesIn) {
	if (bufferSetup) {
		return false;
	}
	else {
		int size = w / 2 * (h - h % 10);
		numBytes = numBytesIn;
		pixels = new unsigned char[numBytes];
		bufferSetup = true;
		newFrame = false;
		latestBufferLength = 0;
		FrameHeight = h;
		FrameWidth = w;
		if (PhaseBuffer == NULL)
		{
			PhaseBuffer = new unsigned short[size];
		    AmplitudeBuffer = new unsigned short[size];
			AmbientBuffer = new unsigned char[size];
			FlagsBuffer = new unsigned char[size];
		}

	}
	return true;
}

//------------------------------------------------
STDMETHODIMP_(ULONG) GrabberCB::AddRef() { return 1; }
STDMETHODIMP_(ULONG) GrabberCB::Release() { return 2; }

//------------------------------------------------
STDMETHODIMP GrabberCB::QueryInterface(REFIID, void **ppvObject) {
	*ppvObject = static_cast<ISampleGrabberCB*>(this);
	return S_OK;
}

//This method is meant to have less overhead
//------------------------------------------------
STDMETHODIMP GrabberCB::SampleCB(double, IMediaSample *pSample) {
	return E_NOTIMPL;
}

//This method is meant to have more overhead
STDMETHODIMP GrabberCB::BufferCB(double SampleTime, BYTE *pBuffer, LONG BufferLen) {

	if (BufferLen == numBytes) {

		unsigned char * framePtr = pBuffer;
		EnterCriticalSection(&critSection);
		newFrame = true;
		//Split frame to 4 depth parts
		Common::SplitRawFrame((FrameWidth / 2), FrameHeight, framePtr, &FrameAngle, PhaseBuffer, AmplitudeBuffer, AmbientBuffer, FlagsBuffer);

		freezeCheck = 1;
		fps.Update();
		LeaveCriticalSection(&critSection);
		SetEvent(hEvent);

		if (userDepthCallBackFunc)
			userDepthCallBackFunc(FrameAngle, PhaseBuffer, AmplitudeBuffer, AmbientBuffer, FlagsBuffer);
	}

	return S_OK;
}