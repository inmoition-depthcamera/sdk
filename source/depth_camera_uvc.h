
#ifndef __DEPTH_CAMERA_UVC_PORT_H__
#define __DEPTH_CAMERA_UVC_PORT_H__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>


using namespace std;

class DepthFrame{
public:
    int32_t w;
    int32_t h;
    uint16_t *phase;
    uint16_t *amplitude;
    uint8_t *ambient;
    uint8_t *flags;

	DepthFrame(int32_t _w, int32_t _h);
	~DepthFrame();
	bool CopyTo(DepthFrame *df);
	bool CopyFrom(DepthFrame *df);
};

#if defined _MSC_VER 
    #include "os/uvc_interface_direct_show.h"
    #define UVC_INTERFACE_DRIVER UvcInterfaceDirectShow
#else
    #include "os/uvc_interface_v4l.h"
    #define UVC_INTERFACE_DRIVER UvcInterfaceV4L
#endif 

class DepthCameraUvcPort : public UVC_INTERFACE_DRIVER
{
public:
    DepthCameraUvcPort();
    ~DepthCameraUvcPort();

	virtual bool Open(std::string &camera_name) override;
	virtual bool Close() override;

    bool GetDepthCameraList(vector<string> &camera_list);
    
    void SetDepthFrameCallback(std::function<void(const DepthFrame *depth_frame, void *param)>, void *param);

    bool GetDepthFrame(DepthFrame *df);

    int32_t DepthToPointCloud(const DepthFrame *df, float *point_clould);
    int32_t DepthToPointCloud(const DepthFrame *df, float *point_clould, uint16_t phaseMax = 3072, uint16_t amplitudeMin = 64);
	
	virtual int32_t GetWidth() { return mDepthFrame ? mDepthFrame->w : -1; }
	virtual int32_t GetHeight() { return mDepthFrame ? mDepthFrame->h : -1; }

	void SetHdrMode(bool enable);

private:
	std::function<void(const DepthFrame *, void *)> mOnDepthFrameCallBack;
	void *mOnDepthFrameCallBackParam;

	static void OnUvcFrame(double sample_time, uint8_t *frame_buf, int32_t frame_buf_len, void *param);
	void SplitUvcFrameToDepthFrame(uint8_t *frame_buf, int32_t frame_buf_len);

    DepthFrame *mDepthFrame, *mLastDepthFrame, *mHdrDepthFrame;
	float mWFocal, mHFocal;
	std::mutex mMutex;
	float *mD2PTable;
	std::atomic_bool mAlreadyPrepared;
	std::atomic_bool mHdrMode;

};

#endif