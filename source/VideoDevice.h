
#ifndef __VIDEO_DEVICE_H__
#define __VIDEO_DEVICE_H__

#include "dshow.h"
#include "GrabberCB.h"

class VideoDevice {
public:

	VideoDevice();
	void setSize(int w, int h, int pixel_size);
	void NukeDownstream(IBaseFilter *pBF);
	void destroyGraph();
	~VideoDevice();

	int videoSize;
	int pixelSize;
	int width;
	int height;

	int tryWidth;
	int tryHeight;
	GUID tryVideoType;

	ICaptureGraphBuilder2 *pCaptureGraph;    // Capture graph builder object
	IGraphBuilder *pGraph;                    // Graph builder object
	IMediaControl *pControl;                // Media control object
	IBaseFilter *pVideoInputFilter;          // Video Capture filter
	IBaseFilter *pGrabberF;
	IBaseFilter * pDestFilter;
	IAMStreamConfig *streamConf;
	ISampleGrabber * pGrabber;                // Grabs frame
	AM_MEDIA_TYPE * pAmMediaType;

	IMediaEventEx * pMediaEvent;

	GUID videoType;
	long formatType;

	GrabberCB * sgCallback;

	bool tryDiffSize;
	bool tryDiffVideoType;

	bool readyToCapture;
	bool sizeSet;
	bool setupStarted;
	bool specificFormat;
	bool autoReconnect;
	int  nFramesForReconnect;
	unsigned long nFramesRunning;
	int  connection;
	int  myID;
	long requestedFrameTime; //ie fps

	char  nDeviceName[255];
	WCHAR wDeviceName[255];

	unsigned char * pixels;
	char * pBuffer;

	static bool verbose;

};

#endif
