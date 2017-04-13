
#ifndef __DEPTH_CAMERA_UVC_PORT_H__
#define __DEPTH_CAMERA_UVC_PORT_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>


using namespace std;

#if defined WIN32
	#if defined UVC_DRIVER_LIBUVC
		#include "os/uvc_interface_libuvc.h"
		#define UVC_INTERFACE_DRIVER UvcInterfaceLibUvc
	#else // default uvc driver in windows
		#include "os/uvc_interface_direct_show.h"
		#define UVC_INTERFACE_DRIVER UvcInterfaceDirectShow
	#endif
#else
	#ifdef UVC_DRIVER_LIBUVC
		#include "os/uvc_interface_libuvc.h"
		#define UVC_INTERFACE_DRIVER UvcInterfaceLibUvc
	#else // default uvc driver in linux
		#include "os/uvc_interface_v4l.h"
		#define UVC_INTERFACE_DRIVER UvcInterfaceV4L
	#endif
#endif 

class DepthCameraUvcPort : public UVC_INTERFACE_DRIVER
{
public:
    DepthCameraUvcPort();
    ~DepthCameraUvcPort();

	virtual bool Open(std::string &camera_name) override;
	virtual bool Close() override;

    int32_t GetDepthCameraList(vector<string> &camera_list);
    
    void SetDepthFrameCallback(std::function<void(const uint16_t * phase, const uint16_t * amplitude, 
		const uint8_t * ambient, const uint8_t * flags, void *param)>, void *param);

    bool GetDepthFrame(uint16_t * phase, uint16_t * amplitude, uint8_t * ambient, uint8_t * flags);

    int32_t DepthToPointCloud(const uint16_t *phase, float *point_clould);
    int32_t DepthToPointCloud(const uint16_t *phase, float *point_clould,
		const uint16_t *amplitude = NULL, uint16_t phaseMax = 3072, uint16_t amplitudeMin = 64);
	
	virtual int32_t GetWidth() { return mWidth; };
	virtual int32_t GetHeight() { return mHeight; };

private:
	std::function<void(const uint16_t *, const uint16_t *, const uint8_t *, const uint8_t *, void *)> mOnDepthFrameCallBack;
	void *mOnDepthFrameCallBackParam;

	static void OnUvcFrame(double sample_time, uint8_t *frame_buf, int32_t frame_buf_len, void *param);

	void SplitUvcFrameToDepthFrame(uint8_t *frame_buf, int32_t frame_buf_len);

	uint16_t* mDepthBuffer;
	uint16_t* mAmplitudeBuffer;
	uint8_t*  mAmbintBuffer;
	uint8_t*  mFlagBuffer;

	int32_t mWidth;
	int32_t mHeight;

	float mWFocal, mHFocal;

	std::mutex mMutex;

	float *mD2PTable;

	bool mAlreadyPrepared;
};

#endif