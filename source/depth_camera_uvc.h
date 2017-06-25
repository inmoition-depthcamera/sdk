
#ifndef __DEPTH_CAMERA_UVC_PORT_H__
#define __DEPTH_CAMERA_UVC_PORT_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include "depth_frame.h"

using namespace std;

#if defined _MSC_VER 
    #include "os/uvc_interface_direct_show.h"
    #define DepthCameraUvcPort UvcInterfaceDirectShow
#else
    #include "os/uvc_interface_v4l.h"
    #define DepthCameraUvcPort UvcInterfaceV4L
#endif 



#endif

