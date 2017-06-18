

#include <iostream>
#include <stdio.h>
#include <depth_camera_cmd_video.h>

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
		dt_avg_v = (int32_t)dt_avg.count();
		last_avg_time = cur_avg_time;
	}

	last_time = cur_time;
	int32_t dt_v = (int32_t)dt.count();
	printf("frame size: w = %d, h = %d, fps_rt: %0.02f fps: %0.02f (%d)\n",
		   df->w, df->h, 1000.0f / dt_v,
		   dt_avg_v ? 1000.0f * 30 / dt_avg_v : 0, total_count);
}

int main(int argc, char **argv)
{
	DepthCameraCmdVideo cmd_video_port;

	std::vector<std::string> camera_list;
	string cmd_port_name;

	// get the valid camera names
	cmd_video_port.GetDepthCameraList(camera_list);
	for(auto name : camera_list){
		std::cout << name << endl;
	}

    if(camera_list.size() > 0){
		cmd_video_port.SetDepthFrameCallback(OnDepthFrame, nullptr);
		
		if (cmd_video_port.Open(camera_list[0])) {
			std::string status;
			cmd_video_port.GetSystemStatus(status);
			std::cout << status << endl;

			// Grabber 10 seconds frame
			this_thread::sleep_for(chrono::seconds(10));

			std::cout << "close uvc port" << endl;
			cmd_video_port.Close();
		}
	}
	std::cout << "app shutdown" << endl;
	return 0;
}