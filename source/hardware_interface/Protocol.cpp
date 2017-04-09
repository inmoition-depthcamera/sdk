#include "Protocol.h"

#include "BufferProtocol.h"

Protocol::Protocol(int32_t interface_id, uint32_t maxPackageSize)
{
	mMaxPackageSize = maxPackageSize;
	mRxBuffer = new uint8_t[maxPackageSize];
	mTxBuffer = new uint8_t[maxPackageSize];

	for (int i = 0; i < MAX_CALL_BACK_NUM; i++)
	{
		mCallBackInfoArray[i].id = -1;
		mCallBackInfoArray[i].func = NULL;
	}

	mCallBackInfoArraySize = 0;
	mInterfaceId = interface_id;
}

Protocol::~Protocol()
{
	if (mRxBuffer)
		delete[] mRxBuffer;

	if (mTxBuffer)
		delete[] mTxBuffer;
}


Protocol * Protocol::CreateProtocol(int32_t interface_id, ProtocolType type, uint32_t max_pakage_size)
{
	switch (type)
	{
	case PT_BUFFER:
		return new BufferProtocol(interface_id, max_pakage_size);
		break;
	default:
		break;
	}
	return nullptr;
}

int Protocol::AddCallBack(HardwareInterfaceCallBack func, void * param)
{
	for (int i = 0; i < MAX_CALL_BACK_NUM; i++)
	{
		if (mCallBackInfoArray[i].id == -1)
		{
			mCallBackInfoArray[i].id = i;
			mCallBackInfoArray[i].func = func;
			mCallBackInfoArray[i].param = param;
			mCallBackInfoArraySize ++;
			return i;
		}
	}
	return -1;
}

int Protocol::RemoveCallBack(int id)
{
	for (int i = 0; i < MAX_CALL_BACK_NUM; i++)
	{
		if (mCallBackInfoArray[i].id == id)
		{
			for (int j = i; j < MAX_CALL_BACK_NUM - 1; j++)
				mCallBackInfoArray[j] = mCallBackInfoArray[j + 1];
			mCallBackInfoArray[MAX_CALL_BACK_NUM - 1].id = -1;
			mCallBackInfoArraySize--;
			return id;
		}
	}
	return -1;
}
