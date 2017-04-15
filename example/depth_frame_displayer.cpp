#include <iostream>

#include <depth_camera_cmd.h>
#include <depth_camera_uvc.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>

using namespace cv;
using namespace std;
using namespace chrono;

void cvDrawDepthFrame(const DepthFrame *df, const char *name, const char *text){
	const char *names[4] = {"Phase", "Amplitude", "Ambient", "Flags"};
	Mat frame_mat = Mat(df->h * 2, df->w * 2, CV_8UC1);
	Mat sub_frames[4];
	Rect sub_frame_rect[4] = {Rect(0, 0, df->w, df->h), Rect(df->w, 0, df->w, df->h),
							  Rect(0, df->h, df->w, df->h), Rect(df->w, df->h, df->w, df->h)};
	double roi_sums[4];
	Rect roi_rect = Rect(df->w / 2 - 3, df->h / 2 - 3, 6, 6);

	int types[4] = {CV_16UC1, CV_16UC1, CV_8UC1, CV_8UC1};
	void *datas[4] = {df->phase, df->amplitude, df->ambient, df->flags};
	double scales[4][2] = {{0, 3072}, {0, 255}, {0, 16}, {0, 8}};
	for(int i = 0 ; i < 4; i ++){
		Mat mat = Mat(df->h, df->w, types[i], datas[i]);
		Mat roi_mat = mat(roi_rect);
		sub_frames[i] = frame_mat(sub_frame_rect[i]); // get roi frame
		roi_sums[i] = sum(roi_mat).val[0];
		double alpha = 255.0 / (scales[i][1] - scales[i][0]);
		mat.convertTo(mat, CV_8U, alpha, -scales[i][0] *alpha);
		mat.copyTo(sub_frames[i]);
		mat.release();
	}
	char line_buf[128];
	int pos[4][2] = {{0, 10}, {df->w, 10}, {0, df->h + 10}, {df->w, df->h + 10}};
	for(int i = 0 ; i < 4; i ++){
		int size = roi_rect.size().area();
		sprintf(line_buf, "%s(%0.02f)", names[i], (float)roi_sums[i] / size);
		Size name_size = getTextSize(line_buf, CV_FONT_HERSHEY_SIMPLEX, 0.4, 1, 0);
		putText(frame_mat, line_buf, Point(pos[i][0] + (df->w - name_size.width) / 2, pos[i][1]), CV_FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 0, 0));
		rectangle(sub_frames[i], roi_rect, Scalar(255, 255, 255));
	}

	putText(frame_mat, text, Point(0, df->h * 2 - 4), CV_FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 0, 0));

	imshow(name, frame_mat);
	frame_mat.release();
}

void OnDepthFrame(const DepthFrame *df, void*param)
{
	const int FPS_AVG = 20;
	static bool first_display = true;
	static auto last_avg_time = system_clock::now();
	static int32_t dt_avg_v = 0, total_count = 0;

	if(total_count++ % FPS_AVG == 0){
		auto cur_avg_time = system_clock::now();
		auto dt_avg = duration_cast<milliseconds>(cur_avg_time - last_avg_time);
		dt_avg_v = dt_avg.count();
		last_avg_time = cur_avg_time;
	}
	char info[128];
	sprintf(info, "FPS %0.02f FRM %d", 1000.0f * FPS_AVG / dt_avg_v, total_count);
	cvDrawDepthFrame(df, "DepthFrame", info);

	if(first_display){
		first_display = false;
		moveWindow("DepthFrame", 100, 100);
	}
	// wait key to give window to update content
    cvWaitKey(1);
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

	// Grabber 10 seconds frame
	this_thread::sleep_for(chrono::seconds(100));

	cout << "close uvc port" << endl;
	uvc_port.Close();

	cout << "close cmd port" << endl;
	cmd_port.Close();

	cout << "app shutdown" << endl;
	return 0;
}