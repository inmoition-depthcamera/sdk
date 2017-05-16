
#include <depth_camera_uvc.h>
#include <denoise_filter.h>
#include <GLFW/glfw3.h>


GLFWwindow* PointsCloudWnd = NULL;

bool MouseDownFlag = false, ShowHideFlag = false;
double LastMousePosX, LastMousePosY;
DenoiseFilter DepthCameraDenoiseFilter;

float Rotates[3] = { 0 , 0, 0 }, Scale = 5.0f, ObjPos[3] = { 0, 0, 0 };

static void DrawCone(float h, float r)
{
	glBegin(GL_QUAD_STRIP);
	int i = 0;
	for (i = 0; i <= 390; i += 15)
	{
		float p = i * 3.14f / 180;
		glVertex3f(0, 0, h);
		glVertex3f(r * sin(p), r * cos(p), 0.0f);
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 0.0f, 0.0f);
	for (i = 0; i <= 390; i += 15) {
		float p = i * 3.14f / 180;
		glVertex3f(r * sin(p), r * cos(p), 0.0f);
	}
	glEnd();
}

static void DrawBox(float size)
{
	glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, size);
	glVertex3f(size, 0.0f, size);
	glVertex3f(size, 0.0f, 0.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(size, 0.0f, 0.0f);
	glVertex3f(size, size, 0.0f);
	glVertex3f(0.0f, size, 0.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, size, 0.0f);
	glVertex3f(0.0f, size, size);
	glVertex3f(0.0f, 0.0f, size);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 1.0);
	glVertex3f(size, 0.0f, 0.0f);
	glVertex3f(size, 0.0f, size);
	glVertex3f(size, size, size);
	glVertex3f(size, size, 0.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 0.0);
	glVertex3f(0.0f, size, 0.0f);
	glVertex3f(0.0f, size, size);
	glVertex3f(size, size, size);
	glVertex3f(size, size, 0.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.0, 1.0, 1.0);
	glVertex3f(0.0f, 0.0f, size);
	glVertex3f(size, 0.0f, size);
	glVertex3f(size, size, size);
	glVertex3f(0.0f, size, size);
	glEnd();
}

static void DrawAxis(float len)
{
	glLineWidth(2);
	glColor3f(0.0, 0.0, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(-len, 0.0f, 0);
	glVertex3f(len, 0.0f, 0);
	glEnd();
	glColor3f(0.0, 1.0, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0, -len, 0);
	glVertex3f(0.0, len, 0);
	glEnd();
	glColor3f(1.0, 0.0, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0, 0.0f, -5);
	glVertex3f(0, 0.0f, 5);
	glEnd();

	glColor3f(0.0, 0.0, 1.0f);
	glPushMatrix();
	glTranslatef(len, 0, 0);
	glRotatef(90, 0, 1, 0);
	DrawCone(0.5f, 0.1f);
	glPopMatrix();

	glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();
	glTranslatef(0, len, 0);
	glRotatef(90, -1, 0, 0);
	DrawCone(0.5f, 0.1f);
	glPopMatrix();

	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslatef(0, 0, len);
	DrawCone(0.5f, 0.1f);
	glPopMatrix();
}

// Windows Events
static void pc_cursor_position_callback(GLFWwindow* window, double x, double y)
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

		LastMousePosX = x;
		LastMousePosY = y;
	}
}

static void pc_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if ((button == GLFW_MOUSE_BUTTON_LEFT) && action == GLFW_PRESS) {
		MouseDownFlag = 1;
		glfwGetCursorPos(window, &LastMousePosX, &LastMousePosY);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT) {
		MouseDownFlag = 0;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods == 2) {
		Rotates[0] = Rotates[1] = Rotates[2] = 0;
		Scale = 5.0f;
	}
}

static void pc_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Scale *= yoffset > 0 ? 1.01f : 0.99f;
}

