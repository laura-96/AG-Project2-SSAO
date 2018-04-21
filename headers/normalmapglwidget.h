#ifndef NORMALMAPGLWIDGET_H
#define NORMALMAPGLWIDGET_H

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
#include <QOpenGLTexture>
#include <QTime>
#include <QWheelEvent>

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../headers/definitions.h"


class NormalMapGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    NormalMapGLWidget(QWidget *parent = 0);
    ~NormalMapGLWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

	// Textures
	void loadTex1Texture(QString filename);
	void deleteTex1Texture();
	void loadTex2Texture(QString filename);
	void deleteTex2Texture();

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

	// Camera
	void projectionTransform(); // Type of camera
	void resetCamera();
	void viewTransform(); // Position of the camera

	// Scene
	void changeBackgroundColor();
	void createBuffersScene();
	void computeBBoxScene();
	void sceneTransform(); // Position and orientation of the scene

	//Lighting
	void setLighting();

	/* Attributes */
	// Screen
	int m_width;
	int m_height;
	
	// Camera
	float m_ar;
	float m_fov;
	float m_fovIni;
	float m_zNear;
	float m_zFar;
	float m_radsZoom;
	float m_xPan;
	float m_yPan;

	// Scene
	glm::vec3 m_sceneCenter;
	float m_sceneRadius;
	GLuint m_VAO, m_VBOVerts, m_VBONorms, m_VBOCols, m_VBOTexCoords, m_VBOTangents, m_VBOBitangents;
	QColor m_bkgColor;
	bool m_backFaceCulling;

	// Lights
	glm::vec4 m_lightPos;
	glm::vec3 m_lightCol;

	// Mouse
	int m_xClick;
	int m_yClick;
	float m_xRot;
	float m_yRot;
	InteractiveAction m_doingInteractive;

	// Shaders
    QOpenGLShaderProgram *m_program;
	GLuint m_transLoc, m_projLoc, m_viewLoc;
	GLuint m_vertexLoc, m_normalLoc, m_colorLoc, m_texCoordsLoc, m_tangentLoc, m_bitangentLoc;
	GLuint m_tex1LoadedLoc, m_tex2LoadedLoc;
	GLuint m_tex1TextureLoc, m_tex2TextureLoc;
	GLuint m_lightPosLoc, m_lightColLoc;

	// Textures
	QOpenGLTexture *m_tex1Tex, *m_tex2Tex;
	bool m_tex1Loaded, m_tex2Loaded;
	
};

#endif
