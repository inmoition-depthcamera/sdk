#ifndef __CMD_INTERFACE_H__
#define __CMD_INTERFACE_H__

#include <thread>
#include <inttypes.h>
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include <string>
#include <condition_variable>
#include <string.h>

using namespace std;

class CmdInterface
{
public:
	CmdInterface();
	~CmdInterface();

	virtual bool Open(string &port_name) = 0;
	virtual bool Close() = 0;
	virtual bool GetUvcRelatedCmdPort(string &uvc_port_name, string &cmd_port_name) = 0;

	bool IsOpened();

	int64_t GetRxCount() { return mRxCount;}

	int32_t SendCmd(const char * read_buffer, int32_t size);
	int32_t SendCmdAndWaitResponse(const char *cmd_buffer, int32_t cmd_buffer_len, int32_t timeout, char *res_buffer, int32_t res_buf_len, const char *keyword);

	int32_t SetRxDataCallBack(std::function<void(const uint8_t *, int32_t, void *)> cb, void *param);

protected:
	string mPortName;
	int64_t mRxCount, mErrorCnt;

	std::function<void(const uint8_t *, int32_t, void *)> mRxCallBack;
    void *mRxCallBackParam;

	void SetVideoModeRxDataCallBack(std::function<void(const uint8_t *, int32_t, void *)> cb, void *param) {
		mVideoModeRxDataCallBack = cb;
		mVideoModeRxDataCallBackParam = param;
	}
	
	std::function<void(const uint8_t *, int32_t, void *)> mVideoModeRxDataCallBack;
	void *mVideoModeRxDataCallBackParam;

    std::thread *mRxThread;
    static void mRxThreadProc(void *param);
	std::atomic<bool> mRxThreadExitFlag;
	std::atomic<bool> mIsCmdOpened, mVideoMode;
	
	std::condition_variable mRxEvent;
	std::mutex mRxEventMutex;

	std::mutex mMutex;

	char *mResponseBuffer, *mCmdStrProcessBuffer;
	int32_t mResponseBufferLen, mCmdStringProcessOffset;

	virtual bool ReadFromIO(uint8_t *rx_buf, uint32_t rx_buf_len, uint32_t *rx_len) = 0;
	virtual bool WriteToIo(const uint8_t *tx_buf, uint32_t tx_buf_len, uint32_t *tx_len) = 0;
	virtual bool GetCmdDevices(std::vector<std::pair<std::string, std::string>> &device_list) = 0;

	bool ProcessCmdStr(const char *rx_buf, uint32_t rx_buf_len);
};

#endif