#include "cmd_interface.h"

#define MAX_ACK_BUF_LEN 4096

CmdInterface::CmdInterface()
{
	mIsOpened = false;
	mRxThreadExitFlag = true;
	mRxCallBack = NULL;
	mRxCallBackParam = NULL;

	ResponseBuffer = new char[MAX_ACK_BUF_LEN];
	ResponseBufferLen = 0;
	mRxThread = NULL;
}

CmdInterface::~CmdInterface()
{
	delete[] ResponseBuffer;
}

bool CmdInterface::IsOpened()
{
	return mIsOpened.load();
}

int32_t CmdInterface::SendCmd(const char * cmd_buf, int32_t size)
{
	if (mIsOpened.load()) {
		uint32_t writed = 0;
		bool res = WriteToIo((const uint8_t *)cmd_buf, size, &writed);
		return res ? (int32_t)writed : -1;
	}
	return -1;
}

int32_t CmdInterface::SendCmdAndWaitResponse(const char * cmd_buf, int32_t cmd_len, int32_t timeout, char * ack_buffer, int32_t ack_buf_len)
{
	if (mIsOpened.load()) {
		uint32_t writed = 0;
		bool res = WriteToIo((const uint8_t *)cmd_buf, cmd_len, &writed);
		if (res) {
			std::unique_lock <std::mutex> lck(mRxEventMutex);
			if (mRxEvent.wait_for(lck, std::chrono::milliseconds(timeout)) != std::cv_status::timeout) {
				int32_t cplen = -1;
				mMutex.lock();				
				if (ResponseBufferLen > 0 && ack_buffer) {
					cplen = ack_buf_len > ResponseBufferLen ? ResponseBufferLen : ack_buf_len;
					memcpy(ack_buffer, ResponseBuffer, cplen);
					ack_buffer[cplen] = 0;
				}
				mMutex.unlock();
				return cplen;
			}
		}
	}
	return -1;
}

int32_t CmdInterface::SetRxDataCallBack(std::function<void(unsigned char *, int, void *)> cb, void * param)
{
	mRxCallBack = cb;
	mRxCallBackParam = param;
	return 0;
}

void CmdInterface::mRxThreadProc(void * param)
{
	CmdInterface *cmd_if = (CmdInterface *)param;
	char * rx_buf = new char[MAX_ACK_BUF_LEN + 1];
	uint32_t rx_offset = 0;
	while (!cmd_if->mRxThreadExitFlag.load()) {
		uint32_t readed = 0;
		bool res = cmd_if->ReadFromIO((uint8_t *)rx_buf + rx_offset, MAX_ACK_BUF_LEN - rx_offset, &readed);
		if (res) {			
			if (cmd_if->mRxCallBack)
				cmd_if->mRxCallBack((uint8_t *)rx_buf + rx_offset, readed, cmd_if->mRxCallBackParam);

			rx_offset += readed;
			char *str1 = NULL;
			// check for "/>"
			for (uint32_t i = 1; i < rx_offset; i++) {
				if (rx_buf[i - 1] == '/' && rx_buf[i] == '>') {
					str1 = rx_buf + i + 2;
					break;
				}
			}

			if (str1 != NULL) {
				char *str2 = str1;
				while (str1 > rx_buf && *str1 != '\n')
					str1--;
				cmd_if->mMutex.lock();
				cmd_if->ResponseBufferLen = str1 - rx_buf + 1;
				memcpy(cmd_if->ResponseBuffer, rx_buf, cmd_if->ResponseBufferLen);
				cmd_if->mMutex.unlock();
				
				// copy reset to head
				rx_offset = rx_offset - (str2 - rx_buf);
				if(rx_offset)
					memcpy(rx_buf, str2, rx_offset);
				// notify ack event
				cmd_if->mRxEvent.notify_all();
			}else if (rx_offset >= MAX_ACK_BUF_LEN) {
				rx_offset = 0;
			}
		}
	}

	delete []rx_buf;
}
