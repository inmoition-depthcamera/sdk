#include "std_event.h"

std_event::std_event()
{
	mFlag = false;
}

std_event::~std_event()
{

}

void std_event::set()
{
	mFlag = true;
	mCv.notify_all();
}

void std_event::reset()
{
	mFlag = false;
}

void std_event::wait()
{
	std::unique_lock <std::mutex> lck(mMtx);
	mCv.wait(lck, [this] {return mFlag.load(); });
}

int32_t std_event::wait(int32_t timeout)
{
	std::unique_lock <std::mutex> lck(mMtx);
	bool ret = mCv.wait_for(lck, std::chrono::milliseconds(timeout), 
		[this] {return mFlag.load(); });
	return ret == (bool)std::cv_status::timeout ? -1 : 0;
}