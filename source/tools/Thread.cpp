#include "Thread.h"

Thread::Thread()
{
	mHasExited = 0;
	mRunningFlag = 0;
	mCallBack = NULL;
	mThreadHandle = INVALID_HANDLE_VALUE;
}

Thread::~Thread()
{
	Exit();
}

int Thread::Create(ThreadCallback callback, void *param)
{
	mCallBack = callback;
	mUserParam = param;
	mThreadHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)WorkThread, this, CREATE_SUSPENDED, NULL);
	return 0;
}

int Thread::Exit(int timeout /* = -1 */)
{
	if (mThreadHandle != INVALID_HANDLE_VALUE)
	{
		mRunningFlag = 0;
		WaitForSingleObject(mThreadHandle, timeout);
		CloseHandle(mThreadHandle);
		mThreadHandle = INVALID_HANDLE_VALUE;
		return 0;
	}
	return -1;
}

int Thread::Start()
{
	if (mThreadHandle == INVALID_HANDLE_VALUE)
	{
		if (mCallBack != NULL)
			Create(mCallBack, mUserParam);
		else
			return -1;
	}

	mRunningFlag = 1;
	Resume();

	return 0;
}

void Thread::Suspend()
{
	if (mThreadHandle != INVALID_HANDLE_VALUE)
		SuspendThread(mThreadHandle);
}

void Thread::Resume()
{
	if (mThreadHandle != INVALID_HANDLE_VALUE)
		ResumeThread(mThreadHandle);
}

void Thread::WorkThreadLoop()
{
	if (mCallBack)
	{
		while (mRunningFlag)
			if (!mCallBack(mUserParam))
				mRunningFlag = 0;
	}

	CloseHandle(mThreadHandle);
	mThreadHandle = INVALID_HANDLE_VALUE;
}
