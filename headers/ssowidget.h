#ifndef SSOWIDGET_H
#define SSOWIDGET_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QColor>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QTime>
#include <QWheelEvent>
#include <qopenglframebufferobject.h>

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "definitions.h"
#include "model.h"
#include "Camera.h"

class SSOWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
	Q_OBJECT

public:
	SSOWidget(QString modelFilename, bool showFps, QWidget *parent = 0);
	~SSOWidget();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	// Camera
	void sceneCameraType(int type);
	void activateSSAO(bool active);
	void setSSAOIntensity(double value);
	void activateDrawOnlySSAO(bool active);

	public slots:
	void cleanup();

signals:

protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int width, int height) override;

	// Keyboard and mouse interaction
	void keyPressEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	// Shaders
	void loadShaders();
	void reloadShaders();

	void loadGShader();
	void loadLightShader();

	// Camera
	void initCamera();
	void projectionTransform(bool useSSAO = false); // Type of camera
	void resetCamera();
	void viewTransform(); // Position of the camera

						  // Scene
	void changeBackgroundColor();
	void computeCenterRadiusScene();

	// Model
	void createBuffersModel();
	void cleanBuffersModel();
	void computeBBoxModel();
	void modelTransform(); // Position and orientation of the scene
	bool m_modelLoaded;

	// Quad
	void createBuffersQuad(); 

	// SSAO
	void createGBuffers();
	void createSSAOKernels();

	//Lighting
	void setLighting();

	// FPS
	void computeFps();
	void showFps();

	// Draw
	void GeometryPass(); // 1st Pass
	void LightPass(); // 3rd Pass

	/* Attributes */
	// Screen
	int m_width;
	int m_height;

	// Camera
	float m_ar;
	float m_fov;
	float m_fovIni;

	float m_radsZoom;
	float m_xPan;
	float m_yPan;

	float m_xRotCam;
	float m_yRotCam;

	int m_xRotPoint;
	int m_yRotPoint;

	int cam_type;
	Camera* camera;

	// Scene
	glm::vec3 m_sceneCenter;
	float m_sceneRadius;
	QColor m_bkgColor;
	bool m_backFaceCulling;

	// Model
	Model m_model;
	QString m_modelFilename;
	glm::vec3 m_modelCenter;
	float m_modelRadius;
	GLuint m_VAOModel, m_VBOModelVerts, m_VBOModelNorms;
	GLuint m_VBOModelMatAmb, m_VBOModelMatDiff, m_VBOModelMatSpec, m_VBOModelMatShin;

	// Lights
	glm::vec4 m_lightPos;
	glm::vec3 m_lightCol;

	// Mouse
	int m_xClick;
	int m_yClick;
	float m_xRot;
	float m_yRot;
	int m_doingInteractive;
	

	// Shaders
	GLuint m_matAmbLoc, m_matDiffLoc, m_matSpecLoc, m_matShinLoc;
	GLuint m_lightPosLoc, m_lightColLoc;

	// GPass Shader
	QOpenGLShaderProgram* gPass_program;
	GLuint gp_aPos, gp_aNormal, gp_aTexCoords;	// vertex
	GLuint gp_model, gp_view, gp_projection;	// vertex

	// Light Shader
	QOpenGLShaderProgram* light_program;
	GLuint light_vertex, light_texcoords, gPositionTex, gNormalTex, gAlbedo;
	GLuint light_projection, light_trans, light_view;
	GLuint texNoise, noiseTexture;
	GLuint useSSAO, ssaoIntensityLoc, drawSSAOLoc;

	// Quad
	GLuint quadVAO, quadVBOVert, quadVBOTexCoord;

	// FBO
	QOpenGLFramebufferObject* g_fbo;

	// Kernels
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;

	// FPS
	QTime m_time;
	int m_frameCount;
	float m_fps;
	bool m_showFps;

	bool flag_ssao = false;
	bool usingSSAO = true;
	float ssao_intensity = 0.2f;
	bool drawSSAO = false;

};

#endif
