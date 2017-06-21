
#include "depth_camera_cmd_video.h"
#include <sstream>
#include <iostream>
#include <chrono>

#define MAX_CMD_VIDEO_PACKAGE_SIZE 192000

DepthCameraCmdVideo::DepthCameraCmdVideo()
{
	mVideoMode = true;
	SetVideoModeRxDataCallBack(OnNewCmdDataCallBack, this);
	mHadFindHeader = false;
	mPackageHeaderCnt = mPackageTailCnt = mPackageSize = 0;
	mRxBuffer = new uint8_t[MAX_CMD_VIDEO_PACKAGE_SIZE];
	mIsCmdVideoOpened = false;
	mIsVideoOpened = false;
}

DepthCameraCmdVideo::~DepthCameraCmdVideo()
{
	delete[] mRxBuffer;
}

bool DepthCameraCmdVideo::GetCameraList(std::vector<std::string>& camera_list, const char * filter)
{
	std::vector<std::pair<std::string, std::string>> device_list;
	
	GetCmdDevices(device_list);
	
	for (auto dev : device_list) {
		camera_list.push_back(dev.first + "__" + dev.second);
	}
	return camera_list.size() > 0 ? true : false;
}

void DepthCameraCmdVideo::OnNewCmdDataCallBack(const uint8_t *pbuf, int32_t cnt, void *param)
{
	DepthCameraCmdVideo * cmd_video = (DepthCameraCmdVideo *)param;
	for (int i = 0; i < cnt; i++){
		uint8_t data = pbuf[i];
		if (data == 0xAA){
			cmd_video->mPackageHeaderCnt++;
			if (cmd_video->mPackageHeaderCnt == 8) { // find header
				cmd_video->mPackageHeaderCnt = 0;
				cmd_video->mHadFindHeader = true;
				cmd_video->mPackageSize = 0;
				continue;
			}
		}else{
			cmd_video->mPackageHeaderCnt = 0;
		}

		if (cmd_video->mHadFindHeader){
			cmd_video->mRxBuffer[cmd_video->mPackageSize++] = data;
			if (cmd_video->mPackageSize >= MAX_CMD_VIDEO_PACKAGE_SIZE){
				cmd_video->mHadFindHeader = false;
				cmd_video->mPackageSize = 0;
			}
			if (data == 0x55){
				cmd_video->mPackageTailCnt++;
				if (cmd_video->mPackageTailCnt == 8) { // find tail
					cmd_video->mPackageTailCnt = 0;
					cmd_video->mPackageSize -= 12; // 8 bytes tail + 4 bytes 0
					if (cmd_video->mPackageSize > 4){
						if (cmd_video->Crc32(cmd_video->mRxBuffer, cmd_video->mPackageSize, 0) == 0) {
							uint32_t *p32 = (uint32_t *)cmd_video->mRxBuffer;
							uint32_t type = *p32++;
							uint32_t len = *p32;
							switch (type) {
							case 0:// video
								cmd_video->mFrameCallBack(0, cmd_video->mRxBuffer + 16, 
									cmd_video->mPackageSize - 4 - 16, cmd_video->mFrameCallBackParam);
								break;
							case 1:// string
								if(cmd_video->mPackageSize - 4 - 16 == (int)len)
									cmd_video->ProcessCmdStr((const char *)(cmd_video->mRxBuffer + 16), len);
								break;
							}
						} else
                            cmd_video->mErrorCnt ++;
						cmd_video->mHadFindHeader = false;
					}
					cmd_video->mPackageSize = 0;
				}
			}
			else
				cmd_video->mPackageTailCnt = 0;
		}
	}
}

bool DepthCameraCmdVideo::GetDepthCameraList(vector<string>& camera_list)
{
	return GetCameraList(camera_list, "INMOTION");
}

bool DepthCameraCmdVideo::Open(std::string & camera_name)
{
	std::string com_name, status, dev_name;
	mVideoMode = true;
	GetUvcRelatedCmdPort(camera_name, com_name);
	bool ret = DepthCameraCmdPort::Open(com_name);
	if(ret == false){
		std::cout << "DepthCameraCmdVideo::Open  DepthCameraCmdPort::Open failed!" << endl;
		return false;
	}
	SendCmd("\r\n", 2);
	ret = ret && GetSystemStatus(status);
	if (ret) {
		std::stringstream ss;
		ss.str(status);
		string line;
		while (getline(ss, line)) {
			unsigned long pos;
			if ((pos = line.find("Product")) != string::npos) {
				dev_name = line.substr(pos + 2 + strlen("Product"));
				continue;
			}

			if ((pos = line.find("Video Width")) != string::npos) {
				string v = line.substr(pos + 2 + strlen("Video Width"));
				mVideoWidth = std::stoi(v);
				continue;
			}

			if ((pos = line.find("Video Height")) != string::npos) {
				string v = line.substr(pos + 2 + strlen("Video Height"));
				mVideoHeight = std::stoi(v);
				continue;
			}
		}
		ret = ret && DepthVideoInterface::Open(dev_name);
		if (ret) {
			ret = ret && VideoControl(true);
			if(ret)
				mIsCmdVideoOpened = true;
		}
	}else{
		std::cout << "DepthCameraCmdVideo::Open  GetSystemStatus failed!" << endl;
	}

	if (!ret)
		DepthCameraCmdPort::Close();
	
	return ret;
}

bool DepthCameraCmdVideo::Close()
{
	VideoControl(false);
	bool ret = DepthCameraCmdPort::Close();
	ret = ret && DepthVideoInterface::Close();
	mIsCmdVideoOpened = false;
	return ret;
}

bool DepthCameraCmdVideo::VideoControl(bool start_stop)
{
	bool ret = CdcVideoControl(start_stop);
	if (ret)
		mIsVideoOpened = start_stop;
	return ret;
}
