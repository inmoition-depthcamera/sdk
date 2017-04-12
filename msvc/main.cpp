
#include "../source/depth_camera_cmd.h"
#include "../source/depth_camera_uvc.h"

#include "iostream
int main(int argc, char **argv)
{
	print_devs();
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;

	std::vector<std::string> camera_list;
	string cmd_port_name;// = "/dev/ttyACM0";

	uvc_port.GetDepthCameraList(camera_list);

	if(camera_list.size() > 0){
		cmd_port.GetUvcRelatedCmdPort(camera_list[0], cmd_port_name);
		cout << "uvc related cmd port:" << cmd_port_name << endl;
		if(cmd_port.Open(cmd_port_name))
			cout << "cmd port: " << cmd_port_name << " Open Successed!" << endl;
		else {
			cout << "cmd port: " << cmd_port_name << " Open Failed!" << endl;
			return -1;
		}

		string status;
		cmd_port.GetSystemStatus(status);
		cout << status << endl;
	}

	char ch;
	std::cin >> ch;

	return 0;
}