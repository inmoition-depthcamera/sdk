
#ifndef __STD_EVENT_H__
#define __STD_EVENT_H__

#include <condition_variable>
#include <mutex>
#include <inttypes.h>
#include <atomic>

class std_event
{
public:
    std_event();
    ~std_event();

    void set();
    void reset();

	void wait();
    int32_t wait(int32_t timeout);

private:
    std::condition_variable mCv;
    std::mutex mMtx;
	std::atomic<bool> mFlag;

	bool newf;
};

#endif