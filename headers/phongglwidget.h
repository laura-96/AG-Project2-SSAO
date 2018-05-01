#ifndef PHONGGLWIDGET_H
#define PHONGGLWIDGET_H

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

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "definitions.h"
#include "model.h"
#include "Camera.h"


class PhongGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    PhongGLWidget(QString modelFilename, bool showFps, QWidget *parent = 0);
    ~PhongGLWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

	// Camera
	void sceneCameraType(int type);

public slots:
    void cleanup();
	void computeFps();
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

	// Camera
	void projectionTransform(); // Type of camera
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

	//Lighting
	void setLighting();

	// FPS
	//void computeFps();
	void showFps();
	
	/* Attributes */
	// Screen
	int m_width;
	int m_height;
	
	// Camera
	float m_ar;
	float m_fov;
	float m_fovIni;
	float m_radsZoom;

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
	int m_xRotPoint;
	int m_yRotPoint;
	
	int m_xClick;
	int m_yClick;
	float m_xRot;
	float m_yRot;
	float m_xRotCam;
	float m_yRotCam;
	int m_doingInteractive;

	Camera* cam;

	// Shaders
    QOpenGLShaderProgram *m_program;
	GLuint m_transLoc, m_projLoc, m_viewLoc;
	GLuint m_vertexLoc, m_normalLoc;
	GLuint m_matAmbLoc, m_matDiffLoc, m_matSpecLoc, m_matShinLoc;
	GLuint m_lightPosLoc, m_lightColLoc;

	// FPS
	//QTime m_time;
	int m_frameCount;
	float m_fps;
	bool m_showFps;

};

#endif
