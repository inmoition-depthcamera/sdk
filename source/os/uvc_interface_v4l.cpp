#include "uvc_interface_v4l.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dirent.h>

UvcInterfaceV4L::UvcInterfaceV4L()
{
	mFd = -1;
	mReadFrameThread = NULL;
	mReadFrameThreadExitFlag = true;
}

UvcInterfaceV4L::~UvcInterfaceV4L()
{
	Close();
}

bool UvcInterfaceV4L::GetUvcCameraList(std::vector<std::string>& camera_list, const char * filter)
{
	DIR *dir;
    struct dirent *ptr;
    if ((dir=opendir("/dev/")) == NULL)
        return false;
    while ((ptr=readdir(dir)) != NULL){
		if(ptr->d_type == 2){ // file
			if(strstr(ptr->d_name, "video")){
				std::string full_path = std::string("/dev/") + ptr->d_name;
				struct v4l2_capability cap;
				int ffd;
				if ((ffd = open(full_path.c_str(), O_RDWR)) != -1) {
					int rt = ioctl(ffd, VIDIOC_QUERYCAP, &cap);
					if(rt == 0 && strstr((const char *)cap.card, filter)){
						full_path += "__" + std::string((const char *)cap.bus_info) + 
									 "__" + std::string((const char *)cap.card);
						camera_list.push_back(full_path);
					}
					close(ffd);
				}
			}
		}		
    }
    closedir(dir);
	return true;
}

bool UvcInterfaceV4L::Open(std::string & camera_name)
{
	std::size_t pos = camera_name.find("__");
	mDeviceName = camera_name.substr(0, pos);
	if (InitV4L() == false) {
		fprintf(stderr, " Init v4L2 failed !! exit fatal \n");
		goto error;
	}

	if(StartStream() < 0){
		fprintf(stderr, " Start Stream failed \n");
		goto error;
	}

	mReadFrameThreadExitFlag = false;
	mReadFrameThread = new std::thread(ReadFrameThreadProc, this);
	return true;

error:
	close(mFd);
	mFd = -1;
	return false;
}

bool UvcInterfaceV4L::Close()
{
	if(mIsOpened == false)
		return true;

	mReadFrameThreadExitFlag = true;

	if (mIsOpened)
		StopStream();

    if(mReadFrameThread){
        if(mReadFrameThread->joinable())
            mReadFrameThread->join();

        delete mReadFrameThread;
        mReadFrameThread = NULL;
    }

	for (int i = 0; i < NB_BUFFER; i++){
        if(mMemBuffers[i])
            munmap(mMemBuffers[i], mUvcWidth * mUvcHeight * 2);
    }

    if(mFd != -1){
        close(mFd);
        mFd = -1;
    }

	return true;
}

bool UvcInterfaceV4L::InitV4L()
{
	int i;
	int ret = 0;
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers rb;
	struct v4l2_buffer buf;
	if ((mFd = open(mDeviceName.c_str(), O_RDWR)) == -1) {
		perror("ERROR opening V4L interface \n");
		exit(1);
	}
	memset(&cap, 0, sizeof (struct v4l2_capability));
	ret = ioctl(mFd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		fprintf(stderr, "Error opening device %s: unable to query device.\n",
			mDeviceName.c_str());
		return false;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		fprintf(stderr,
			"Error opening device %s: video capture not supported.\n",
			mDeviceName.c_str());
		return false;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "%s does not support streaming i/o\n",
			mDeviceName.c_str());
		return false;
	}

	/* set format in */
	memset(&fmt, 0, sizeof (struct v4l2_format));
	fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(mFd, VIDIOC_G_FMT, &fmt);
	if (ret < 0) {
		fprintf(stderr, "Unable to get format: %d.\n", errno);
		return false;
	}

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	mUvcWidth = fmt.fmt.pix.width;
	mUvcHeight = fmt.fmt.pix.height;
	
	ret = ioctl(mFd, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		fprintf(stderr, "Unable to set format: %d.\n", errno);
		return false;
	}
	
	/* request buffers */
	memset(&rb, 0, sizeof (struct v4l2_requestbuffers));
	rb.count = NB_BUFFER;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(mFd, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		fprintf(stderr, "Unable to allocate buffers: %d.\n", errno);
		return false;
	}
	/* map the buffers */
	for (i = 0; i < NB_BUFFER; i++) {
		memset(&buf, 0, sizeof (struct v4l2_buffer));
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(mFd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			fprintf(stderr, "Unable to query buffer (%d).\n", errno);
			return false;
		}
		mMemBuffers[i] = mmap(0,buf.length, PROT_READ, MAP_SHARED, mFd, buf.m.offset);
		if (mMemBuffers[i] == MAP_FAILED) {
			fprintf(stderr, "Unable to map buffer (%d)\n", errno);
			return false;
		}
	}
	/* Queue the buffers. */
	for (i = 0; i < NB_BUFFER; ++i) {
		memset(&buf, 0, sizeof (struct v4l2_buffer));
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(mFd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			fprintf(stderr, "Unable to queue buffer (%d).\n", errno);
			return false;
		}
	}
	return true;
}

int32_t UvcInterfaceV4L::StartStream()
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;
	ret = ioctl(mFd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		fprintf(stderr, "Unable to %s capture: %d.\n", "start", errno);
		return ret;
	}
	mIsOpened = 1;
	return 0;
}

int32_t UvcInterfaceV4L::StopStream()
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = ioctl(mFd, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		fprintf(stderr, "Unable to %s capture: %d.\n", "stop", errno);
		return ret;
	}

	mIsOpened = false;
	return 0;
}

void UvcInterfaceV4L::ReadFrameThreadProc(UvcInterfaceV4L *v4l_if)
{
	uint32_t frame_size = v4l_if->mUvcHeight * v4l_if->mUvcWidth * 2;
	struct v4l2_buffer v4l_buf;

	while(v4l_if->mReadFrameThreadExitFlag == false){
		memset(&v4l_buf, 0, sizeof (struct v4l2_buffer));
		v4l_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l_buf.memory = V4L2_MEMORY_MMAP;
		v4l_buf.length = frame_size;
		int ret = ioctl(v4l_if->mFd, VIDIOC_DQBUF, &v4l_buf);
		if (ret < 0) {
			fprintf(stderr, "Unable to dequeue buffer (%d).\n", errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		
		if(v4l_buf.bytesused == frame_size && v4l_if->mFrameCallBack){
			v4l_if->mFrameCallBack(0, (uint8_t *)v4l_if->mMemBuffers[v4l_buf.index], frame_size, v4l_if->mFrameCallBackParam);
		}

		ret = ioctl(v4l_if->mFd, VIDIOC_QBUF, &v4l_buf);
		if (ret < 0) {
			fprintf(stderr, "Unable to requeue buffer (%d).\n", errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}