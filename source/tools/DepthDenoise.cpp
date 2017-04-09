#include"DepthDenoise.h"

const int DepthDenoise::dx1[8] = { -1,-1,-1,0,0,1,1,1 };
const int DepthDenoise::dy1[8] = { -1,0,1,-1,1,-1,0,1 };
const int DepthDenoise::dx2[16] = { -2,-2,-2,-2,-2,-1,-1,0,0,1,1,2,2,2,2,2 };
const int DepthDenoise::dy2[16] = { -2,-1,0,1,2,-2,2,-2,2,-2,2,-2,-1,0,1,2 };

float DepthDenoise::Big[25] =
{ 0.15f,0.2f,0.15f,0.2f,0.15f,
0.2f,0.4f,0.4f,0.4f,0.2f,
0.25f,0.4f,0.5f,0.4f,0.25f,
0.2f,0.4f,0.4f,0.4f,0.2f,
0.15f,0.2f,0.15f,0.2f,0.15f };

float DepthDenoise::Sma[25] =
{ 0.05f,0.15f,0.15f,0.15f,0.05f,
0.1f,0.2f,0.25f,0.2f,0.1f,
0.15f,0.25f,0.7f,0.25f,0.15f,
0.1f,0.2f,0.25f,0.2f,0.1f,
0.05f,0.15f,0.15f,0.15f,0.05f };

float DepthDenoise::Dis2D[25] =
{ 1.0f / 2.828427125f,1.0f / 2.236067977f,1.0f / 2.0f,1.0f / 2.236067977f,1.0f / 2.828427125f,
1.0f / 2.236067977f,1.0f / 1.414213562f,1.0f / 1.0f,1.0f / 1.414213562f,1.0f / 2.236067977f,
1.0f / 2.0f,1.0f / 1.0f, 2.5f,1.0f / 1.0f,1.0f / 2.0f,
1.0f / 2.236067977f,1.0f / 1.414213562f,1.0f / 1.0f,1.0f / 1.414213562f,1.0f / 2.236067977f,
1.0f / 2.828427125f,1.0f / 2.236067977f,1.0f / 2.0f,1.0f / 2.236067977f,1.0f / 2.828427125f};

float DepthDenoise::One[25] =
{ 1.0f,1.0f,1.0f,1.0f,1.0f,
1.0f,1.0f,1.0f,1.0f,1.0f,
1.0f,1.0f,0.0f,1.0f,1.0f,
1.0f,1.0f,1.0f,1.0f,1.0f,
1.0f,1.0f,1.0f,1.0f,1.0f };

Matrix DepthDenoise::OneMat(5, 5, One);
Matrix DepthDenoise::BigMat(5, 5, Big);
Matrix DepthDenoise::SmaMat(5, 5, Sma);
Matrix DepthDenoise::Dis2DMat(5, 5, Dis2D);
Matrix DepthDenoise::Relation(5, 5);
Matrix DepthDenoise::DepthROI(5, 5);
Matrix DepthDenoise::AmpMatROI(5, 5);
Matrix DepthDenoise::AmpRatio;

DepthDenoise::DepthDenoise()
{
}

DepthDenoise::~DepthDenoise()
{
}

void DepthDenoise::InitDepthDenoise(int frameW, int frameH)
{
	AmpMat.SetMatrix( frameH,frameW, NULL);
}

void DepthDenoise::Depth_Denoise(int frameW, int frameH,unsigned short *phase, unsigned short *amplitude, unsigned char *flags, unsigned short* DstFrame, int Amp_Thr)
{
	memset(DstFrame, 0, (sizeof(unsigned short) * frameW * frameH));
	Matrix depthMat(frameH, frameW, phase);

	for (int i = 0; i < frameH; i++)
	{
		for (int j = 0; j < frameW; j++)
		{
			int Index = i*frameW + j;
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

	for (int i = 2; i < frameH - 2; i++)
	{
		
		for (int j = 2; j < frameW - 2; j++)
		{
			int Index = i*frameW + j;

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
