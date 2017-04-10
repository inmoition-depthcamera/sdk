#include "depth_camera_filter.h"


const int DepthCameraFilter::dx1[8] = { -1,-1,-1,0,0,1,1,1 };
const int DepthCameraFilter::dy1[8] = { -1,0,1,-1,1,-1,0,1 };
const int DepthCameraFilter::dx2[16] = { -2,-2,-2,-2,-2,-1,-1,0,0,1,1,2,2,2,2,2 };
const int DepthCameraFilter::dy2[16] = { -2,-1,0,1,2,-2,2,-2,2,-2,2,-2,-1,0,1,2 };

float DepthCameraFilter::Big[25] =
{ 0.15f,0.2f,0.15f,0.2f,0.15f,
0.2f,0.4f,0.4f,0.4f,0.2f,
0.25f,0.4f,0.5f,0.4f,0.25f,
0.2f,0.4f,0.4f,0.4f,0.2f,
0.15f,0.2f,0.15f,0.2f,0.15f };

float DepthCameraFilter::Sma[25] =
{ 0.05f,0.15f,0.15f,0.15f,0.05f,
0.1f,0.2f,0.25f,0.2f,0.1f,
0.15f,0.25f,0.7f,0.25f,0.15f,
0.1f,0.2f,0.25f,0.2f,0.1f,
0.05f,0.15f,0.15f,0.15f,0.05f };

float DepthCameraFilter::Dis2D[25] =
{ 1.0f / 2.828427125f,1.0f / 2.236067977f,1.0f / 2.0f,1.0f / 2.236067977f,1.0f / 2.828427125f,
1.0f / 2.236067977f,1.0f / 1.414213562f,1.0f / 1.0f,1.0f / 1.414213562f,1.0f / 2.236067977f,
1.0f / 2.0f,1.0f / 1.0f, 2.5f,1.0f / 1.0f,1.0f / 2.0f,
1.0f / 2.236067977f,1.0f / 1.414213562f,1.0f / 1.0f,1.0f / 1.414213562f,1.0f / 2.236067977f,
1.0f / 2.828427125f,1.0f / 2.236067977f,1.0f / 2.0f,1.0f / 2.236067977f,1.0f / 2.828427125f };

float DepthCameraFilter::One[25] =
{ 1.0f,1.0f,1.0f,1.0f,1.0f,
1.0f,1.0f,1.0f,1.0f,1.0f,
1.0f,1.0f,0.0f,1.0f,1.0f,
1.0f,1.0f,1.0f,1.0f,1.0f,
1.0f,1.0f,1.0f,1.0f,1.0f };

Matrix DepthCameraFilter::OneMat(5, 5, One);
Matrix DepthCameraFilter::BigMat(5, 5, Big);
Matrix DepthCameraFilter::SmaMat(5, 5, Sma);
Matrix DepthCameraFilter::Dis2DMat(5, 5, Dis2D);
Matrix DepthCameraFilter::Relation(5, 5);
Matrix DepthCameraFilter::DepthROI(5, 5);
Matrix DepthCameraFilter::AmpMatROI(5, 5);
Matrix DepthCameraFilter::AmpRatio;

DepthCameraFilter::DepthCameraFilter()
{
}

DepthCameraFilter::~DepthCameraFilter()
{
}

void DepthCameraFilter::InitDepthDenoise(int w, int h)
{
	AmpMat.SetMatrix(w, h, NULL);
}

void DepthCameraFilter::Depth_Denoise(int w, int h, unsigned short * phase, unsigned short * amplitude, unsigned char * flags, unsigned short * DstFrame, int Amp_Thr)
{
	Matrix depthMat(w, h, phase);

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			int Index = i*w + j;
			if (flags[Index] != 0)
			{
				AmpMat.data[i][j] = 0;
			}
			else if (amplitude[Index] > 300)
			{
				AmpMat.data[i][j] = 10;
			}
			else if (amplitude[Index]  > 200)
			{
				AmpMat.data[i][j] = 6;
			}
			else if (amplitude[Index]  > 100)
			{
				AmpMat.data[i][j] = 3;
			}
			else if (amplitude[Index]  > Amp_Thr)
			{
				AmpMat.data[i][j] = 1;
			}
			else
			{
				AmpMat.data[i][j] = 0;
			}
		}
	}

	for (int i = 2; i < h - 2; i++)
	{

		for (int j = 2; j < w - 2; j++)
		{
			int Index = i*w + j;

			if (AmpMat.data[i][j] == 0)
			{
				DstFrame[Index] = 0;
				continue;
			}


			if (AmpMat.data[i][j] > 5)
			{
				AmpRatio = SmaMat;
			}
			else
			{
				AmpRatio = BigMat;
			}

			ROI55(&AmpMat, &AmpMatROI, j - 2, i - 2, 5, 5);

			ROI55(&depthMat, &DepthROI, j - 2, i - 2, 5, 5);

			float Center = DepthROI.data[2][2];
			int edgeNum = 0;
			for (int k = 0; k < 8; k++)
			{
				{
					if (Center - DepthROI.data[2 + dx1[k]][2 + dy1[k]]>250)
					{
						edgeNum++;
						Relation.data[2 + dx1[k]][2 + dy1[k]] = 0;
					}
					else if (Center - DepthROI.data[2 + dx1[k]][2 + dy1[k]] < -250)
					{
						edgeNum++;
						Relation.data[2 + dx1[k]][2 + dy1[k]] = 0;
					}
					else
					{
						Relation.data[2 + dx1[k]][2 + dy1[k]] = 1;
					}
				}
			}
			if ((edgeNum > 1))
			{
				DstFrame[Index] = 0;
				continue;
			}
			DstFrame[Index] = Convolution(&AmpMatROI, &Relation, &Dis2DMat, &DepthROI);
		}
	}
}
