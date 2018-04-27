#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "definitions.h"

enum MovementType { FORWARD, BACKWARD, STRAFE_RIGHT, STRAFE_LEFT };

class Camera
{
public:
	Camera(float screen_w, float screen_h, glm::vec3 pos_cam, float scene_radius, int cam_type);

	~Camera();

	void SetType(int type);

	void Reset();
	void Center();

	void ResizeCamera(float fov, float width, float height);

	void Pan(float x_pan, float y_pan);
	void Rotate(float x_rot, float y_rot);
	void Move(MovementType movement);
	void Update();
	void UpdateProjection();
	const glm::mat4 GetProj() const;
	const glm::mat4 GetView() const;
	const int GetType() const;

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
	float m_xRotCam;
	float m_yRotCam;

	// Screen
	int m_width;
	int m_height;

	// Mouse
	int m_xClick;
	int m_yClick;
	float m_xRot;
	float m_yRot;

	glm::vec3 m_camPos;
	glm::vec3 m_direction;

	glm::mat4 m_view;
	glm::mat4 m_projection;

	glm::vec3 m_sceneCenter;
	float m_sceneRadius;

};

#endif
