#pragma once

#include <inttypes.h>


class glModelView
{
public:
	glModelView();
	~glModelView();

	void Init(int w, int h);
	void SetDrawCallBack(void(*cb)(void *, void *), void *param);
	void SetCameraPos(float x, float y, float z);
	void SetCameraDetaPos(float dx, float dy, float dz);
	void SetCameraAngle(float ax, float ay, float az);
	void SetCameraDetaAngle(float dax, float day, float daz);
	void Resize(int w, int h);
	void ZoomCamera(float deta);
	// left, right, bottom, top, near, far
	void SetProjection(float l, float r, float b, float t, float n, float f);
	void Reset();
	void Render(void *render_param = NULL);

	inline void SetRange(float max, float min) { mMaxRange = max; mMinRange = min; }
	inline void ShowAxis(bool flag) { mShowAxis = flag; }
	inline void ShowGrid(bool flag) { mShowGrid = flag; }
	inline void ShowCamera(bool flag) { mShowCamera = flag; }
	inline void ShowFrustum(bool flag) { mShowFrustum = flag; }

private:

	float mCameraPos[3];
	float mCameraAngle[3];
	float mMaxRange, mMinRange;

	int mWidth, mHeight;

	// left, right, bottom, top, near, far
	float mProjections[6];

	void(*mDrawCallBack)(void *, void *);
	void *mDrawCallBackParam;

	bool mSizeChanged, mShowAxis, mShowGrid, mShowCamera, mShowFrustum;

	void DrawGrid(float size, float step);
	void DrawAxis(float size);
	void DrawFrustum();
	void ComputeFrustumVertices(float l, float r, float b, float t, float n, float f);

	struct Vector3
	{
		float x, y, z;

		// ctors
		Vector3() : x(0), y(0), z(0) {};
		Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

		Vector3&    normalize();                            //
		Vector3     cross(const Vector3& vec) const;        // cross product
		Vector3     operator-(const Vector3& rhs) const;    // subtract rhs
	};

	Vector3 mFrustumVertices[8];         // 8 vertices of frustum
	Vector3 mFrustumNormals[6];          // 6 face normals of frustum

};
