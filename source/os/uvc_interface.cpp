#include "uvc_interface.h"

UvcInterface::UvcInterface()
{
	mIsOpened = false;
	mHasNewFrame = false;
	mFrameCallBack = NULL;
	mFrameCallBackParam = NULL;
}

UvcInterface::~UvcInterface()
{
}

void UvcInterface::SetUvcFrameCallBack(std::function<void(double sample_time, uint8_t*frame_buf, uint32_t frame_buf_len, void*param)> cb, void * param)
{
	mFrameCallBack = cb;
	mFrameCallBackParam = param;
}
