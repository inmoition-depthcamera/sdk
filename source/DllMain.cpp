
#include "DepthCameraInput.h"
#include "Common.h"

#define DLL_VERSION (((1) << 24) | ((0) << 16) | (9)) //1.0.9

#define DLLCALL _stdcall
#define DLLEXP extern "C" __declspec(dllexport) 


DepthCameraInput DCI;

DLLEXP unsigned int DLLCALL GetSDKVersion()
{
	return DLL_VERSION;
}

DLLEXP const char * DLLCALL GetVideoName(int id)
{
	DCI.listVideos(true);
	return DCI.getVideoName(id);
}

DLLEXP int DLLCALL GetVideoNum()
{
	return DCI.listVideos(true);
}

DLLEXP int DLLCALL Close(int id)
{
	if (id >= 0)
		DCI.Close(id);
	return 1;
}

DLLEXP int DLLCALL OpenById(int id, int openCmd)
{
	return DCI.OpenById(id,openCmd);
}


DLLEXP int DLLCALL SetDepthCameraFrameCallBack(int id, OnDepthCameraFrameCallBack cb)
{
	return DCI.setDepthCameraFrameCallBack(id, cb) ? 1 : 0;
}

DLLEXP int DLLCALL IsFrameNew(int id)
{
	return DCI.isFrameNew(id) ? 1 : 0;
}

DLLEXP int DLLCALL ReadDepthCameraFrame(int id, float * angle, unsigned short * phase_buffer, unsigned short * amplitude_buffer, unsigned char * ambient_buffer, unsigned char * flags_buffer)
{
	return DCI.getPixels(id, angle, phase_buffer, amplitude_buffer, ambient_buffer, flags_buffer);
}


DLLEXP int DLLCALL IsVideoSetup(int id)
{
	return DCI.isVideoSetup(id);
}

DLLEXP void DLLCALL GetFrameSize(int id, int *w, int *h)
{
	if (DCI.isVideoSetup(id))
	{
		if (w) *w = DCI.getFrameWidth(id);
		if (h) *h = DCI.getFrameHeight(id);
	}
}

DLLEXP int DLLCALL GetCurrentFps(int id)
{
	if (DCI.isVideoSetup(id))
		return DCI.getCurrentFrameRate(id);
	return 0;
}

DLLEXP void DLLCALL Bit16ToRGB24(unsigned short *source, unsigned short source_maxValue, unsigned short source_minValue, int source_size, unsigned char *rgb_buf, int rgb_size, char color_flag)
{
	Common::Bit16ToRGB24(source, source_maxValue, source_minValue, source_size, rgb_buf, rgb_size, color_flag);
}

DLLEXP void DLLCALL Bit8ToRGB24(unsigned char *source, unsigned char source_maxValue, unsigned char source_minValue, int source_size, unsigned char *rgb_buf, int rgb_size, char color_flag)
{
	Common::Bit8ToRGB24(source, source_maxValue, source_minValue, source_size, rgb_buf, rgb_size, color_flag);
}

DLLEXP void DLLCALL PhaseToColor(float *rColor, float *gColor, float *bColor, unsigned short phaseValue, unsigned short phaseMax, unsigned short phaseMin)
{
	Common::PhaseToColor(rColor, gColor, bColor, phaseValue, phaseMax, phaseMin);
}

DLLEXP void DLLCALL DepthToPointCloud(int id ,unsigned short *phase, float *pc, float depth_scale)
{
	DCI.DepthToPointCloud(id, phase, pc);
}

DLLEXP int DLLCALL DepthToPointCloudColor(int id, unsigned short *phase, unsigned short *amplitude, unsigned short phaseMax, unsigned short amplitudeMin, float *pc, float *pcc, float depth_scale)
{
	return DCI.DepthToPointCloud(id, phase, amplitude, phaseMax, amplitudeMin,pc, pcc);
}

DLLEXP void DLLCALL PhaseDenoise(unsigned short *phase, unsigned short *amplitude, unsigned char *flags, unsigned short* DstFrame, int Amp_Thr)
{
	DCI.PhaseDenoise(phase, amplitude, flags, DstFrame, Amp_Thr);
}

DLLEXP int DLLCALL IsCmdBusy()
{
	return DCI.isCmdBusy();
}

DLLEXP int DLLCALL IsCmdStart()
{
	return DCI.isCmdStart() ;
}


DLLEXP int DLLCALL SetDepthCameraCmdCallBack(OnDepthCameraCmdCallBack cb)
{
	return DCI.setDepthCameraCmdCallBack(cb);
}

DLLEXP int DLLCALL WriteCmd(const char *sned_buffer, int32_t sned_len)
{
	if (DCI.isCmdBusy())
		return -1;
	return DCI.writeCmd(sned_buffer, sned_len);
}

DLLEXP int DLLCALL SetIntegrationTime(uint8_t value)
{
	return DCI.setIntegrationTime(value);
}

DLLEXP int DLLCALL SetExLight(uint8_t value)
{
	return DCI.setExLight(value);
}

DLLEXP int DLLCALL SetInLight(uint8_t value)
{
	return DCI.setInLight(value);
}

DLLEXP int DLLCALL SetFrameRate(uint16_t value)
{
	return DCI.setFrameRate(value);
}

DLLEXP int DLLCALL SetMotorSpeed(uint8_t value)
{
	return DCI.setMotorSpeed(value);
}

DLLEXP int DLLCALL SwitchMirror()
{
	return DCI.switchMirror();
}

DLLEXP int DLLCALL SetBinning(uint8_t rows, uint8_t columns)
{
	return DCI.setBinning(rows, columns);
}

DLLEXP int DLLCALL RestoreFactorySettings()
{
	return DCI.restoreFactorySettings();
}

DLLEXP int DLLCALL SaveConfig()
{
	return DCI.saveConfig();
}

DLLEXP int DLLCALL GetSystemStatus(char * read_buffer, int32_t buffer_size, int32_t *read_len)
{
	return DCI.getSystemStatus(read_buffer, buffer_size, read_len);
}

DLLEXP int DLLCALL UpdateFirmwareStart(int id, char* path)
{
	return DCI.updateFirmwareStart(id,path);
}

DLLEXP void DLLCALL UpdateFirmwareStop(int id)
{
    DCI.updateFirmwareStop(id);
}

DLLEXP int DLLCALL GetUpdateProgress()
{
	return DCI.getUpdateProgress();
}

DLLEXP int DLLCALL IsUpdating()
{
   return DCI.isUpdating();
}