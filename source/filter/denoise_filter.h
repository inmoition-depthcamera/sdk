
#ifndef __DEPTH_CAMERA_FILTER_H__
#define __DEPTH_CAMERA_FILTER_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "matrix.h"


using namespace std;


class DenoiseFilter
{
public:
	DenoiseFilter();
	~DenoiseFilter();

	void Init(int w, int h);
	void Denoise(int w, int h, unsigned short *phase, unsigned short *amplitude, unsigned char *flags,
		unsigned short *new_phase_frame, int amplitude_th, int filter_level = 1);

private:
	static const int dx1[8];
	static const int dy1[8];
	static const int dx2[24];
	static const int dy2[24];
	static float Big[25];
	static float Sma[25];
	static float Dis2D[25];
	static float One[25];
	static Matrix OneMat;
	static Matrix BigMat;
	static Matrix SmaMat;
	static Matrix Dis2DMat;
	static Matrix Relation;
	static Matrix DepthROI;
	static Matrix AmpMatROI;
	static Matrix AmpRatio;
	Matrix AmpMat;
	bool mInited;
};

#endif