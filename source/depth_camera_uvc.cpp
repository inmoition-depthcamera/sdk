#include "depth_camera_uvc.h"
#include "math.h"
#include "string.h"

DepthCameraUvcPort::DepthCameraUvcPort()
{
	SetUvcFrameCallBack(OnUvcFrame, this);

	mDepthFrame = NULL;
	mLastDepthFrame = NULL;
	mHdrDepthFrame = NULL;

	mOnDepthFrameCallBack = NULL;
	mOnDepthFrameCallBackParam = NULL;
	mHdrMode = false;
	mAlreadyPrepared = false;
	mD2PTable = NULL;
}

DepthCameraUvcPort::~DepthCameraUvcPort()
{
	if(mDepthFrame){
		delete mDepthFrame;
		mDepthFrame = NULL;
	}

	if(mLastDepthFrame){
		delete mLastDepthFrame;
		mLastDepthFrame = NULL;
	}

	if(mHdrDepthFrame){
		delete mHdrDepthFrame;
		mHdrDepthFrame = NULL;
	}

	if (mD2PTable) delete[] mD2PTable;
}

bool DepthCameraUvcPort::Open(std::string &camera_name)
{
	bool res = UVC_INTERFACE_DRIVER::Open(camera_name);

	if (res) {		
		int32_t w = mUvcWidth / 2;
		int32_t h = mUvcHeight;

		switch (mUvcWidth) {
		case 480: w = 320; break;
		case 120: w = 80; break;
		}

		if (mDepthFrame == NULL || mDepthFrame->w != w || mDepthFrame->h != h) {

			if(mDepthFrame) delete mDepthFrame;
			if(mLastDepthFrame) delete mLastDepthFrame;
			if(mHdrDepthFrame) delete mHdrDepthFrame;

			if (mD2PTable) delete[] mD2PTable;

			mDepthFrame     = new DepthFrame(w, h);
			mLastDepthFrame = new DepthFrame(w, h);
			mHdrDepthFrame = new DepthFrame(w, h);
			mD2PTable = new float[w * h];
		}

		if (camera_name.find("IDC8060S") != string::npos){
			mHFocal = mWFocal = 1.62f / 0.03f;  // Focal Length 1.62mm / pixel size 0.03mm
		}
		else if (camera_name.find("ILD38TOF") != string::npos){
			mHFocal = mWFocal = 6.0f / 0.03f;  // Focal Length 6mm / pixel size 0.03mm
		}
		else if (camera_name.find("IDC3224S") != string::npos ||
			camera_name.find("IRDC3224") != string::npos){
			mHFocal = mWFocal = 3.6f / 0.015f;  // Focal Length 3.6mm / pixel size 0.015mm
		}
		else if (camera_name.find("IDC3224L") != string::npos){
			mHFocal = mWFocal = 2.3f / 0.015f;  // Focal Length 2.3mm / pixel size 0.015mm
		}
		else {
			mHFocal = mWFocal = 247.0f;
		}

		// Create lookup table
		float k1 = 1 / mWFocal;
		float k2 = 1 / mHFocal;
		float *p = mD2PTable;
		for (int32_t y = 0; y < h; y++)
			for (int32_t x = 0; x < w; x++)
				*p++ = (float)(1.0 / sqrt(1 + k1 * k1 * (w / 2 - x) * (w / 2 - x) + k2 * k2 * (h / 2 - y) * (h / 2 - y)));

		mAlreadyPrepared = true;
	}
	return res;
}

bool DepthCameraUvcPort::Close()
{
	bool res = UVC_INTERFACE_DRIVER::Close();

	mAlreadyPrepared = false;
	
	return res;
}

bool DepthCameraUvcPort::GetDepthCameraList(vector<string>& camera_list)
{
	return UVC_INTERFACE_DRIVER::GetUvcCameraList(camera_list, "INMOTION");
}

void DepthCameraUvcPort::SetDepthFrameCallback(std::function<void(const DepthFrame*, void*)>cb, void * param)
{
	mOnDepthFrameCallBack = cb;
	mOnDepthFrameCallBackParam = param;
}

bool DepthCameraUvcPort::GetDepthFrame(DepthFrame *df)
{
	std::lock_guard<std::mutex> lck(mMutex);
	return mDepthFrame->CopyTo(df);
}

int32_t DepthCameraUvcPort::DepthToPointCloud(const DepthFrame *df, float * point_clould)
{
	const uint16_t* p_buf = df->phase;

	int32_t w = mDepthFrame->w;
	int32_t h = mDepthFrame->h;
	float *table = mD2PTable;

	int32_t totalPoint = 0;

	for (int32_t y = 0; y < h; y++) {
		for (int32_t x = 0; x < w; x++) {
			float z = (*p_buf)  * (*table);//z
			*point_clould++ = -(float)(x - w / 2) * z / mWFocal;//x
			*point_clould++ = -(float)(y - h / 2) * z / mHFocal;//y
			*point_clould++ = z;
			totalPoint++;
			p_buf++;
			table++;
		}
	}
	return totalPoint;
}

int32_t DepthCameraUvcPort::DepthToPointCloud(const DepthFrame *df, float * point_clould, uint16_t phaseMax, uint16_t amplitudeMin)
{
	const uint16_t* p_buf = df->phase;
	const uint16_t* a_buf = df->amplitude;
	
	int32_t w = mDepthFrame->w;
	int32_t h = mDepthFrame->h;
	float *table = mD2PTable;

	int32_t totalPoint = 0;

	for (int32_t y = 0; y < h; y++){
		for (int32_t x = 0; x < w; x++){
			if (*p_buf > 0 && *p_buf <= phaseMax && *a_buf >= amplitudeMin){
				float z = (*p_buf)  * (*table);//z
				*point_clould++ = -(float)(x - w / 2) * z / mWFocal;//x
				*point_clould++ = -(float)(y - h / 2) * z / mHFocal;//y
				*point_clould++ = z;
				totalPoint++;
			}
			a_buf++;
			p_buf++;
			table++;
		}
	}
	return totalPoint;
}

