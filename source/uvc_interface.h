
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

	virtual bool GetUvcCameraList(std::vector<std::string> &camera_list) = 0;
	virtual bool Open(std::string camera_name) = 0;
	virtual bool Close() = 0;
	virtual bool GetFrame(uint8_t *buf, uint32_t buf_len) = 0;

	inline bool GetWidth() { return mUvcWidth; }
	inline bool GetHeight() { return mUvcHeight; }
	
	bool IsOpened() { return mIsOpened.load(); }
	void SetFrameCallBack(std::function<void(uint8_t *frame_buf, uint32_t frame_buf_len, void * param)> cb, void *param);

private:
	std::atomic<bool> mIsOpened;
	std::function<void(uint8_t *, uint32_t, void *)> mFrameCallBack;
	void * mFrameCallBackParam;

	int32_t mUvcWidth;
	int32_t mUvcHeight;
};

#endif