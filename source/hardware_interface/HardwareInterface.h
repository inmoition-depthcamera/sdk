
#ifndef __HARRWARE_INTERFACE_H__
#define __HARRWARE_INTERFACE_H__


#include "Protocol.h"
#include "Thread.h"
#include "CriticalSection.h"

#define MAX_PROTOCOL_NUM 4

enum InterfaceType
{
	SERIAL_PORT,
	TCP_SERVER,
	TCP_CLIENT,
	UDP
};


class HardwareInterface
{
public:
	HardwareInterface();
	~HardwareInterface();

	virtual int Open(char *file_name, uint32_t param1, uint32_t param2) = 0;
	virtual int Close() = 0;
	virtual int ReOpen() = 0;
	virtual InterfaceType GetInterfaceType() = 0;
	
	int32_t GetID() { return mId; }

	int AddRxCallBack(ProtocolType type, HardwareInterfaceCallBack func, void *param);
	int RemoveRxCallBack(ProtocolType type, int id);

	int WriteData(ProtocolType type, uint8_t * points, int32_t len);

	int IsOpened() { return mIsOpened; }

	uint64_t GetTotalRxByteCount() { return mTotalRxBytes; }
	uint64_t GetTotalTxByteCount() { return mTotalTxBytes; }
	void ResetCounters();

protected:

	int mIsOpened, mSysErrCode;
	Thread mRxThread;
	char mFileName[128];

private:

	CriticalSection mCS;

	int32_t mId;
	static int32_t mIdCounter;

	Protocol *mProtocolArray[MAX_PROTOCOL_NUM];
	int32_t mProtocolArraySize;

	uint64_t mTotalRxBytes, mTotalTxBytes;
	
	virtual int32_t ReadBufferFromInterface(uint8_t *byBuf, int32_t dwLen, int32_t dwTimeOut = -1) = 0;
	virtual int32_t WriteBufferToInterface(const uint8_t *byBuf, int32_t dwLen, int32_t dwTimeOut = -1) = 0;

	static int RxThreadLoop(void *param);
	
};


#endif