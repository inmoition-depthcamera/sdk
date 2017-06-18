
#ifndef __DEPTH_VIDEO_INTERFACE__
#define __DEPTH_VIDEO_INTERFACE__

#include <inttypes.h>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include "depth_frame.h"
#include "video_interface.h"

/// @brief INMOTION depth camera streaming interface.
///
/// This class is use to connect depth camera usb port,
/// and give user many mothed to get the camera's
/// depth related data
class DepthVideoInterface : public VideoInterface
{
public:
	/// @brief The constructor
	DepthVideoInterface();

	/// @brief The destructor
	~DepthVideoInterface();

	/// @brief Open video port with given name
	/// 
	/// The camera name can be found by GetDepthCameraList function.
	///
	/// @param camera_name The camera's name to be opened
	/// @return Return ture if successed
	virtual bool Open(std::string &camera_name) override; // should been called by child class;

	/// @brief Close the video port
	/// @return Return ture if successed
	virtual bool Close() override; // should been called by child class;

	/// @brief Get the depth camera list in the system
	/// @return Return ture if successed
	virtual bool GetDepthCameraList(vector<string> &camera_list) = 0;

	/// @brief Set the depth frame call back function.
	/// 
	/// The callback function will be called every frame
	///
	void SetDepthFrameCallback(std::function<void(const DepthFrame *depth_frame, void *param)>, void *param);

	/// @brief Get a depth frame from the camrea
	/// @param df The depth camera frame buffer. This frame buffer need to been initialized before call this function
	/// @return Return ture if successed
	bool GetDepthFrame(DepthFrame *df);

	/// @brief Converting depth data into 3D point cloud data.
	/// @param df The depth camera frame buffer.
	/// @param point_cloud The output point cloud buffer. The buffer size should be 3 * width * height
	/// @param scale The scale of point_cloud points. This value can come from DepthCameraCmdPort.GetDepthScale function.
	/// @return Return the points in the points cloud array( = w * h);
	int32_t ToPointsCloud(const DepthFrame *df, float *point_clould, float scale = 1.0f);

	/// @brief Converting depth data into 3D point cloud data with given phase and size
	/// @param phase The phase buffer. The size of phase buffer should be width * height.
	/// @param w The width of phase buffer.
	/// @param h The height of phase buffer.
	/// @param point_cloud The output point cloud buffer. The buffer size should be 3 * width * height  (phase to x y z)
	/// @param scale The scale of point_cloud points. This value can come from DepthCameraCmdPort.GetDepthScale function.
	/// @return Return the points in the points cloud array( = w * h);
	int32_t ToPointsCloud(const uint16_t *phase, int32_t w, int32_t h, float * point_clould, float scale = 1.0f);

	/// @brief Converting depth data into 3D point cloud data with simple filter
	/// @param df The depth camera frame buffer.
	/// @param point_cloud The output point cloud buffer. The buffer size should be 3 * width * height
	/// @param scale The scale of point_cloud points. This value can come from DepthCameraCmdPort.GetDepthScale function.
	/// @param phase_max The max valid phase value. The value more than phase_max will been ignored.
	/// @param amplitude_min The min valid amplitude value. The value less than amplitude_min will been ignored.
	/// @return Return the points in the points cloud array( = w * h);
	int32_t ToFiltedPointsCloud(const DepthFrame *df, float *point_clould, float scale = 1.0f, uint16_t phase_max = 3072, uint16_t amplitude_min = 64);

	/// @brief Converting depth data into 3D point cloud data with simple filter
	/// @param df The depth camera frame buffer.
	/// @param point_cloud The output point cloud buffer. The buffer size should be 3 * width * height
	/// @param scale The scale of point_cloud points. This value can come from DepthCameraCmdPort.GetDepthScale function.
	/// @param phase_max The max valid phase value. The value more than phase_max will been ignored.
	/// @param amplitude_min The min valid amplitude value. The value less than amplitude_min will been ignored.
	/// @return Return the points in the points cloud array( = w * h);
	int32_t ToFiltedPointsCloud(const uint16_t *phase, const DepthFrame *df, float *point_clould, float scale = 1.0f, uint16_t phase_max = 3072, uint16_t amplitude_min = 64);

	/// @brief Get the width of a depth frame
	/// @return Return the width of a depth frame
	virtual int32_t GetWidth() { return mDepthFrame ? mDepthFrame->w : -1; }

	/// @brief Get the height of a depth frame
	/// @return Return the height of a depth frame
	virtual int32_t GetHeight() { return mDepthFrame ? mDepthFrame->h : -1; }

	/// @brief Set the frame processor to hdr mode
	/// @param enable the to ture to enalbe hdr mode and false to disable hdr mode.
	void SetHdrMode(bool enable);

private:
	std::function<void(const DepthFrame *, void *)> mOnDepthFrameCallBack;
	void *mOnDepthFrameCallBackParam;

	static void OnVideoFrame(double sample_time, uint8_t *frame_buf, int32_t frame_buf_len, void *param);
	void SplitVideoFrameToDepthFrame(uint8_t *frame_buf, int32_t frame_buf_len);

	DepthFrame *mDepthFrame, *mLastDepthFrame, *mHdrDepthFrame;
	float mWFocal, mHFocal;
	std::mutex mMutex;
	float *mD2PTable;
	std::atomic_bool mAlreadyPrepared;
	std::atomic_bool mHdrMode;
};

#endif
