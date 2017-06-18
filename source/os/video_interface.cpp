#include "video_interface.h"

VideoInterface::VideoInterface()
{
	mIsVideoOpened = false;
	mHasNewFrame = false;
	mFrameCallBack = NULL;
	mFrameCallBackParam = NULL;
}

VideoInterface::~VideoInterface()
{
}

void VideoInterface::SetVideoFrameCallBack(std::function<void(double sample_time, uint8_t*frame_buf, int32_t frame_buf_len, void*param)> cb, void * param)
{
	mFrameCallBack = cb;
	mFrameCallBackParam = param;
}
