
#ifndef __DEPTH_CAMERA_UVC_PORT_H__
#define __DEPTH_CAMERA_UVC_PORT_H__

#include "inttypes.h"
#include <vector>
#include <string>

using namespace std;

#ifdef _MSC_VER
typedef void(__stdcall *OnDepthFrameCallBack)(unsigned short * phase, unsigned short * amplitude, unsigned char * ambient, unsigned char * flags, void *param);
#else
typedef void(*OnDepthFrameCallBack)(unsigned short * phase, unsigned short * amplitude, unsigned char * ambient, unsigned char * flags, void *param);
#endif

class DepthCameraUvcPort
{
public:
    DepthCameraUvcPort();
    ~DepthCameraUvcPort();

    static int32_t GetDepthCameraList(vector<string> *camera_list);
    
    bool Open(string camera_name);
    bool Close();
    bool IsUvcOpened();

    void SetDepthFrameCallback(OnDepthFrameCallBack, void *param);
    void RemoveDepthFrameCallback();

    bool HasNewFrame();
    bool GetDepthFrame(unsigned short * phase, unsigned short * amplitude, unsigned char * ambient, unsigned char * flags);

    static int32_t DepthToPointCloud(unsigned short *phase, float *point_clould);
    static int32_t DepthToPointCloud(unsigned short *phase, float *point_clould, unsigned short *amplitude = NULL, unsigned short phaseMax = 3072, unsigned short amplitudeMin = 64);
    static int32_t DepthToPointCloud(unsigned short *phase, float *point_clould, float *point_clould_color,  unsigned short *amplitude = NULL, unsigned short phaseMax = 3072, unsigned short amplitudeMin = 64);

    static void PhaseDenoise(unsigned short *phase, unsigned short *amplitude, unsigned char *flags, unsigned short* new_phase, int amp_thr);

private:
    UvcInterface mUvcIf;
};

#endif