void DepthCameraUvcPort::OnUvcFrame(double sample_time, uint8_t * frame_buf, int32_t frame_buf_len, void * param)
{
	DepthCameraUvcPort * dc = (DepthCameraUvcPort *)param;

	if(dc->mAlreadyPrepared)
		dc->SplitUvcFrameToDepthFrame(frame_buf, frame_buf_len);
}

void DepthCameraUvcPort::SplitUvcFrameToDepthFrame(uint8_t * frame_buf, int32_t frame_buf_len)
{
	int32_t w = mDepthFrame->w;
	int32_t h = mDepthFrame->h;
	uint16_t* phase_buffer = mDepthFrame->phase;
	uint16_t* amplitude_buffer = mDepthFrame->amplitude;
	uint8_t*  ambient_buffer = mDepthFrame->ambient;
	uint8_t*  flags_buffer = mDepthFrame->flags;
	
	int32_t total_size = mUvcHeight * mUvcWidth * 2;
	if (frame_buf_len != total_size)
		return;

	mMutex.lock();
	if ((mUvcWidth == 120) || (mUvcWidth == 480)) { // 3 bytes per pixel
		uint8_t *ptr8 = frame_buf;
		for (int32_t i = 0; i < h; i++) {			
			for (int32_t j = 0; j < w; j++) {
				*amplitude_buffer++ = (uint16_t)*ptr8++;
			}
			uint16_t *ptr16 = (uint16_t *)ptr8;
			for (int32_t j = 0; j < w; j++) {
				uint16_t v = *ptr16++;
				*phase_buffer++ = v & 0xFFF;
				*ambient_buffer++ = (v & 0x7000) >> 11;
				*flags_buffer++ = (v & 0x8000) >> 12;
			}
			ptr8 = (uint8_t *)ptr16;
		}
	}
	else { // 4 bytes per pixel
		uint16_t * framePtr = (uint16_t *)frame_buf;
		for (int32_t i = 0, j; i < h; i++){
			for (j = 0; j < w; j++){
				*amplitude_buffer++ = (*framePtr & 0x0FFF);
				*ambient_buffer++ = ((*framePtr & 0xF000) >> 12);
				framePtr++;
			}
			for (; j < w * 2; j++){
				*phase_buffer++ = (*framePtr & 0x0FFF);
				if (h == 60)
					*flags_buffer++ = ((*framePtr & 0xC000) >> 12);
				else
					*flags_buffer++ = ((*framePtr & 0xF000) >> 12);
				framePtr++;
			}
		}
	}

	DepthFrame *out_frame = mDepthFrame;
	if(mHdrMode){
		int32_t frame_size = w * h;
		for(int i = 0 ; i < frame_size; i ++){
			// if this frame is overexposed and last is not, use last
			if(mDepthFrame->flags[i] && (!mLastDepthFrame->flags[i])){
				mDepthFrame->phase[i]     = mLastDepthFrame->phase[i];
				mDepthFrame->amplitude[i] = mLastDepthFrame->amplitude[i];
				mDepthFrame->ambient[i]   = mLastDepthFrame->ambient[i];
				mDepthFrame->flags[i]     = 0;
			}
		}
		// save current from to last frame
		mLastDepthFrame->CopyFrom(mDepthFrame);
	}
	mMutex.unlock();

	if (mOnDepthFrameCallBack)
		mOnDepthFrameCallBack(out_frame, mOnDepthFrameCallBackParam);
}

void DepthCameraUvcPort::SetHdrMode(bool enable)
{
	mHdrMode = enable;
}



////////// DepthFrame Class /////////////
DepthFrame::DepthFrame(int32_t _w, int32_t _h)
{
	w = _w;
	h = _h;
	phase     = new uint16_t[w * h];
	amplitude = new uint16_t[w * h];
	ambient   = new uint8_t[w * h];
	flags     = new uint8_t[w * h];
}

DepthFrame::~DepthFrame()
{
	delete[] phase;
	delete[] amplitude;
	delete[] ambient;
	delete[] flags;
}

bool DepthFrame::CopyFrom(DepthFrame *df)
{
	if(!df || df->w != w || df->h != h)
		return false;
	int size = w * h;
	if (df->phase)     memcpy(phase,     df->phase,      size * sizeof(uint16_t));
	if (df->amplitude) memcpy(amplitude, df->amplitude,  size * sizeof(uint16_t));
	if (df->ambient)   memcpy(ambient,   df->ambient,    size * sizeof(uint8_t));
	if (df->flags)     memcpy(flags,     df->flags,      size * sizeof(uint8_t));
	return true;
}

bool DepthFrame::CopyTo(DepthFrame *df)
{
	if(!df || df->w != w || df->h != h)
		return false;
	int size = w * h;
	if (df->phase)     memcpy(df->phase,     phase,      size * sizeof(uint16_t));
	if (df->amplitude) memcpy(df->amplitude, amplitude,  size * sizeof(uint16_t));
	if (df->ambient)   memcpy(df->ambient,   ambient,    size * sizeof(uint8_t));
	if (df->flags)     memcpy(df->flags,     flags,      size * sizeof(uint8_t));
	return true;
}