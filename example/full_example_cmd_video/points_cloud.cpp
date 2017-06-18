
#include <depth_camera_cmd_video.h>
#include <denoise_filter.h>
#include <GLFW/glfw3.h>
#include "model_view.h"

#define ENABLE_DINOISE_FILTER 1

GLFWwindow* PointsCloudWnd = NULL;

static bool MouseDownFlag = false, ShowHideFlag = false;
static double LastMousePosX, LastMousePosY;
static DenoiseFilter DepthCameraDenoiseFilter;
static glModelView ModelView;
static DepthCameraCmdVideo *CmdVideo;
static float UserScale = 1.0f, MaxRange = 7.5f;

static float Scale = 3.0f;

// Windows Events
static void pc_cursor_position_callback(GLFWwindow* window, double x, double y){
	if (MouseDownFlag) {
		ModelView.SetCameraDetaAngle((float)(y - LastMousePosY), (float)(x - LastMousePosX), 0);
		LastMousePosX = x;
		LastMousePosY = y;
	}
}

static void pc_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			MouseDownFlag = true;
			glfwGetCursorPos(window, &LastMousePosX, &LastMousePosY);
		}
		else {
			MouseDownFlag = false;
		}
	}

	if (action == GLFW_RELEASE) {
		static auto before = std::chrono::high_resolution_clock::now();
		auto now = std::chrono::high_resolution_clock::now();
		double diff_ms = std::chrono::duration <double, std::milli>(now - before).count();
		before = now;
		if (diff_ms > 30 && diff_ms < 300) {
			ModelView.Reset(); //double click reset
		}
	}
}

static void pc_scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	ModelView.ZoomCamera((float)(yoffset * 0.4));
}

static void pc_resize_callback(GLFWwindow* window, int w, int h){
	ModelView.Resize(w, h);
}

static void pc_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	static bool show_camera = true, show_axis = true, show_grid = true, show_frustum = true;
	static float camera_angles[4] = {0, 90, 180, 270};
	static int32_t camera_angle_indexs[3] = { 0, 1, 2};
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_C:
			show_camera = !show_camera;
			ModelView.ShowCamera(show_camera);
			break;
		case GLFW_KEY_A:
			show_axis = !show_axis;
			ModelView.ShowAxis(show_axis);
			break;
		case GLFW_KEY_G:
			show_grid = !show_grid;
			ModelView.ShowGrid(show_grid);
			break;
		case GLFW_KEY_F:
			show_frustum = !show_frustum;
			ModelView.ShowFrustum(show_frustum);
			break;
		case GLFW_KEY_1:
			ModelView.SetCameraAngle(camera_angles[camera_angle_indexs[0] & 0x03], 0, 0);
			camera_angle_indexs[0] ++;
			break;
		case GLFW_KEY_2:
			ModelView.SetCameraAngle(0, -camera_angles[camera_angle_indexs[1] & 0x03], 0);
			camera_angle_indexs[1] ++;
			break;
		case GLFW_KEY_3:
			ModelView.SetCameraAngle(0, 0, camera_angles[camera_angle_indexs[2] & 0x03]);
			camera_angle_indexs[2] ++;
			break;
		}
	}
}

static void pc_close_callback(GLFWwindow* window){
	ShowHideFlag = false;
	glfwHideWindow(PointsCloudWnd);
}

static void DrawCallBack(void *cb_param, void *render_param){

	static float *cloud_points = NULL;
	static uint16_t *filted_phase = NULL;
	static bool filter_init = false;
	int points_cnt;
	DepthFrame *df = (DepthFrame *)render_param;

	int32_t frame_size = df->w * df->h;

	if (cloud_points == NULL)
		cloud_points = new float[frame_size * 3];

	if (filted_phase == NULL)
		filted_phase = new uint16_t[frame_size];

	// use filter to denoise
#if ENABLE_DINOISE_FILTER
	if (filter_init == false) {
		DepthCameraDenoiseFilter.Init(df->w, df->h);
		filter_init = true;
	}
	DepthCameraDenoiseFilter.Denoise(df->w, df->h, df->phase, df->amplitude, df->flags, filted_phase, 8);
	points_cnt = CmdVideo->ToFiltedPointsCloud(filted_phase, df, cloud_points, Scale * UserScale, 4096, 8);
#else 
	points_cnt = CmdVideo->ToFiltedPointsCloud(NULL, df, cloud_points, Scale * UserScale);
#endif

	glDisable(GL_LIGHTING);
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);

	glRotatef(180, 0, 1.0f, 0.0f);
	glTranslatef(0, 0, -MaxRange / 2);
	
	// Draw Points
	glBegin(GL_POINTS);
	float *ptr = cloud_points;
	for (int i = 0; i < points_cnt; i++) {
		float x = *ptr++;
		float y = *ptr++;
		float z = *ptr++;
		glVertex3f(x, y, z);
	}
	glEnd();
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

// Public functions
bool InitPointsCloudWindow(int32_t w, int32_t h, DepthCameraCmdVideo *cmd_video, float scale, float max_range) {

	if (PointsCloudWnd) {
		return false;
	}

	// Hide the windows in start up
	glfwWindowHint(GLFW_VISIBLE, 0);
	PointsCloudWnd = glfwCreateWindow(w * 2, h * 2, "Depth Points Cloud", NULL, NULL);
	if (!PointsCloudWnd)
		return false;

	glfwSetCursorPosCallback(PointsCloudWnd, pc_cursor_position_callback);
	glfwSetMouseButtonCallback(PointsCloudWnd, pc_mouse_button_callback);
	glfwSetFramebufferSizeCallback(PointsCloudWnd, pc_resize_callback);
	glfwSetScrollCallback(PointsCloudWnd, pc_scroll_callback);
	glfwSetKeyCallback(PointsCloudWnd, pc_key_callback);
	glfwSetWindowCloseCallback(PointsCloudWnd, pc_close_callback);

	glfwMakeContextCurrent(PointsCloudWnd);
	ModelView.Init(w, h);
	ModelView.SetDrawCallBack(DrawCallBack, NULL);

	UserScale = scale;
	CmdVideo = cmd_video;
	MaxRange = max_range;


	return true;
}

void UpdatePointsCloudWindow(bool *show_hide, DepthFrame *df) {
	static bool last_show_hide = true;
	glfwMakeContextCurrent(PointsCloudWnd);
	if (last_show_hide != *show_hide) {
		if (*show_hide){
			glfwShowWindow(PointsCloudWnd);
		}else{
			glfwHideWindow(PointsCloudWnd);
		}
		ShowHideFlag = *show_hide;
	}
	*show_hide = ShowHideFlag;
	last_show_hide = ShowHideFlag;
	if (*show_hide) {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ModelView.Render(df);

		glfwSwapBuffers(PointsCloudWnd);
	}
}