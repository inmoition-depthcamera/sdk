
#include "../source/depth_camera_cmd.h"
#include "../source/depth_camera_uvc.h"

#include "iostream"

int main(int argc, char **argv)
{
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;

	std::vector<std::string> camera_list;
	string cmd_port_name = "/dev/ttyACM0";

	uvc_port.GetDepthCameraList(camera_list);
	
	char status[4096];
	cmd_port.GetSystemStatus(status, 1024);
	cout << status << endl;

	cmd_port.GetCameraConfig(status, 4096);
	cout << status << endl;

	char ch;
	std::cin >> ch;

	return 0;
}