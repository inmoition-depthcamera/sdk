#include "VideoDevice.h"
#include "Common.h"

bool VideoDevice::verbose = false;

VideoDevice::VideoDevice() {

	pCaptureGraph = NULL;    // Capture graph builder object
	pGraph = NULL;    // Graph builder object
	pControl = NULL;    // Media control object
	pVideoInputFilter = NULL; // Video Capture filter
	pGrabber = NULL; // Grabs frame
	pDestFilter = NULL; // Null Renderer Filter
	pGrabberF = NULL; // Grabber Filter
	pMediaEvent = NULL;
	streamConf = NULL;
	pAmMediaType = NULL;

	//This is our callback class that processes the frame.
	sgCallback = new GrabberCB();
	sgCallback->newFrame = false;

	//Default values for capture type
	connection = PhysConn_Video_Composite;

	videoSize = 0;
	pixelSize = 0;
	width = 0;
	height = 0;

	tryWidth = 640;
	tryHeight = 480;
	nFramesForReconnect = 10000;
	nFramesRunning = 0;
	myID = -1;

	tryDiffSize = false;
	tryDiffVideoType = false;
	readyToCapture = false;
	sizeSet = false;
	setupStarted = false;
	specificFormat = false;
	autoReconnect = false;
	requestedFrameTime = -1;

	memset(wDeviceName, 0, sizeof(WCHAR) * 255);
	memset(nDeviceName, 0, sizeof(char) * 255);
}


// ----------------------------------------------------------------------
//    The only place we are doing new
//
// ----------------------------------------------------------------------

void VideoDevice::setSize(int w, int h, int pixel_size) {
	if (sizeSet) {
		if (verbose)printf("SETUP: Error device size should not be set more than once \n");
	}
	else
	{
		width = w;
		height = h;
		pixelSize = pixel_size;
		videoSize = w * h * pixel_size;
		sizeSet = true;
		pixels = new unsigned char[videoSize];
		pBuffer = new char[videoSize];

		memset(pixels, 0, videoSize);
		sgCallback->setupBuffer(w,h,videoSize);
	}
}


// ----------------------------------------------------------------------
//    Borrowed from the SDK, use it to take apart the graph from
//  the capture device downstream to the null renderer
// ----------------------------------------------------------------------

void VideoDevice::NukeDownstream(IBaseFilter *pBF) {
	IPin *pP, *pTo;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;
	HRESULT hr = pBF->EnumPins(&pins);
	pins->Reset();
	while (hr == NOERROR)
	{
		hr = pins->Next(1, &pP, &u);
		if (hr == S_OK && pP)
		{
			pP->ConnectedTo(&pTo);
			if (pTo)
			{
				hr = pTo->QueryPinInfo(&pininfo);
				if (hr == NOERROR)
				{
					if (pininfo.dir == PINDIR_INPUT)
					{
						NukeDownstream(pininfo.pFilter);
						pGraph->Disconnect(pTo);
						pGraph->Disconnect(pP);
						pGraph->RemoveFilter(pininfo.pFilter);
					}
					pininfo.pFilter->Release();
					pininfo.pFilter = NULL;
				}
				pTo->Release();
			}
			pP->Release();
		}
	}
	if (pins) pins->Release();
}


// ----------------------------------------------------------------------
//    Also from SDK
// ----------------------------------------------------------------------

void VideoDevice::destroyGraph() {
	HRESULT hr = 0;
	//int FuncRetval=0;
	//int NumFilters=0;

	int i = 0;
	while (hr == NOERROR)
	{
		IEnumFilters * pEnum = 0;
		ULONG cFetched;

		// We must get the enumerator again every time because removing a filter from the graph
		// invalidates the enumerator. We always get only the first filter from each enumerator.
		hr = pGraph->EnumFilters(&pEnum);
		if (FAILED(hr)) { if (verbose)printf("SETUP: pGraph->EnumFilters() failed. \n"); return; }

		IBaseFilter * pFilter = NULL;
		if (pEnum->Next(1, &pFilter, &cFetched) == S_OK)
		{
			FILTER_INFO FilterInfo;
			memset(&FilterInfo, 0, sizeof(FilterInfo));
			hr = pFilter->QueryFilterInfo(&FilterInfo);
			FilterInfo.pGraph->Release();

			int count = 0;
			char buffer[255];
			memset(buffer, 0, 255 * sizeof(char));

			while (FilterInfo.achName[count] != 0x00)
			{
				buffer[count] = (char)FilterInfo.achName[count];
				count++;
			}

			if (verbose)printf("SETUP: removing filter %s...\n", buffer);
			hr = pGraph->RemoveFilter(pFilter);
			if (FAILED(hr)) { if (verbose)printf("SETUP: pGraph->RemoveFilter() failed. \n"); return; }
			if (verbose)printf("SETUP: filter removed %s  \n", buffer);

			pFilter->Release();
			pFilter = NULL;
		}
		else break;
		pEnum->Release();
		pEnum = NULL;
		i++;
	}

	return;
}


