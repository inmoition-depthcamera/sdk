#include "Common.h"
#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <time.h>

int Common::Jet_vR[COLOR_SIZE];
int Common::Jet_vG[COLOR_SIZE];
int Common::Jet_vB[COLOR_SIZE];
float Common::Jet_R[COLOR_SIZE];
float Common::Jet_G[COLOR_SIZE];
float Common::Jet_B[COLOR_SIZE];

Common::Common()
{
	InitColorMap();
}

void Common::WriteToLogFile(char * fmt, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	FILE *file = NULL;
	if (fopen_s(&file, "log.txt", "r+") == 0)
	{
		fseek(file, 0L, SEEK_END);
		struct tm tmNow;
		time_t long_time;
		time(&long_time);              /* Get time as long integer. */
		localtime_s(&tmNow, &long_time); /* Convert to local time. */
		fprintf(file, "[%d-%d-%d %d:%d:%d] ", 1900 + tmNow.tm_year, tmNow.tm_mon + 1,
			tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);
		char buf[1024];
		int len = vsprintf_s(buf, fmt, arg_ptr);
		fprintf(file, buf);
		fclose(file);
	}
	
	va_end(arg_ptr);
}

void Common::FreeMediaType(AM_MEDIA_TYPE& mt) {
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// Unecessary because pUnk should not be used, but safest.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

void Common::DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt != NULL)
	{
		FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}

void Common::SplitRawFrame(int fw, int fh, unsigned char * source_frame, float * angle, unsigned short * phase_buffer, unsigned short * amplitude_buffer, unsigned char * ambient_buffer, unsigned char * flags_buffer)
{
	int w = fw;
	int h = fh;
	unsigned short * framePtr = (unsigned short *)source_frame;
	int angle_t = 0;
	if (h % 10 > 0)  //如果成了证明是3D雷达（3D雷达为了传输每一帧的角度而将每一帧多传了2行来传输角度）
	{
		int a = h;
		h = w;
		w = a;
		if (angle)
		{
			angle_t = (int)*framePtr;
			*angle = (float)angle_t / 10.0f;
		}
		framePtr += (((w % 10) * h) * 2);
		for (int i = 0; i < (w - w % 10); i++)
		{
			for (int j = 0; j < h; j++)
			{
				amplitude_buffer[j * (w - w % 10) + i] = (*framePtr & 0x0FFF);
				ambient_buffer[j * (w - w % 10) + i] = ((*framePtr & 0xF000) >> 12);
				framePtr++;
			}
			for (int j = 0; j < h; j++)
			{
				phase_buffer[j * (w - w % 10) + i] = (*framePtr & 0x0FFF);
				flags_buffer[j * (w - w % 10) + i] = ((*framePtr & 0xC000) >> 12);
				framePtr++;
			}
		}
	}
	else
	{
		*angle = 0;
		for (int i = 0; i < h; i++)
		{
			int j;
			for (j = 0; j < w; j++)
			{
				*amplitude_buffer++ = (*framePtr & 0x0FFF);
				*ambient_buffer++ = ((*framePtr & 0xF000) >> 12);
				framePtr++;
			}

			for (; j < w * 2; j++)
			{
				*phase_buffer++ = (*framePtr & 0x0FFF);
				if (h == 60)
					*flags_buffer++ = ((*framePtr & 0xC000) >> 12);
				else
					*flags_buffer++ = ((*framePtr & 0xF000) >> 12);
				framePtr++;
			}
		}
	}
}

void Common::InitColorMap(void)
{
	for (int i = 0; i < 64; i++)
	{
		//r = 255, g++, b = 0
		Jet_vR[i] = 255;
		Jet_vG[i] = i * 4;
		Jet_vB[i] = 0;

		Jet_R[i] = 1;
		Jet_G[i] = i * 4 / 255.0f;
		Jet_B[i] = 0;
	}
	for (int i = 64; i < 128; i++)
	{
		//r--, g = 255, b = 0
		Jet_vR[i] = 255 - (i - 64) * 4;
		Jet_vG[i] = 255;
		Jet_vB[i] = 0;

		Jet_R[i] = ((255 - (i - 64) * 4) / 255.0f);
		Jet_G[i] = 1;
		Jet_B[i] = 0;
	}
	for (int i = 128; i < 192; i++)
	{
		//r = 0, g = 255, b ++
		Jet_vR[i] = 0;
		Jet_vG[i] = 255;
		Jet_vB[i] = (i - 128) * 4;

		Jet_R[i] = 0;
		Jet_G[i] = 1;
		Jet_B[i] = ((i - 128) * 4 / 255.0f);
	}
	for (int i = 192; i < 256; i++)
	{
		//r = 0, g --, b = 255
		Jet_vR[i] = 0;
		Jet_vG[i] = 255 - (i - 192) * 4;
		Jet_vB[i] = 255;

		Jet_R[i] = 0;
		Jet_G[i] = (255 - (i - 192) * 4) / 255.0f;
		Jet_B[i] = 1;
	}
}


