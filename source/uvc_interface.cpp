#include "uvc_interface.h"

UvcInterface::UvcInterface()
{
	mIsOpened = false;
	mFrameCallBack = NULL;
	mFrameCallBackParam = NULL;
}

UvcInterface::~UvcInterface()
{
}

void UvcInterface::SetFrameCallBack(std::function<void(uint8_t*frame_buf, uint32_t frame_buf_len, void*param)> cb, void * param)
{
	mFrameCallBack = cb;
	mFrameCallBackParam = param;
}
