
#ifndef __DEPTH_CAMERA_FILTER_H__
#define __DEPTH_CAMERA_FILTER_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>

#include "matrix.h"


using namespace std;


class DepthCameraFilter
{
public:
	DepthCameraFilter();
    ~DepthCameraFilter();

	void InitDepthDenoise(int w, int h);
	void Depth_Denoise(int w, int h, unsigned short *phase, unsigned short *amplitude, unsigned char *flags, unsigned short* DstFrame, int Amp_Thr);

private:
	static const int dx1[8];
	static const int dy1[8];
	static const int dx2[16];
	static const int dy2[16];
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
};

#endif