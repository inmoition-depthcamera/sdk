#include "BufferProtocol.h"


BufferProtocol::BufferProtocol(int32_t interface_id, uint32_t maxPackageSize):Protocol(interface_id, maxPackageSize)
{

}


BufferProtocol::~BufferProtocol()
{

}

int BufferProtocol::PushBuffer(uint8_t * points, int32_t len)
{
	for (int i = 0; i < mCallBackInfoArraySize; i++)
	{
		CallBackInfo *pcbi = &mCallBackInfoArray[i];
		if (pcbi->func)
			pcbi->func(mInterfaceId, pcbi->param, points, len);
	}

	return len;
}

const uint8_t * BufferProtocol::PackageBuffer(uint8_t * points, int32_t in_len, int32_t * out_len)
{
	*out_len = in_len;
	return points;
}

