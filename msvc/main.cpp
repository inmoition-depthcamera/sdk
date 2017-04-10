
#include "../source/depth_camera_cmd.h"
#include "../source/depth_camera_uvc.h"

#include "iostream"

void main()
{
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;

	std::vector<std::string> camera_list;
	std::vector<std::string> cmd_list;

	uvc_port.GetDepthCameraList(camera_list);

	if (camera_list.size() > 0) {
		string cmd_port_name;
		cmd_port.GetUvcRelatedCmdPort(camera_list[0], cmd_port_name);
		cmd_port.Open(cmd_port_name);
		uvc_port.Open(camera_list[0]);
	}
	
	char status[4096];
	cmd_port.GetSystemStatus(status, 1024);
	cout << status << endl;

	cmd_port.GetCameraConfig(status, 4096);
	cout << status << endl;

	char ch;
	std::cin >> ch;
}