
#include "depth_camera_cmd.h"
#include "depth_camera_uvc.h"

#include "iostream"

using namespace std;
using namespace chrono;

void OnDepthFrame(const DepthFrame *df, void*param){

	static auto last_time = system_clock::now();
	static auto last_avg_time = system_clock::now();
	static int32_t dt_avg_v = 0, total_count = 0;
	auto cur_time = system_clock::now();
	auto dt = duration_cast<milliseconds>(cur_time - last_time);

	total_count ++;

	if(total_count % 30 == 0){
		auto cur_avg_time = system_clock::now();
		auto dt_avg = duration_cast<milliseconds>(cur_avg_time - last_avg_time);
		dt_avg_v = dt_avg.count();
		last_avg_time = cur_avg_time;
	}

	last_time = cur_time;
	uint32_t dt_v = dt.count();
	printf("frame size: w = %d, h = %d, fps_rt: %0.02f fps: %0.02f (%d)\n",
		   df->w, df->h, 1000.0f / dt_v,
		   dt_avg_v ? 1000.0f * 30 / dt_avg_v : 0, total_count);
}

int main(int argc, char **argv)
{
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;

	std::vector<std::string> camera_list;
	string cmd_port_name;

	uvc_port.GetDepthCameraList(camera_list);
	for(auto name : camera_list){
		cout << name << endl;
	}

    if(camera_list.size() > 0){

		// get uvc relate cmd port(ttyACMx)
		cmd_port.GetUvcRelatedCmdPort(camera_list[0], cmd_port_name);

		// should open cmd port first
		cmd_port.Open(cmd_port_name);

		std::string status;
		cmd_port.GetSystemStatus(status);

		cout << status << endl;

		// setup depth data call back
		uvc_port.SetDepthFrameCallback(OnDepthFrame, nullptr);

		// open camera
		uvc_port.Open(camera_list[0]);
	}

	this_thread::sleep_for(chrono::seconds(10));

	cout << "close uvc port" << endl;
	uvc_port.Close();

	cout << "close cmd port" << endl;
	cmd_port.Close();

	cout << "app shutdown" << endl;
	return 0;
}