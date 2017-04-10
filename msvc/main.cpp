
#include "../source/depth_camera_cmd.h"
#include "../source/depth_camera_uvc.h"

#include "iostream"

void main()
{
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;

	std::vector<std::string> camera_list;

	uvc_port.GetDepthCameraList(camera_list);

	if(camera_list.size() > 0)
		uvc_port.Open(camera_list[0]);

	char ch;
	std::cin >> ch;
}