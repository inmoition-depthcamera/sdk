
#ifndef __VIDEO_INTERFACE_H__
#define __VIDEO_INTERFACE_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <atomic>
#include <functional>

class VideoInterface
{
public:
	VideoInterface();
	~VideoInterface();

	virtual bool GetCameraList(std::vector<std::string> &camera_list, const char *filter) = 0;
	virtual bool Open(std::string &camera_name) = 0;
	virtual bool Close() = 0;

	inline int32_t GetWidth() { return mVideoWidth; }
	inline int32_t GetHeight() { return mVideoHeight; }
	
	bool IsOpened() { return mIsVideoOpened.load(); }

protected:
	// This function should been called before open
	void SetVideoFrameCallBack(std::function<void(double sample_time, uint8_t *frame_buf, int32_t frame_buf_len, void * param)> cb, void *param);

	std::atomic<bool> mIsVideoOpened;
	std::function<void(double, uint8_t *, int32_t, void *)> mFrameCallBack;
	void * mFrameCallBackParam;

	int32_t mVideoWidth;
	int32_t mVideoHeight;

	std::atomic<bool> mHasNewFrame;
};

#endif