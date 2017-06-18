#include "model_view.h"

#include <GLFW/glfw3.h>
#include "camera.h"
#include <math.h>

glModelView::glModelView()
{
	Reset();

	mDrawCallBack = NULL;
	mDrawCallBackParam = NULL;

	mSizeChanged = false;

	mWidth = 0;
	mHeight = 0;
}

glModelView::~glModelView()
{
}

void glModelView::Init(int w, int h)
{
	glShadeModel(GL_SMOOTH);                        // shading mathod: GL_SMOOTH or GL_FLAT
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          // 4-byte pixel alignment
													// enable /disable features
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_SCISSOR_TEST);

	// track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glClearColor(0, 0, 0, 0);   // background color
	glClearStencil(0);                              // clear stencil buffer
	glClearDepth(1.0f);                             // 0 is near, 1 is far
	glDepthFunc(GL_LEQUAL);

	// set up light colors (ambient, diffuse, specular)
	GLfloat lightKa[] = { .0f, .0f, .0f, 1.0f };      // ambient light
	GLfloat lightKd[] = { .9f, .9f, .9f, 1.0f };      // diffuse light
	GLfloat lightKs[] = { 1, 1, 1, 1 };               // specular light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

	// position the light in eye space
	float lightPos[4] = { 0, 0, 1, 0 };               // directional light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glEnable(GL_LIGHT0);                            // MUST enable each light source after configuration

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Resize(w, h);
}

void glModelView::SetDrawCallBack(void(*cb)(void *, void *), void *param)
{
	mDrawCallBack = cb;
	mDrawCallBackParam = param;
}

void glModelView::SetCameraPos(float x, float y, float z)
{
	mCameraPos[0] = x;
	mCameraPos[1] = y;
	mCameraPos[2] = z;
}

void glModelView::SetCameraDetaPos(float dx, float dy, float dz)
{
	mCameraPos[0] += dx;
	mCameraPos[1] += dy;
	mCameraPos[2] += dz;
}

void glModelView::SetCameraAngle(float ax, float ay, float az)
{
	mCameraAngle[0] = ax;
	mCameraAngle[1] = ay;
	mCameraAngle[2] = az;
}

void glModelView::SetCameraDetaAngle(float dax, float day, float daz)
{
	mCameraAngle[0] += dax;
	mCameraAngle[1] += day;
	mCameraAngle[2] += daz;
}

void glModelView::Resize(int w, int h)
{
	mWidth = w;
	mHeight = h;
	mSizeChanged = true;
}

void glModelView::ZoomCamera(float deta)
{
	mCameraPos[2] += deta;
}

void glModelView::SetProjection(float l, float r, float b, float t, float n, float f)
{
	mProjections[0] = l;
	mProjections[1] = r;
	mProjections[2] = b;
	mProjections[3] = t;
	mProjections[4] = n;
	mProjections[5] = f;
}

void glModelView::Reset()
{
	mShowFrustum = mShowAxis = true;
	mShowCamera = mShowGrid = true;

	mMinRange = 1.0f;
	mMaxRange = 7.5f;

	mCameraAngle[0] = 45.0f;   // x
	mCameraAngle[1] = -45.0f;  // y
	mCameraAngle[2] = 0;       // z

	mCameraPos[0] = 0;
	mCameraPos[1] = 0;
	mCameraPos[2] = mMaxRange * 2;	

	mProjections[0] = -0.65f; // left
	mProjections[1] = 0.65f; // right
	mProjections[2] = -0.65f; // bottom
	mProjections[3] = 0.65f; // top
	mProjections[4] = mMinRange; // near
	mProjections[5] = mMaxRange; // far

	//double fov = atan(-mProjections[0]) * 2 / 3.14159265 * 180.0;
}

