#ifndef __FRAME_RATE_H__
#define __FRAME_RATE_H__

#include <Windows.h>

class FrameRate
{
public:
	FrameRate();
	~FrameRate() {}

	void Update();
	float GetFps();

private:
	LARGE_INTEGER CpuFrequence;
	LARGE_INTEGER LastTime;

	int FrameCount;
	float Fps;
};

#endif