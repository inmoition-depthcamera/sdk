#include "CriticalSection.h"



CriticalSection::CriticalSection()
{
#ifdef _MSC_VER
	InitializeCriticalSection(&mCS);
#else
	mCS = PTHREAD_MUTEX_INITIALIZER;
#endif
}


CriticalSection::~CriticalSection()
{
#ifdef _MSC_VER
	DeleteCriticalSection(&mCS);
#else
	//
#endif
}

void CriticalSection::Enter()
{
#ifdef _MSC_VER
	EnterCriticalSection(&mCS);
#else
	pthread_mutex_lock(&mCS);
#endif
}

void CriticalSection::Leave()
{
#ifdef _MSC_VER
	LeaveCriticalSection(&mCS);
#else
	pthread_mutex_unlock(&mCS);
#endif
}