static void pc_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width <= height) {
		float factor = (GLfloat)height / (GLfloat)width;
		glOrtho(-10.0f, 10.0f, -10.0f * factor, 10.0f * factor, -20.0f, 20.0f);
	}
	else {
		float factor = (GLfloat)width / (GLfloat)height;
		glOrtho(-10.0f * factor, 10.0f * factor, -10.0f, 10.0f, -20.0f, 20.0f);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void pc_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_LEFT:
			ObjPos[0] += 0.2f;
			break;
		case GLFW_KEY_RIGHT:
			ObjPos[0] -= 0.2f;
			break;
		case GLFW_KEY_UP:
			ObjPos[1] += 0.2f;
			break;
		case GLFW_KEY_DOWN:
			ObjPos[1] -= 0.2f;
			break;
		case GLFW_KEY_O:
			ObjPos[2] += 0.2f;
			break;
		case GLFW_KEY_P:
			ObjPos[2] -= 0.2f;
			break;
		}
	}
}

static void pc_close_callback(GLFWwindow* window)
{
	ShowHideFlag = false;
	glfwHideWindow(PointsCloudWnd);
}

static void DrawPointsCloud(GLFWwindow * window, DepthFrame *df, DepthCameraUvcPort *uvc, float scale)
{
	int fw, fh;
	static float *cloud_points = NULL;
	static uint16_t *filted_phase = NULL;
	static bool filter_init = false;

	int32_t frame_size = df->w * df->h;

	if (cloud_points == NULL)
		cloud_points = new float[frame_size * 3];

	if (filted_phase == NULL)
		filted_phase = new uint16_t[frame_size];

	glfwGetFramebufferSize(window, &fw, &fh);

	pc_size_callback(window, fw, fh);

#ifdef _DEBUG
	uvc->ToPointsCloud(df, cloud_points, Scale * scale);
#else // use filter to denoise

	if (filter_init == false) {
		DepthCameraDenoiseFilter.Init(df->w, df->h);
		filter_init = true;
	}
	
	DepthCameraDenoiseFilter.Denoise(df->w, df->h, df->phase, df->amplitude, df->flags, filted_phase, 8);
	uvc->ToPointsCloud(filted_phase, df->w, df->h,  cloud_points, Scale * scale);

#endif
	glColor3f(1.0, 1.0, 1.0);
	glLoadIdentity();
	glPushMatrix();

	// Rotate the object
	glRotatef(Rotates[0], 1, 0, 0);
	glRotatef(Rotates[1], 0, 1, 0);
	glRotatef(Rotates[2], 0, 0, 1);

	glPushMatrix();
	float box_size = 0.3f;
	glTranslatef(-box_size / 2, -box_size / 2, -box_size / 2);
	DrawBox(box_size);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(ObjPos[0], ObjPos[1], ObjPos[2]);

	glColor3f(1.0, 1.0, 1.0);
	// Draw Points
	glBegin(GL_POINTS);
	float *ptr = cloud_points;
	for (int i = 0; i < frame_size; i++) {
		float x = *ptr++;
		float y = *ptr++;
		float z = *ptr++;
		glVertex3f(x, y, z);
	}
	glEnd();
	glPopMatrix();

	DrawAxis(5);

	glPopMatrix();
}

// Public functions
bool InitPointsCloudWindow(int32_t w, int32_t h) {

	if (PointsCloudWnd) {
		return false;
	}

	PointsCloudWnd = glfwCreateWindow(w * 2, h * 2, "Depth Points Cloud", NULL, NULL);
	if (!PointsCloudWnd)
		return false;

	// Hide the windows in start up
	glfwHideWindow(PointsCloudWnd);

	glfwSetCursorPosCallback(PointsCloudWnd, pc_cursor_position_callback);
	glfwSetMouseButtonCallback(PointsCloudWnd, pc_mouse_button_callback);
	glfwSetScrollCallback(PointsCloudWnd, pc_scroll_callback);
	glfwSetKeyCallback(PointsCloudWnd, pc_key_callback);
	glfwSetWindowCloseCallback(PointsCloudWnd, pc_close_callback);

	return true;
}

void UpdatePointsCloudWindow(bool *show_hide, DepthCameraUvcPort *uvc, DepthFrame *df, float scale) {
	static bool last_show_hide = false;
	if (last_show_hide != *show_hide) {
		if (*show_hide)
			glfwShowWindow(PointsCloudWnd);
		else
			glfwHideWindow(PointsCloudWnd);
		ShowHideFlag = *show_hide;
	}
	*show_hide = ShowHideFlag;
	last_show_hide = *show_hide;
	if (*show_hide) {
		glfwMakeContextCurrent(PointsCloudWnd);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawPointsCloud(PointsCloudWnd, df, uvc, scale);
		glfwSwapBuffers(PointsCloudWnd);
	}
}