void glModelView::ComputeFrustumVertices(float l, float r, float b, float t, float n, float f)
{
	float ratio;
	float farLeft;
	float farRight;
	float farBottom;
	float farTop;

	// perspective mode
	ratio = f / n;
	farLeft = l * ratio;
	farRight = r * ratio;
	farBottom = b * ratio;
	farTop = t * ratio;

	// compute 8 vertices of the frustum
	// near top right
	mFrustumVertices[0].x = r;
	mFrustumVertices[0].y = t;
	mFrustumVertices[0].z = -n;

	// near top left
	mFrustumVertices[1].x = l;
	mFrustumVertices[1].y = t;
	mFrustumVertices[1].z = -n;

	// near bottom left
	mFrustumVertices[2].x = l;
	mFrustumVertices[2].y = b;
	mFrustumVertices[2].z = -n;

	// near bottom right
	mFrustumVertices[3].x = r;
	mFrustumVertices[3].y = b;
	mFrustumVertices[3].z = -n;

	// far top right
	mFrustumVertices[4].x = farRight;
	mFrustumVertices[4].y = farTop;
	mFrustumVertices[4].z = -f;

	// far top left
	mFrustumVertices[5].x = farLeft;
	mFrustumVertices[5].y = farTop;
	mFrustumVertices[5].z = -f;

	// far bottom left
	mFrustumVertices[6].x = farLeft;
	mFrustumVertices[6].y = farBottom;
	mFrustumVertices[6].z = -f;

	// far bottom right
	mFrustumVertices[7].x = farRight;
	mFrustumVertices[7].y = farBottom;
	mFrustumVertices[7].z = -f;

	// compute normals
	mFrustumNormals[0] = (mFrustumVertices[5] - mFrustumVertices[1]).cross(mFrustumVertices[2] - mFrustumVertices[1]);
	mFrustumNormals[0].normalize();

	mFrustumNormals[1] = (mFrustumVertices[3] - mFrustumVertices[0]).cross(mFrustumVertices[4] - mFrustumVertices[0]);
	mFrustumNormals[1].normalize();

	mFrustumNormals[2] = (mFrustumVertices[6] - mFrustumVertices[2]).cross(mFrustumVertices[3] - mFrustumVertices[2]);
	mFrustumNormals[2].normalize();

	mFrustumNormals[3] = (mFrustumVertices[4] - mFrustumVertices[0]).cross(mFrustumVertices[1] - mFrustumVertices[0]);
	mFrustumNormals[3].normalize();

	mFrustumNormals[4] = (mFrustumVertices[1] - mFrustumVertices[0]).cross(mFrustumVertices[3] - mFrustumVertices[0]);
	mFrustumNormals[4].normalize();

	mFrustumNormals[5] = (mFrustumVertices[7] - mFrustumVertices[4]).cross(mFrustumVertices[5] - mFrustumVertices[4]);
	mFrustumNormals[5].normalize();
}

void glModelView::DrawFrustum()
{
	ComputeFrustumVertices(mProjections[0], mProjections[1], mProjections[2], mProjections[3], mProjections[4], mProjections[5]);

	float colorLine1[4] = { 0.7f, 0.7f, 0.7f, 0.7f };
	float colorLine2[4] = { 0.2f, 0.2f, 0.2f, 0.7f };
	float colorPlane1[4] = { 0.5f, 0.5f, 0.5f, 0.2f };

	// draw lines
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_LINES);
	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(&mFrustumVertices[4].x);

	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(&mFrustumVertices[5].x);

	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(&mFrustumVertices[6].x);

	glColor4fv(colorLine2);
	glVertex3f(0, 0, 0);
	glColor4fv(colorLine1);
	glVertex3fv(&mFrustumVertices[7].x);
	glEnd();

	glColor4fv(colorLine1);
	glBegin(GL_LINE_LOOP);
	glVertex3fv(&mFrustumVertices[4].x);
	glVertex3fv(&mFrustumVertices[5].x);
	glVertex3fv(&mFrustumVertices[6].x);
	glVertex3fv(&mFrustumVertices[7].x);
	glEnd();

	glColor4fv(colorLine1);
	glBegin(GL_LINE_LOOP);
	glVertex3fv(&mFrustumVertices[0].x);
	glVertex3fv(&mFrustumVertices[1].x);
	glVertex3fv(&mFrustumVertices[2].x);
	glVertex3fv(&mFrustumVertices[3].x);
	glEnd();

	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);

	// frustum is transparent.
	// Draw the frustum faces twice: backfaces first then frontfaces second.
	for (int i = 0; i < 2; ++i)
	{
		if (i == 0)
		{
			// for inside planes
			glCullFace(GL_FRONT);
			glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		}
		else
		{
			// draw outside planes
			glCullFace(GL_BACK);
			glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
		}

		glColor4fv(colorPlane1);
		glBegin(GL_QUADS);
		// left
		glNormal3fv(&mFrustumNormals[0].x);
		glVertex3fv(&mFrustumVertices[1].x);
		glVertex3fv(&mFrustumVertices[5].x);
		glVertex3fv(&mFrustumVertices[6].x);
		glVertex3fv(&mFrustumVertices[2].x);
		// right
		glNormal3fv(&mFrustumNormals[1].x);
		glVertex3fv(&mFrustumVertices[0].x);
		glVertex3fv(&mFrustumVertices[3].x);
		glVertex3fv(&mFrustumVertices[7].x);
		glVertex3fv(&mFrustumVertices[4].x);
		// bottom
		glNormal3fv(&mFrustumNormals[2].x);
		glVertex3fv(&mFrustumVertices[2].x);
		glVertex3fv(&mFrustumVertices[6].x);
		glVertex3fv(&mFrustumVertices[7].x);
		glVertex3fv(&mFrustumVertices[3].x);
		// top
		glNormal3fv(&mFrustumNormals[3].x);
		glVertex3fv(&mFrustumVertices[0].x);
		glVertex3fv(&mFrustumVertices[4].x);
		glVertex3fv(&mFrustumVertices[5].x);
		glVertex3fv(&mFrustumVertices[1].x);
		// front
		glNormal3fv(&mFrustumNormals[4].x);
		glVertex3fv(&mFrustumVertices[0].x);
		glVertex3fv(&mFrustumVertices[1].x);
		glVertex3fv(&mFrustumVertices[2].x);
		glVertex3fv(&mFrustumVertices[3].x);
		// back
		glNormal3fv(&mFrustumNormals[5].x);
		glVertex3fv(&mFrustumVertices[7].x);
		glVertex3fv(&mFrustumVertices[6].x);
		glVertex3fv(&mFrustumVertices[5].x);
		glVertex3fv(&mFrustumVertices[4].x);
		glEnd();
	}
}

