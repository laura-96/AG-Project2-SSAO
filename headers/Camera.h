#ifndef __CAMERA_H__
#define __CAMERA_H__


#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "definitions.h"

class Camera
{
public:
	Camera();

	~Camera();

	void Reset();
	void SetType(int type);

private:

	int type;

	float m_ar;
	float m_fov;
	float m_fovIni;
	float m_zNear;
	float m_zFar;
	float m_radsZoom;
	float m_xPan;
	float m_yPan;
	glm::vec3 m_camPos;

	// Screen
	int m_width;
	int m_height;

	// Mouse
	int m_xClick;
	int m_yClick;
	float m_xRot;
	float m_yRot;

};

#endif
