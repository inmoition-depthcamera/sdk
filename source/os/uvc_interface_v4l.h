
#ifndef __UVC_INTERFACE_V4L_H__
#define __UVC_INTERFACE_V4L_H__

#include "depth_video_interface.h"
#include <linux/videodev2.h>
#include <thread>

#define NB_BUFFER 32
class UvcInterfaceV4L:public DepthVideoInterface
{
public:
	UvcInterfaceV4L();
	~UvcInterfaceV4L();

	// Inherited via VideoInterface
	virtual bool GetDepthCameraList(vector<string>& camera_list);
	virtual bool Open(std::string & camera_name) override;
	virtual bool Close() override;

private:
	bool InitV4L();
	int32_t StartStream();
	int32_t StopStream();
	virtual bool GetCameraList(std::vector<std::string>& camera_list, const char * filter) override;

	std::thread *mReadFrameThread;
	std::atomic<bool> mReadFrameThreadExitFlag;
	static void ReadFrameThreadProc(UvcInterfaceV4L *param);

	int32_t mFd;
	std::string mDeviceName;
	void *mMemBuffers[NB_BUFFER];

};

#endif