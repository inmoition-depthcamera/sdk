#include <iostream>

#include <depth_camera_cmd.h>
#include <depth_camera_uvc.h>
#include <denoise_filter.h>

#include <GLFW/glfw3.h>
#include "console_controller.h"

using namespace std;
using namespace chrono;

GLFWwindow* GlMainWindow;
float Rotates[3] = {0}, Scale = 1.0f;
bool MouseDownFlag = 0;
double LastMousePosX, LastMousePosY;
DenoiseFilter DepthCameraDenoiseFilter;
ConsoleController Console;

static void DisplayInfo() {

	const int FPS_AVG = 20;
	static auto last_avg_time = system_clock::now();
	static int32_t dt_avg_v = 0, total_count = 0, console_dly = 0;

	if (total_count++ % FPS_AVG == 0) {
		auto cur_avg_time = system_clock::now();
		auto dt_avg = duration_cast<milliseconds>(cur_avg_time - last_avg_time);
		dt_avg_v = (int32_t)dt_avg.count();
		last_avg_time = cur_avg_time;
	}

	if (console_dly++ % FPS_AVG == 0) {

		int32_t x, y;
		COORD win_size = Console.getWindowSize();
		Console.getCurPos(&x, &y);
		Console.moveCursor(0, win_size.Y);
		Console << "Frame Count: " << total_count << "\n";
		Console << "Frame Rate: " << 1000.0f * FPS_AVG / dt_avg_v << "\n";
		Console.moveCursor(x, y);
	}
}

static void DepthFrameToRgbFrame(DepthFrame *df,  uint8_t * out) {

	uint16_t *phase_ptr, *amplitude_ptr;
	uint8_t *flags_ptr, *ambient_ptr;

	phase_ptr = df->phase;
	amplitude_ptr = df->amplitude;
	ambient_ptr = df->ambient;
	flags_ptr = df->flags;

	for (int i = 0; i < df->h; i++) {
		for (int j = 0; j < df->w; j++) {
			uint8_t v = (*phase_ptr++) >> 4;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}

		for (int j = 0; j < df->w; j++) {
			uint8_t v = (*amplitude_ptr++) >> 4;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}
	}

	for (int i = 0; i < df->h; i++) {
		for (int j = 0; j < df->w; j++) {
			uint8_t v = (*ambient_ptr++) << 5;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}

		for (int j = 0; j < df->w; j++) {
			uint8_t v = (*flags_ptr++) << 4;
			*out++ = v;
			*out++ = v;
			*out++ = v;
		}
	}
}

