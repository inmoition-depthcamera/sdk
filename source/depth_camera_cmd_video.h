
#ifndef __DEPTH_CAMERA_CMD_VIDEO_H__
#define __DEPTH_CAMERA_CMD_VIDEO_H__

#include "depth_camera_cmd.h"
#include "depth_video_interface.h"

#define IDC_CMD_VIDEO_VERSION ((2 << 24) | (3 << 16) | 18) // 2.3.18

class DepthCameraCmdVideo: public DepthCameraCmdPort, public DepthVideoInterface
{
public:
	DepthCameraCmdVideo();
	~DepthCameraCmdVideo();

	// Inherited via DepthVideoInterface
	
	virtual bool GetDepthCameraList(vector<string>& camera_list) override;

	virtual bool Open(std::string &camera_name) override;
	virtual bool Close() override;
	bool IsOpened() { return mIsCmdVideoOpened; }
	bool VideoControl(bool start_stop);
	bool IsVideoOpened() { return mIsVideoOpened; }

private:
	virtual bool GetCameraList(std::vector<std::string>& camera_list, const char * filter) override;
	
	static void OnNewCmdDataCallBack(const uint8_t*, int32_t, void *);

	std::atomic<bool> mIsCmdVideoOpened, mIsVideoOpened;


	bool mHadFindHeader;
	int32_t mPackageHeaderCnt, mPackageTailCnt, mPackageSize;
	uint8_t *mRxBuffer;
};

#endif