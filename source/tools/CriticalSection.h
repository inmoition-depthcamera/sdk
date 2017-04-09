#ifndef  __CRITICAL_SECTION_H
#define  __CRITICAL_SECTION_H

#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif

class CriticalSection
{
public:
	CriticalSection();
	~CriticalSection();

	void Enter();
	void Leave();

private:
#ifdef _MSC_VER
	CRITICAL_SECTION mCS;
#else
	pthread_mutex_t mCS;
#endif
};

#endif