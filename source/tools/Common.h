#ifndef __COMMON_H__
#define __COMMON_H__

#include "dshow.h"

#define COLOR_SIZE 256

class Common 
{
public:
	Common();
	~Common() {};
	//static int  LoadDepthCameraParam(char * file_path);
	static void WriteToLogFile(char * param, ...);

	static void FreeMediaType(AM_MEDIA_TYPE& mt);
	static void DeleteMediaType(AM_MEDIA_TYPE *pmt);
	static void SplitRawFrame(int w, int h, unsigned char * source_frame, float *  angle, unsigned short * phase_buffer, unsigned short * amplitude_buffer, unsigned char * ambient_buffer, unsigned char * flags_buffer);
	static void Bit16ToRGB24(unsigned short *source, unsigned short source_maxValue, unsigned short source_minValue, int source_size, unsigned char *rgb_buf, int rgb_size, char color_flag);
	static void Bit8ToRGB24(unsigned char *source, unsigned char source_maxValue, unsigned char source_minValue, int source_size, unsigned char *rgb_buf, int rgb_size, char color_flag);
	static void InitColorMap(void);
	static float * CreateD2PTable(int w, int h, float w_f, float h_f, float p_x, float p_y);
	static void DepthToPointCloud(unsigned short *phase, float *pc, int w, int h, float w_f, float h_f, float p_x, float p_y);
	static int  DepthToPointCloud(unsigned short *phase, unsigned short *amplitude, unsigned short phaseMax, unsigned short amplitudeMin, float *pc, float *pcc, int w, int h, float w_f, float h_f, float p_x, float p_y);
	static void PhaseToColor(float *rColor, float *gColor, float *bColor, unsigned short phaseValue, unsigned short phaseMax, unsigned short phaseMin);

private:
	static int Jet_vR[COLOR_SIZE];
	static int Jet_vG[COLOR_SIZE];
	static int Jet_vB[COLOR_SIZE];
	static float Jet_R[COLOR_SIZE];
	static float Jet_G[COLOR_SIZE];
	static float Jet_B[COLOR_SIZE];
};
#endif