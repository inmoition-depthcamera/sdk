#ifndef __DEPTH_DENOISE_H__
#define __DEPTH_DENOISE_H__

#include<iostream>
#include"Matrix.h"

class DepthDenoise
{
public:
	DepthDenoise();
	~DepthDenoise();
	void InitDepthDenoise(int w, int h);
	void Depth_Denoise(int w, int h, unsigned short *phase, unsigned short *amplitude, unsigned char *flags, unsigned short* DstFrame, int Amp_Thr = 64);
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