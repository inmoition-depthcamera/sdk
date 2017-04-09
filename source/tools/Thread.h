#ifndef __THREAD_H__
#define __THREAD_H__

#ifdef _MSC_VER
#include "windows.h"
#else
#endif

typedef int (*ThreadCallback)(void *param);

class Thread
{
public:
	Thread();
	~Thread();

	int Create(ThreadCallback callback, void *param);

	int Exit(int timeout = -1);
	int Start();

	void Suspend();
	void Resume();

	int HasExited() { return mHasExited; }

private:
	int mHasExited;
	int mRunningFlag;

	void *mUserParam;
	ThreadCallback mCallBack;

	void WorkThreadLoop();
	
#ifdef _MSC_VER
	HANDLE mThreadHandle;	

	static DWORD WorkThread(LPVOID lpParameter)
	{
		Thread *work_thread = (Thread *)lpParameter;
		work_thread->WorkThreadLoop();
		return 0;
	}

#else


#endif
};

#endif