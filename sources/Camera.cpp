#include "Camera.h"

Camera::Camera(float screen_w, float screen_h, glm::vec3 pos_cam, float scene_radius, int cam_type)
{
	// Type
	type = cam_type;

	// Screen
	m_width = screen_w;
	m_height = screen_h;

	// Camera
	m_ar = 1.0f;
	m_fov = PI / 3.0f;
	m_fovIni = m_fov;
	m_radsZoom = 0.0f;
	m_xPan = 0.0f;
	m_yPan = 0.0f;

	m_xRotCam = 0.0f;
	m_yRotCam = 0.0f;
	
	m_camPos = pos_cam;
	m_zNear = 0.1;
	m_zFar = scene_radius * 6;

	m_sceneRadius = scene_radius;
	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	Update();
	UpdateProjection();
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
	m_radsZoom = 0.0f;
	m_xPan = 0.0f;
	m_yPan = 0.0f;

	m_xRotCam = 0.0f;
	m_yRotCam = 0.0f;

	m_camPos = glm::vec3(0.0f, 0.0f, -2.0f * m_sceneRadius);
	m_zNear = 0.1;
	m_zFar = m_sceneRadius * 6;

	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);

	m_projection = glm::perspective(m_fov, m_ar, m_zNear, m_zFar);
	Update();
	UpdateProjection();

}

void Camera::Center()
{
	m_camPos = glm::vec3(0.0f , 0.0f, -m_sceneRadius/2);
	Update();
	UpdateProjection();
}

void Camera::ResizeCamera(float fov, float width, float height)
{
	m_fov = fov;
	m_width = width;
	m_height = height;

	UpdateProjection();
}

void Camera::UpdateProjection()
{
	m_projection = glm::mat4(1.0f);
	m_zNear = 0.1;
	m_zFar = 3.0f * m_sceneRadius;

	m_projection = glm::perspective(m_fov, m_ar, m_zNear, m_zFar);
	
}

const int Camera::GetType() const
{
	return type;
}

void Camera::Update()
{
	if (type == 1)
	{
		glm::mat4 rotMatrix(1.0);
		rotMatrix = glm::rotate(rotMatrix, m_xRotCam, glm::vec3(0.0f, -1.0, 0.0));

		glm::vec4 forward = rotMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		glm::vec4 strafe = rotMatrix * glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec4 up = rotMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		
		glm::normalize(forward);
		glm::normalize(strafe);
		glm::normalize(up);

		glm::vec3 strafeDir(strafe);
		glm::vec3 eye_pos = -m_camPos;

		rotMatrix = glm::mat4(1.0);
		rotMatrix = glm::rotate(rotMatrix, m_yRotCam, strafeDir);

		glm::vec4 vrpDir = rotMatrix * forward;

		m_direction = (glm::vec3(vrpDir.x, vrpDir.y, vrpDir.z) * 0.5f) + eye_pos;

		m_view = glm::translate(m_view, m_sceneCenter + m_camPos);
		m_view = glm::translate(m_view, glm::vec3(m_xPan, -m_yPan, 0.0f));
		m_view = glm::translate(m_view, -m_sceneCenter);

		m_view = glm::lookAt(eye_pos, m_direction, glm::vec3(up)); //eye at up
	}
	else
	{
		m_view = glm::mat4(1.0f);

		m_view = glm::translate(m_view, m_sceneCenter + m_camPos);
		m_view = glm::translate(m_view, glm::vec3(m_xPan, -m_yPan, 0.0f));
		m_view = glm::translate(m_view, -m_sceneCenter);
	}
	
}

const glm::mat4 Camera::GetProj() const
{
	return m_projection;
}

const glm::mat4 Camera::GetView() const
{
	return m_view;
}

void Camera::Pan(float x_pan, float y_pan)
{
	m_xPan += x_pan;
	m_yPan += y_pan;

	Update();
}

void Camera::Rotate(float x_rot, float y_rot)
{
	m_xRotCam += x_rot;
	m_yRotCam += y_rot;

	Update();
}

void Camera::Move(MovementType movement)
{
	if (type == 1)
	{
		switch (movement)
		{
		case FORWARD:
			m_camPos += glm::normalize(glm::vec3(m_direction.x, 0.0f, m_direction.z));
			break;
		case BACKWARD:
			m_camPos -= glm::normalize(glm::vec3(m_direction.x, 0.0f, m_direction.z));
			break;
		case STRAFE_RIGHT:
			m_camPos += glm::normalize(glm::cross(glm::normalize(m_direction), glm::vec3(0.0f, 1.0f, 0.0f)));
			break;
		case STRAFE_LEFT:
			m_camPos -= glm::normalize(glm::cross(glm::normalize(m_direction), glm::vec3(0.0f, 1.0f, 0.0f)));
			break;
		}
		
		Update();
		UpdateProjection();
	}
	
}

void Camera::SetSceneCenter(glm::vec3 scene_center)
{
	m_sceneCenter = scene_center;
}

void Camera::SetType(int type)
{
	this->type = type;
}



