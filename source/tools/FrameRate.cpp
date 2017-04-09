#include "FrameRate.h"

FrameRate::FrameRate()
{
	QueryPerformanceFrequency(&CpuFrequence);
	LastTime.QuadPart = 0;
	Fps = 0;
	FrameCount = 0;
}

void FrameRate::Update()
{
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	UINT64 dt = 1000000L * (current_time.QuadPart - LastTime.QuadPart) / CpuFrequence.QuadPart;
	FrameCount++;
	if (dt > 1000000L)
	{
		Fps = (float)(FrameCount * 1000000.0 / (double)dt);
		FrameCount = 0;
		LastTime.QuadPart = current_time.QuadPart;
	}
}

float FrameRate::GetFps()
{
	return Fps;
}