void Common::Bit16ToRGB24(unsigned short *source, unsigned short source_maxValue, unsigned short source_minValue, int source_size, unsigned char *rgb_buf, int rgb_size, char color_flag)
{
	if (source_size * 3 > rgb_size)
		return;

	double k = ((0.0 - 255.0) / (double)(source_minValue - source_maxValue));
	double b = 255.0 - source_maxValue* k;
	int index = 0;
	int value = 0;
	if (color_flag > 0)
	{
		for (int i = 0; i < source_size; i++)
		{
			value = *source++;
			index = (int)(value * k + b);
			if (index >= 256)
				index = 256 - 1;
			if (index < 0)
				index = 0;
			*rgb_buf++ = (unsigned char)Jet_vB[index];
			*rgb_buf++ = (unsigned char)Jet_vG[index];
			*rgb_buf++ = (unsigned char)Jet_vR[index];
		}
	}
	else
	{
		for (int i = 0; i < source_size; i++)
		{
			value = *source++;
			index = (int)(value * k + b);
			if (index >= 256)
				index = 256 - 1;
			if (index < 0)
				index = 0;
			*rgb_buf++ = index;
			*rgb_buf++ = index;
			*rgb_buf++ = index;
		}
	}
}


void Common::Bit8ToRGB24(unsigned char *source, unsigned char source_maxValue, unsigned char source_minValue, int source_size, unsigned char *rgb_buf, int rgb_size, char color_flag)
{
	if (source_size * 3 > rgb_size)
		return;

	double k = ((0 - 255) / (source_minValue - source_maxValue));
	double b = 255 - source_maxValue * k;
	int index = 0;
	int value = 0;
	if (color_flag > 0)
	{
		for (int i = 0; i < source_size; i++)
		{
			value = *source++;
			index = (int)(value * k + b);
			if (index >= 256)
				index = 256 - 1;
			if (index < 0)
				index = 0;
			*rgb_buf++ = (unsigned char)Jet_vG[index];
			*rgb_buf++ = (unsigned char)Jet_vR[index];
			*rgb_buf++ = (unsigned char)Jet_vB[index];
		}
	}
	else
	{
		for (int i = 0; i < source_size; i++)
		{
			value = *source++;
			index = (int)(value * k + b);
			if (index >= 256)
				index = 256 - 1;
			if (index < 0)
				index = 0;
			*rgb_buf++ = index;
			*rgb_buf++ = index;
			*rgb_buf++ = index;
		}
	}
}

void Common::PhaseToColor( float *rColor, float *gColor, float *bColor, unsigned short phaseValue, unsigned short phaseMax, unsigned short phaseMin)
{
	float k = 255.0f / (float)(phaseMax - phaseMin);
	float b = 255.0f - phaseMax * k;
	int index = 0;
	int Cvalue = phaseValue;
	index = (int)(Cvalue * k + b);
	if (index >= 256)
		index = 256 - 1;
	if (index < 0)
		index = 0;
	*rColor = Jet_B[index];
	*gColor = Jet_G[index];
	*bColor = Jet_R[index];
}

float * Common::CreateD2PTable(int w, int h, float w_f, float h_f, float p_x, float p_y)
{
	float *table;
	float k1 = 1 / w_f;
	float k2 = 1 / h_f;
	float *p = table = new float[w * h];
	int i = 0;
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			*p++ = (float)(1.0 / sqrt(1 + k1 * k1 * (p_x - x) * (p_x - x) + k2 * k2 * (p_y - y) * (p_y - y)));
		}
	}
	return table;

}

//DepSpaceToCamSpace(phase, cp, 320, 240, 247.0, 247.0, 7.5 / 3072.0)
void Common::DepthToPointCloud(unsigned short *phase, float *pc, int w, int h, float w_f, float h_f, float p_x, float p_y)
{
	unsigned short * p_buf = phase;
	float *c_buf = pc;

	for (int y = 0; y < h; y++)
	{
		for (int x= 0; x < w; x++)
		{
			float z =  *p_buf;//z
			*c_buf++ = -(float)(x - p_x) * z / w_f;//x
			*c_buf++ = -(float)(y - p_y) * z / h_f;//y
			*c_buf++ = z;
			p_buf++;
		}
	}
}

int Common::DepthToPointCloud(unsigned short *phase, unsigned short *amplitude, unsigned short phaseMax, unsigned short amplitudeMin, float *pc, float *pcc, int w, int h, float w_f, float h_f, float p_x, float p_y)
{
	unsigned short * p_buf = phase;
	unsigned short * a_buf = amplitude;
	float *c_buf = pc;
	float *cc_buf = pcc;
	int totalPoint = 0;
	float rColor, gColor, bColor;
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (*p_buf > 0 && *p_buf <= phaseMax && *a_buf >= amplitudeMin)
			{
				float z = *p_buf;//z
				*c_buf++ = -(float)(x - p_x) * z / w_f;//x
				*c_buf++ = -(float)(y - p_y) * z / h_f;//y
				*c_buf++ = z;
				PhaseToColor(&rColor, &gColor, &bColor, *p_buf, phaseMax, 0);
				*cc_buf++ = bColor;
				*cc_buf++ = gColor;
				*cc_buf++ = rColor;
				totalPoint++;
			}
			a_buf++;
			p_buf++;
		}
	}
	return totalPoint;
}


//int Common::LoadDepthCameraParam(char * file_path)
//{
//	FILE *file = NULL;
//	if (fopen_s(&file, file_path, "r") == 0)
//	{
//		int rows, cols;
//		fscanf_s(file, "%d", &rows);
//		fscanf_s(file, "%d", &cols);
//		if (DepthMapBuf)
//			delete[] DepthMapBuf;
//		DepthMapBuf = new float[rows * cols];
//		double ele;
//		for (int i = 0; i < rows; i++)
//		{
//			for (int j = 0; j < cols; j++)
//			{
//				fscanf_s(file, "%lf", &ele);
//				DepthMapBuf[i * 320 + j] = (float)ele;
//			}
//		}
//		fclose(file);
//		return 1;
//	}
//	return 0;
//}


