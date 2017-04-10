
#ifndef __UVC_INTERFACE_H__
#define __UVC_INTERFACE_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <atomic>
#include <functional>

class UvcInterface
{
public:
	UvcInterface();
	~UvcInterface();

	virtual bool GetUvcCameraList(std::vector<std::string> &camera_list, const char *filter) = 0;
	virtual bool Open(std::string &camera_name) = 0;
	virtual bool Close() = 0;

	inline int32_t GetWidth() { return mUvcWidth; }
	inline int32_t GetHeight() { return mUvcHeight; }
	
	bool IsOpened() { return mIsOpened.load(); }
	bool HasNewFrame() { return mHasNewFrame; }
	// This function should been called before open
	void SetUvcFrameCallBack(std::function<void(double sample_time, uint8_t *frame_buf, uint32_t frame_buf_len, void * param)> cb, void *param);

protected:
	std::atomic<bool> mIsOpened;
	std::function<void(double, uint8_t *, uint32_t, void *)> mFrameCallBack;
	void * mFrameCallBackParam;

	int32_t mUvcWidth;
	int32_t mUvcHeight;

	bool mHasNewFrame;
};

#endif