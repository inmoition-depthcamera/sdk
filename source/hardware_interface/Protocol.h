
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "inttypes.h"

enum ProtocolType
{
	PT_BUFFER,
	PT_SERAL_PACKAGE
};

typedef void(*HardwareInterfaceCallBack)(int32_t id, void *param, const uint8_t *buffer, int32_t len);

typedef struct
{
	int id;
	HardwareInterfaceCallBack func;
	void *param;
}CallBackInfo;

#define MAX_CALL_BACK_NUM 16

class Protocol
{
public:
	Protocol(int32_t interface_id, uint32_t max_package_size);
	~Protocol();

	static Protocol * CreateProtocol(int32_t interface_id, ProtocolType type, uint32_t max_pakage_size = 4096);
	
	virtual int PushBuffer(uint8_t * points, int32_t len) = 0;
	virtual ProtocolType GetProtocolType() = 0;
	virtual const uint8_t * PackageBuffer(uint8_t *points, int32_t in_len, int32_t *out_len) = 0;

	int AddCallBack(HardwareInterfaceCallBack func, void * param);
	int RemoveCallBack(int id);

protected:

	int32_t mMaxPackageSize;
	int32_t mPackageSize;
	uint8_t *mRxBuffer;
	uint8_t *mTxBuffer;
	int32_t mInterfaceId;

	int32_t mCallBackInfoArraySize;

	CallBackInfo mCallBackInfoArray[MAX_CALL_BACK_NUM];	
};


#endif
