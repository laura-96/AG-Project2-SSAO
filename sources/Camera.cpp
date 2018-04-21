#include "Camera.h"


Camera::Camera()
{
	// Type
	type = 0; //Default type is static

	// Screen
	m_width = 500;
	m_height = 500;

	// Camera
	m_ar = 1.0f;
	m_fov = PI / 3.0f;
	m_fovIni = m_fov;
	m_zNear = 0.1f;
	m_zFar = 100.0f;
	m_radsZoom = 0.0f;
	m_xPan = 0.0f;
	m_yPan = 0.0f;
}

Camera::~Camera()
{
}

void Camera::Reset()
{
	
	// Camera
	m_ar = 1.0f;
	m_fov = PI / 3.0f;
	m_fovIni = m_fov;
	m_zNear = 0.1f;
	m_zFar = 100.0f;
	m_radsZoom = 0.0f;
	m_xPan = 0.0f;
	m_yPan = 0.0f;

	//Mouse
	m_xRot = 0.0f;
	m_yRot = 0.0f;
	m_xClick = 0;
	m_yClick = 0;

}

void Camera::SetType(int type)
{
	this->type = type;
}

