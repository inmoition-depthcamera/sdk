#include "HardwareInterface.h"

int32_t HardwareInterface::mIdCounter = 0;

HardwareInterface::HardwareInterface()
{
	ResetCounters();

	mRxThread.Create(RxThreadLoop, this);

	for (int i = 0; i < MAX_PROTOCOL_NUM; i++)
		mProtocolArray[i] = NULL;

	mId = mIdCounter++;

	mIsOpened = mSysErrCode = 0;
	mProtocolArraySize = 0;
	memset(mFileName, 0, sizeof(mFileName));
}

HardwareInterface::~HardwareInterface()
{
	mRxThread.Exit();
}

void HardwareInterface::ResetCounters()
{
	mTotalRxBytes = mTotalTxBytes = 0;
}

int HardwareInterface::RxThreadLoop(void * param)
{
	uint8_t buf[4096];
	int32_t readed;
	HardwareInterface *hif = (HardwareInterface *)param;
	readed = hif->ReadBufferFromInterface(buf, 4096);
	if (readed > 0){		
		hif->mTotalRxBytes += readed;
		hif->mCS.Enter();
		for (int32_t i = 0; i < hif->mProtocolArraySize; i++) 
		{
			Protocol *ptl = hif->mProtocolArray[i];
			ptl->PushBuffer(buf, readed);
		}
		hif->mCS.Leave();
	}

	return 1;
}

int HardwareInterface::AddRxCallBack(ProtocolType type, HardwareInterfaceCallBack func, void *param)
{
	int i;
	mCS.Enter();
	for(i = 0 ; i < mProtocolArraySize ; i ++)
	{
		if (mProtocolArray[i]->GetProtocolType() == type)
		{
			mCS.Leave();
			return mProtocolArray[i]->AddCallBack(func, param);
		}
	}

	if ((i == mProtocolArraySize) && (i < MAX_PROTOCOL_NUM))
	{
		mProtocolArray[i] = Protocol::CreateProtocol(mId, type);
		mProtocolArraySize++;
		mCS.Leave();
		return mProtocolArray[i]->AddCallBack(func, param);
	}

	mCS.Leave();
	return -1;
}

int HardwareInterface::RemoveRxCallBack(ProtocolType type,  int id)
{
	mCS.Enter();
	for (int i = 0; i < mProtocolArraySize; i++)
	{
		if (mProtocolArray[i]->GetProtocolType() == type)
		{
			mCS.Leave();
			return mProtocolArray[i]->RemoveCallBack(id);
		}
	}
	mCS.Leave();
	return -1;
}

int HardwareInterface::WriteData(ProtocolType type, uint8_t * points, int32_t len)
{
	mTotalTxBytes += len;

	mCS.Enter();
	for(int i = 0 ; i < mProtocolArraySize ; i ++)
	{
		if (mProtocolArray[i]->GetProtocolType() == type)
		{
			int32_t pack_len = 0;
			const uint8_t * pack_buf = mProtocolArray[i]->PackageBuffer(points, len, &pack_len);
			if (pack_buf)
			{
				int32_t v = WriteBufferToInterface(pack_buf, pack_len);
				mCS.Leave();
				return v;
			}
		}
	}
	mCS.Leave();

	return -1;
}