// ----------------------------------------------------------------------
// Our deconstructor, attempts to tear down graph and release filters etc
// Does checking to make sure it only is freeing if it needs to
// Probably could be a lot cleaner! :)
// ----------------------------------------------------------------------

VideoDevice::~VideoDevice() {

	if (setupStarted) { if (verbose)printf("\nSETUP: Disconnecting device %i\n", myID); }
	else {
		if (sgCallback) {
			sgCallback->Release();
			delete sgCallback;
		}
		return;
	}

	HRESULT HR = NOERROR;

	//Stop the callback and free it
	if ((sgCallback) && (pGrabber))
	{
		pGrabber->SetCallback(NULL, 1);
		if (verbose)printf("SETUP: freeing Grabber Callback\n");
		sgCallback->Release();

		//delete our pixels
		if (sizeSet) {
			delete[] pixels;
			delete[] pBuffer;
		}

		delete sgCallback;
	}

	//Check to see if the graph is running, if so stop it.
	if ((pControl))
	{
		HR = pControl->Pause();
		if (FAILED(HR)) if (verbose)printf("ERROR - Could not pause pControl\n");

		HR = pControl->Stop();
		if (FAILED(HR)) if (verbose)printf("ERROR - Could not stop pControl\n");
	}

	//Disconnect filters from capture device
	if ((pVideoInputFilter))NukeDownstream(pVideoInputFilter);

	//Release and zero pointers to our filters etc
	if ((pDestFilter)) {
		if (verbose)printf("SETUP: freeing Renderer \n");
		(pDestFilter)->Release();
		(pDestFilter) = 0;
	}
	if ((pVideoInputFilter)) {
		if (verbose)printf("SETUP: freeing Capture Source \n");
		(pVideoInputFilter)->Release();
		(pVideoInputFilter) = 0;
	}
	if ((pGrabberF)) {
		if (verbose)printf("SETUP: freeing Grabber Filter  \n");
		(pGrabberF)->Release();
		(pGrabberF) = 0;
	}
	if ((pGrabber)) {
		if (verbose)printf("SETUP: freeing Grabber  \n");
		(pGrabber)->Release();
		(pGrabber) = 0;
	}
	if ((pControl)) {
		if (verbose)printf("SETUP: freeing Control   \n");
		(pControl)->Release();
		(pControl) = 0;
	}
	if ((pMediaEvent)) {
		if (verbose)printf("SETUP: freeing Media Event  \n");
		(pMediaEvent)->Release();
		(pMediaEvent) = 0;
	}
	if ((streamConf)) {
		if (verbose)printf("SETUP: freeing Stream  \n");
		(streamConf)->Release();
		(streamConf) = 0;
	}

	if ((pAmMediaType)) {
		if (verbose)printf("SETUP: freeing Media Type  \n");
		Common::DeleteMediaType(pAmMediaType);
	}

	if ((pMediaEvent)) {
		if (verbose)printf("SETUP: freeing Media Event  \n");
		(pMediaEvent)->Release();
		(pMediaEvent) = 0;
	}

	//Destroy the graph
	if ((pGraph))destroyGraph();

	//Release and zero our capture graph and our main graph
	if ((pCaptureGraph)) {
		if (verbose)printf("SETUP: freeing Capture Graph \n");
		(pCaptureGraph)->Release();
		(pCaptureGraph) = 0;
	}
	if ((pGraph)) {
		if (verbose)printf("SETUP: freeing Main Graph \n");
		(pGraph)->Release();
		(pGraph) = 0;
	}

	//delete our pointers
	delete pDestFilter;
	delete pVideoInputFilter;
	delete pGrabberF;
	delete pGrabber;
	delete pControl;
	delete streamConf;
	delete pMediaEvent;
	delete pCaptureGraph;
	delete pGraph;

	if (verbose)printf("SETUP: Device %i disconnected and freed\n\n", myID);
}