void glModelView::Render(void *render_param)
{
	// set bottom viewport (perspective)
	glViewport(0, 0, mWidth, mHeight);
	glScissor(0, 0, mWidth, mHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble aspectRatio = mWidth * 1.0f / mHeight;
	//glFrustum(-mFovY * aspectRatio, mFovY * aspectRatio,-mFovY, mFovY,1, 1000);
	glFrustum(mProjections[0] * aspectRatio, mProjections[1] * aspectRatio, mProjections[2], mProjections[3], 1, 1000);

	// switch to modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);   // background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glPushMatrix();

	// First, transform the camera (viewing matrix) from world space to eye space
	glTranslatef(-mCameraPos[0], -mCameraPos[1], -mCameraPos[2]);
	glRotatef(mCameraAngle[0], 1, 0, 0); // pitch
	glRotatef(mCameraAngle[1], 0, 1, 0); // heading
	glRotatef(mCameraAngle[2], 0, 0, 1); // yaw

	if(mShowGrid)
		DrawGrid(mMaxRange, mMaxRange / 20);

	if(mShowAxis)
		DrawAxis(mMaxRange);

	// Draw User funciton
	if (mDrawCallBack)
		mDrawCallBack(mDrawCallBackParam, render_param);

	// draw the camera
	glPushMatrix();
	glTranslatef(0, 0, mMaxRange / 2);

	if (mShowCamera) {
		glPushMatrix();
		glScalef(0.5f, 0.5f, 0.5f);
		DrawCamera();
		glPopMatrix();
	}
		
	if(mShowFrustum)
		DrawFrustum();
	glPopMatrix();

	glPopMatrix();

}

void glModelView::DrawGrid(float size, float step)
{
	// disable lighting
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);

	glColor3f(0.2f, 0.2f, 0.2f);
	for (float i = step; i <= size; i += step)
	{
		glVertex3f(-size, 0, i);   // lines parallel to X-axis
		glVertex3f(size, 0, i);
		glVertex3f(-size, 0, -i);   // lines parallel to X-axis
		glVertex3f(size, 0, -i);

		glVertex3f(i, 0, -size);   // lines parallel to Z-axis
		glVertex3f(i, 0, size);
		glVertex3f(-i, 0, -size);   // lines parallel to Z-axis
		glVertex3f(-i, 0, size);
	}

	// x-axis
	glColor3f(0.5f, 0, 0);
	glVertex3f(-size, 0, 0);
	glVertex3f(size, 0, 0);

	// z-axis
	glColor3f(0, 0, 0.5f);
	glVertex3f(0, 0, -size);
	glVertex3f(0, 0, size);

	glEnd();
	// enable lighting back
	glEnable(GL_LIGHTING);
}

void glModelView::DrawAxis(float size)
{
	glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
	glDisable(GL_LIGHTING);

	// draw axis
	glLineWidth(3);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(size, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, size, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, size);
	glEnd();
	glLineWidth(1);

	// restore default settings
	glEnable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);
}

inline glModelView::Vector3 glModelView::Vector3::operator-(const glModelView::Vector3& rhs) const {
	return glModelView::Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
}

inline glModelView::Vector3& glModelView::Vector3::normalize() {
	//@@const float EPSILON = 0.000001f;
	float xxyyzz = x*x + y*y + z*z;
	//@@if(xxyyzz < EPSILON)
	//@@    return *this; // do nothing if it is ~zero vector

	//float invLength = invSqrt(xxyyzz);
	float invLength = 1.0f / sqrtf(xxyyzz);
	x *= invLength;
	y *= invLength;
	z *= invLength;
	return *this;
}

inline glModelView::Vector3 glModelView::Vector3::cross(const glModelView::Vector3& rhs) const {
	return glModelView::Vector3(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
}
