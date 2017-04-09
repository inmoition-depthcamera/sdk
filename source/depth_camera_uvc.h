
#ifndef __DEPTH_CAMERA_UVC_PORT_H__
#define __DEPTH_CAMERA_UVC_PORT_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>

using namespace std;

#if defined WIN32
	#if defined UVC_DRIVER_LIBUVC
		#include "os/uvc_interface_libuvc.h"
		#define UVC_INTERFACE_DRVIER UvcInterfaceLibUvc
	#else // default uvc driver in windows
		#include "os/uvc_interface_direct_show.h"
		#define UVC_INTERFACE_DRVIER UvcInterfaceDirectShow
	#endif
#else
	#ifdef UVC_DRIVER_LIBUVC
		#include "os/uvc_interface_libuvc.h"
		#define UVC_INTERFACE_DRVIER UvcInterfaceLibUvc
	#else // default uvc driver in linux
		#include "os/uvc_interface_v4l.h"
		#define UVC_INTERFACE_DRVIER UvcInterfaceV4L
	#endif
#endif 

class DepthCameraUvcPort : public UVC_INTERFACE_DRVIER
{
public:
    DepthCameraUvcPort();
    ~DepthCameraUvcPort();

    static int32_t GetDepthCameraList(vector<string> &camera_list);
    
    bool Open(string camera_name);
    bool Close();
    bool IsOpened();

    void SetDepthFrameCallback(std::function<void(uint16_t * phase, uint16_t * amplitude, uint8_t * ambient, uint8_t * flags, void *param)>, void *param);
    void RemoveDepthFrameCallback();

    bool HasNewFrame();
    bool GetDepthFrame(uint16_t * phase, uint16_t * amplitude, uint8_t * ambient, uint8_t * flags);

    static int32_t DepthToPointCloud(uint16_t *phase, float *point_clould);
    static int32_t DepthToPointCloud(uint16_t *phase, float *point_clould, 
            uint16_t *amplitude = NULL, uint16_t phaseMax = 3072, uint16_t amplitudeMin = 64);
    static int32_t DepthToPointCloud(uint16_t *phase, float *point_clould, float *point_clould_color,  
            uint16_t *amplitude = NULL, uint16_t phaseMax = 3072, uint16_t amplitudeMin = 64);

    static void PhaseDenoise(uint16_t *phase, uint16_t *amplitude, uint8_t *flags, uint16_t* new_phase, int amp_thr);

private:

	std::function<void(uint16_t * phase, uint16_t * amplitude, uint8_t * ambient, uint8_t * flags, void *param)> mOnFrame;
	void *mOnFrameParam;
};

#endif