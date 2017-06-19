#include <iostream>
#include "cmd_interface.h"

#define MAX_ACK_BUF_LEN 192000

CmdInterface::CmdInterface()
{
	mIsCmdOpened = false;
	mRxThreadExitFlag = true;
	mRxCallBack = NULL;
	mRxCallBackParam = NULL;

	mResponseBuffer = new char[MAX_ACK_BUF_LEN];
	mCmdStrProcessBuffer = new char[MAX_ACK_BUF_LEN];
	mResponseBufferLen = 0;
	mCmdStringProcessOffset = 0;
	mRxThread = NULL;
	mVideoMode = false;
	mRxCount = 0;
	mErrorCnt = 0;
}

CmdInterface::~CmdInterface()
{
	delete[] mCmdStrProcessBuffer;
	delete[] mResponseBuffer;
}

bool CmdInterface::IsOpened()
{
	return mIsCmdOpened.load();
}

int32_t CmdInterface::SendCmd(const char * cmd_buf, int32_t size)
{
	if (mIsCmdOpened.load()) {
		uint32_t writed = 0;
		bool res = WriteToIo((const uint8_t *)cmd_buf, size, &writed);
		return res ? (int32_t)writed : -1;
	}
	return -1;
}

int32_t CmdInterface::SendCmdAndWaitResponse(const char * cmd_buf, int32_t cmd_len, int32_t timeout, char * ack_buffer, int32_t ack_buf_len, const char *keyword)
{
	if (mIsCmdOpened.load()) {
		uint32_t writed = 0;
		bool res = WriteToIo((const uint8_t *)cmd_buf, cmd_len, &writed);
		if (res) {
			std::unique_lock <std::mutex> lck(mRxEventMutex);
			do {
				if (mRxEvent.wait_for(lck, std::chrono::milliseconds(timeout)) != std::cv_status::timeout) {
					int32_t cplen = -1;
					mMutex.lock();
					if (mResponseBufferLen > 0 && ack_buffer) {
						cplen = ack_buf_len > mResponseBufferLen ? mResponseBufferLen : ack_buf_len;
						memcpy(ack_buffer, mResponseBuffer, cplen);
						ack_buffer[cplen] = 0;
					}
					mMutex.unlock();

					if (keyword) {
						if (!strstr(ack_buffer, keyword))
							continue;
					}
					return cplen;
				}else
					break;
			} while (1);
			
		}
	}
	return -1;
}

int32_t CmdInterface::SetRxDataCallBack(std::function<void(const uint8_t*, int32_t, void *)> cb, void * param)
{
	mRxCallBack = cb;
	mRxCallBackParam = param;
	return 0;
}

void CmdInterface::mRxThreadProc(void * param)
{
	CmdInterface *cmd_if = (CmdInterface *)param;
	char * rx_buf = new char[MAX_ACK_BUF_LEN + 1];
	while (!cmd_if->mRxThreadExitFlag.load()) {
		uint32_t readed = 0;
		bool res = cmd_if->ReadFromIO((uint8_t *)rx_buf, MAX_ACK_BUF_LEN, &readed);
		if (res && readed) {
			cmd_if->mRxCount += readed;
			if (cmd_if->mVideoMode) {
				if (cmd_if->mVideoModeRxDataCallBack)
					cmd_if->mVideoModeRxDataCallBack((uint8_t *)rx_buf, readed, cmd_if->mVideoModeRxDataCallBackParam);
			} else {
				cmd_if->ProcessCmdStr(rx_buf, readed);
			}			
		}
	}

	delete []rx_buf;
}

bool CmdInterface::ProcessCmdStr(const char * in_buf, uint32_t rx_buf_len)
{
	if (mRxCallBack)
		mRxCallBack((uint8_t *)in_buf, rx_buf_len, mRxCallBackParam);

	// remove padding 0
	while (in_buf[rx_buf_len - 1] == 0)
		rx_buf_len--;

	if (mCmdStringProcessOffset + rx_buf_len > (MAX_ACK_BUF_LEN - 1))
		mCmdStringProcessOffset = 0;

	memcpy(mCmdStrProcessBuffer + mCmdStringProcessOffset, in_buf, rx_buf_len);
	mCmdStringProcessOffset += rx_buf_len;
	mCmdStrProcessBuffer[mCmdStringProcessOffset] = 0;
	char *str1 = strstr(mCmdStrProcessBuffer, "idcs>");
	if (str1 != NULL) {
		char *str2 = str1 + 5;
		while ((str1 > mCmdStrProcessBuffer) && (*str1 != '\n'))
			str1--;
		
		if(str1 == mCmdStrProcessBuffer){
			mCmdStringProcessOffset = 0;
			return false;
		}

		mMutex.lock();
		mResponseBufferLen = str1 - mCmdStrProcessBuffer + 1;
		memcpy(mResponseBuffer, mCmdStrProcessBuffer, mResponseBufferLen);
		mResponseBuffer[mResponseBufferLen] = 0;
		mMutex.unlock();

		// copy reset to head
		mCmdStringProcessOffset = mCmdStringProcessOffset - (str2 - mCmdStrProcessBuffer);
		if (mCmdStringProcessOffset){
			for(int i = 0 ; i < mCmdStringProcessOffset ; i ++){
				mCmdStrProcessBuffer[i] = str2[i];
			}
			//memcpy(mCmdStrProcessBuffer, str2, mCmdStringProcessOffset);

			// notify ack event
			mCmdStrProcessBuffer[mCmdStringProcessOffset] = 0;
		}
			
		mRxEvent.notify_all();
	}
	else if (mCmdStringProcessOffset >= MAX_ACK_BUF_LEN) {
		mCmdStringProcessOffset = 0;
	}

	return false;
}