static void DrawAllViews(DepthFrame *df, DepthCameraUvcPort *uvc) {
	int fw, fh, ww, wh;
	static uint8_t * rgb_buf = NULL;
	static float *cloud_points = NULL;
	static uint16_t *filted_phase = NULL;

	DepthCameraDenoiseFilter.Init(df->w, df->h);

	int32_t frame_size = df->w * df->h;
	if (rgb_buf == NULL) {
		rgb_buf = new uint8_t[frame_size * 3 * 4];
	}

	if(cloud_points == NULL){
		cloud_points = new float[frame_size * 3];
	}

	if (filted_phase == NULL) {
		filted_phase = new uint16_t[frame_size];
	}
	
	glfwGetFramebufferSize(GlMainWindow, &fw, &fh);
	glfwGetWindowSize(GlMainWindow, &ww, &wh);

	int32_t gw = (fw - fh) / 2;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw Points Cloud
	glViewport(fw - fh, 0, fh, fh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	DepthCameraDenoiseFilter.Denoise(df->w, df->h, df->phase, df->amplitude, df->flags, filted_phase, 64);
	int32_t cloud_points_count = uvc->DepthToPointCloud(filted_phase, df->w, df->h, cloud_points, 3.5f / 3072.0f);
	//int32_t cloud_points_count = uvc->DepthToPointCloud(df, cloud_points, 2 / 3072.0f);
	glPointSize(1.0f);
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();

	// Rotate the object
	glRotatef(Rotates[0], 1, 0, 0);
	glRotatef(Rotates[1], 0, 1, 0);
	glRotatef(Rotates[2], 0, 0, 1);

	// Scale the object
	glScalef(Scale, Scale, Scale);

	glVertexPointer(3, GL_FLOAT, 0, cloud_points);
	glDrawArrays(GL_POINTS, 0, cloud_points_count);
	
	glPopMatrix();

	// Draw Pixels
	glMatrixMode(GL_PROJECTION);
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0, 0);
	glLoadIdentity();
	glViewport(0, 0, fw - fh, fh);	
	DepthFrameToRgbFrame(df, rgb_buf);	
	glRasterPos2f(-1, 1);
	glPixelZoom((float)gw / (float)(df->w), -(float)fh / (float)(df->h * 2.0f));
	glDrawPixels(df->w * 2, df->h * 2, GL_RGB, GL_UNSIGNED_BYTE, rgb_buf);

	// Draw Grid
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex2f(-1.0, 0);
	glVertex2f(1.0f, 0);
	glVertex2f(0, -1.0);
	glVertex2f(0, 1.0);
	glVertex2f(1.0f, -1.0);
	glVertex2f(1.0f, 1.0);
	glEnd();

	
	DisplayInfo();
}

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	int wnd_width, wnd_height, fb_width, fb_height;
	double scale;

	if (MouseDownFlag) {
		glfwGetWindowSize(window, &wnd_width, &wnd_height);
		glfwGetFramebufferSize(window, &fb_width, &fb_height);

		scale = (double)fb_width / (double)wnd_width;

		x *= scale;
		y *= scale;

		Rotates[1] += (float)(x - LastMousePosX);
		Rotates[0] += (float)(y - LastMousePosY);

		// Remember cursor position
		LastMousePosX = x;
		LastMousePosY = y;
	}	
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if ((button == GLFW_MOUSE_BUTTON_LEFT) && action == GLFW_PRESS)
	{
		// Detect which of the four views was clicked
		MouseDownFlag = 1;
		glfwGetCursorPos(window, &LastMousePosX, &LastMousePosY);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		// Deselect any previously selected view
		MouseDownFlag = 0;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0) {
		Scale *= 1.01f;
	}
	else {
		Scale *= 0.99f;
	}
}

int main(int argc, char **argv)
{
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;

	Console.ShowCursor(false);

	std::vector<std::string> camera_list;
	string cmd_port_name;
	string uvc_name;
	uvc_port.GetDepthCameraList(camera_list);
    if(camera_list.size() > 0){
		// get uvc relate cmd port(ttyACMx)
		cmd_port.GetUvcRelatedCmdPort(camera_list[0], cmd_port_name);
		// should open cmd port first
		if (!cmd_port.Open(cmd_port_name)) {
			cout << "Can't Open cmd port" << endl;
			return -1;
		}
		uvc_name = camera_list[0].substr(camera_list[0].find_last_of('_') + 1);
		cout << "Opening " << uvc_name << "..." << endl;
		// open camera@
		if (!uvc_port.Open(camera_list[0])) {
			cout << "Can't Open uvc port" << endl;
			return -1;
		}
		// get current camera information
		std::string status;
		cmd_port.GetSystemStatus(status);
		cout << status << endl;
	} else {
		cout << "Can't Find Inmotion Depth Camera" << endl;
		return -1;
	}

	int32_t w, h;
	w = uvc_port.GetWidth();
	h = uvc_port.GetHeight();

	DepthFrame *df = new DepthFrame(w, h);
	// Initialize the library
	if (!glfwInit())
		return -1;

	GlMainWindow = glfwCreateWindow(w * 2 + h * 2, h * 2, uvc_name.c_str() , NULL, NULL);
	if (!GlMainWindow)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(GlMainWindow);
	const char *gl_version = (const char *)glGetString(GL_VERSION);
	if (gl_version)
		cout << "OpenGL Version: " << gl_version << endl;

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_VERTEX_ARRAY);

	glfwSetCursorPosCallback(GlMainWindow, cursor_position_callback);
	glfwSetMouseButtonCallback(GlMainWindow, mouse_button_callback);
	glfwSetScrollCallback(GlMainWindow, scroll_callback);

	while (!glfwWindowShouldClose(GlMainWindow))
	{
		if (uvc_port.GetDepthFrame(df)) {		

			DrawAllViews(df, &uvc_port);

			glfwSwapBuffers(GlMainWindow);
		}

		glfwPollEvents();
	}
	
	cout << "close uvc port" << endl;
	uvc_port.Close();

	cout << "close cmd port" << endl;
	cmd_port.Close();

	cout << "app shutdown" << endl;
	return 0;
}