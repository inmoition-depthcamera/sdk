#ifndef __CMD_INTERFACE_H__
#define __CMD_INTERFACE_H__

#include <thread>
#include <inttypes.h>
#include <atomic>
#include <mutex>
#include <functional>


#include "std_event.h"

using namespace std;

class CmdInterface
{
public:
	CmdInterface();
	~CmdInterface();

	virtual int Open(string port_name) = 0;
	virtual int Close() = 0;

	bool IsOpened();

	int32_t SendCmd(const char * read_buffer, int32_t size);
	int32_t SendCmdAndWaitResponse(const char *cmd_buffer, int32_t cmd_buffer_len, int32_t timeout, char *res_buffer, int32_t res_buf_len);

	int32_t SetRxDataCallBack(std::function<void(unsigned char *, int, void *)> cb, void *param);

protected:
	string mPortName;

	std::function<void(unsigned char *, int, void *)> mRxCallBack;
    void *mRxCallBackParam;
    std::thread mRxThread;
    static void mRxThreadProc(void *param);
	std::atomic<bool> mRxThreadExitFlag;


	std::atomic<bool> mIsOpened;
	
	std_event mAckEvent;
	std::mutex mMutex;

	char *ResponseBuffer;
	int32_t ResponseBufferLen;

	virtual bool ReadFromIO(uint8_t *rx_buf, uint32_t rx_buf_len, uint32_t *rx_len) = 0;
	virtual bool WriteToIo(const uint8_t *tx_buf, uint32_t tx_buf_len, uint32_t *tx_len) = 0;
	
};

#endif