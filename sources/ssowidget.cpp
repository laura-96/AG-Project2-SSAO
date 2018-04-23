#include "ssowidget.h"
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QColorDialog>
#include <QMessageBox>
#include <QPainter>
#include <math.h>

#include <iostream>
#include <random>

SSOWidget::SSOWidget(QString modelFilename, bool showFps, QWidget *parent) : QOpenGLWidget(parent)
{
	// To receive key events
	setFocusPolicy(Qt::StrongFocus);

	this->installEventFilter(this);

	// Attributes initialization
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
	m_camPos = glm::vec3(0.0f, 0.0f, -50.0f);

	// Scene
	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	m_sceneRadius = 50.0f;
	m_bkgColor = Qt::black;
	m_backFaceCulling = true;

	// Model
	m_modelLoaded = false;
	m_modelCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	m_modelRadius = 0.0f;
	m_modelFilename = modelFilename;

	// Mouse
	m_xRot = 0.0f;
	m_yRot = 0.0f;
	m_xRotCam = 0.0f;
	m_yRotCam = 0.0f;
	m_xClick = 0;
	m_yClick = 0;
	m_doingInteractive = NONE;

	// FPS
	m_frameCount = 0;
	m_fps = 0.0f;
	m_showFps = showFps;

	// Shaders
	m_program = nullptr;

}

SSOWidget::~SSOWidget()
{
	cleanup();
}

QSize SSOWidget::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize SSOWidget::sizeHint() const
{
	return QSize(m_width, m_height);
}

void SSOWidget::cleanup()
{
	if (m_modelLoaded)
		cleanBuffersModel();

	if (m_program == nullptr)
		return;

	makeCurrent();

	delete m_program;
	m_program = 0;

	doneCurrent();
}

void SSOWidget::initializeGL()
{
	// In this example the widget's corresponding top-level window can change
	// several times during the widget's lifetime. Whenever this happens, the
	// QOpenGLWidget's associated context is destroyed and a new one is created.
	// Therefore we have to be prepared to clean up the resources on the
	// aboutToBeDestroyed() signal, instead of the destructor. The emission of
	// the signal will be followed by an invocation of initializeGL() where we
	// can recreate all resources.
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &SSOWidget::cleanup);
	initializeOpenGLFunctions();
	loadShaders();
	
	createSSAOKernels();
	createGBuffers();
	createQuadBuffers();

	createBuffersModel();
	computeBBoxModel();
	computeCenterRadiusScene();
	initCameraParams();
	projectionTransform();
	viewTransform();
	setLighting();
}

void SSOWidget::paintGL()
{
	// FPS computation
	computeFps();

	// Paint the scene
	glClearColor(m_bkgColor.red() / 255.0f, m_bkgColor.green() / 255.0f, m_bkgColor.blue() / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (m_backFaceCulling)
		glEnable(GL_CULL_FACE);

	GeometryPass();
	GenSSAOTexture();

	// Show FPS if they are enabled 
	if (m_showFps)
		showFps();
}

void SSOWidget::resizeGL(int w, int h)
{
	m_width = w;
	m_height = h;

	glViewport(0, 0, m_width, m_height);
	m_ar = (float)m_width / (float)m_height;

	// We do this if we want to preserve the initial fov when resizing
	if (m_ar < 1.0f) {
		m_fov = 2.0f*atan(tan(m_fovIni / 2.0f) / m_ar) + m_radsZoom;
	}
	else {
		m_fov = m_fovIni + m_radsZoom;
	}

	// After modifying the parameters, we update the camera projection
	projectionTransform();
}

void SSOWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_B:
		// Change the background color
		std::cout << "-- AGEn message --: Change background color" << std::endl;
		changeBackgroundColor();
		break;
	case Qt::Key_C:
		// Set the camera at the center of the scene


		// TO DO: When pressing the C key, the camera must be placed at the center of the scene automatically


		break;
	case Qt::Key_F:
		// Enable/Disable frames per second
		m_showFps = !m_showFps;
		repaint();
		break;
	case Qt::Key_H:
		// Show the help message
		std::cout << "-- AGEn message --: Help" << std::endl;
		std::cout << std::endl;
		std::cout << "Keys used in the application:" << std::endl;
		std::cout << std::endl;
		std::cout << "-B:  change background color" << std::endl;
		std::cout << "-C:  set the camera at the center of the scene" << std::endl;
		std::cout << "-F:  show frames per second (fps)" << std::endl;
		std::cout << "-H:  show this help" << std::endl;
		std::cout << "-R:  reset the camera parameters" << std::endl;
		std::cout << "-F5: reload shaders" << std::endl;
		std::cout << std::endl;
		std::cout << "IMPORTANT: the focus must be set to the glwidget to work" << std::endl;
		std::cout << std::endl;
		break;
	case Qt::Key_R:
		// Reset the camera and scene parameters
		std::cout << "-- AGEn message --: Reset camera" << std::endl;
		resetCamera();
		break;
	case Qt::Key_F5:
		// Reload shaders
		std::cout << "-- AGEn message --: Reload shaders" << std::endl;
		reloadShaders();
		break;
	default:
		event->ignore();
		break;
	}
}

void SSOWidget::mousePressEvent(QMouseEvent *event)
{
	m_xClick = event->x();
	m_yClick = event->y();

	if (event->buttons() & Qt::LeftButton) {
		m_doingInteractive = ROTATE;
	}
	else if (event->buttons() & Qt::RightButton) {
		m_doingInteractive = PAN;
	}
}

void SSOWidget::mouseMoveEvent(QMouseEvent *event)
{
	makeCurrent();

	if (m_doingInteractive == ROTATE)
	{
		m_yRot += (event->x() - m_xClick) * PI / 180.0f;
		m_xRot += (event->y() - m_yClick) * PI / 180.0f;

	}
	else if (m_doingInteractive == PAN) {
		m_xPan += (event->x() - m_xClick)*m_sceneRadius * 0.005f;
		m_yPan += (event->y() - m_yClick)*m_sceneRadius * 0.005f;
		viewTransform();
	}

	m_xClick = event->x();
	m_yClick = event->y();
	update();
}

void SSOWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_doingInteractive = NONE;
	event->ignore();
}

void SSOWidget::wheelEvent(QWheelEvent* event)
{
	// In many mouses, each step of the wheel is considered 15 degs.
	int numDegrees = event->delta() / 8;

	// Number of steps of the wheel
	//int numSteps = numDegrees / 15;

	// For each step, we zoom in or out 7.5 degrees
	float numRads = DEG2RAD(numDegrees / 2.0f);

	float maxFov = DEG2RAD(175.0f);
	float minFov = DEG2RAD(15.0f);

	if (m_fov >= minFov && m_fov <= maxFov) {

		makeCurrent();

		float fovBeforeZoom = m_fov;

		float newFov = m_fov + numRads / 2.0f;
		newFov = MIN(newFov, maxFov);
		m_fov = MAX(newFov, minFov);

		// Total change of the fov to resize well the screen
		m_radsZoom += m_fov - fovBeforeZoom;

		// After modifying the parameters, we update the camera projection
		projectionTransform();
		update();
	}

	event->accept();
}

void SSOWidget::loadShaders()
{
	loadGShader();
	loadSSAOShader();

}

void SSOWidget::reloadShaders()
{

	// TO DO: Insert your code here to reload the shaders and update the view

}

void SSOWidget::loadGShader()
{
	// Declaration of the shaders
	QOpenGLShader vs(QOpenGLShader::Vertex, this);
	QOpenGLShader fs(QOpenGLShader::Fragment, this);

	// Load and compile the shaders
	vs.compileSourceFile("./shaders/gbuffer.vert");
	fs.compileSourceFile("./shaders/gbuffer.frag");

	// Create the program
	gPass_program = new QOpenGLShaderProgram;

	// Add the shaders
	gPass_program->addShader(&fs);
	gPass_program->addShader(&vs);

	// Link the program
	gPass_program->link();

	// Bind the program (we are gonna use this program)
	gPass_program->bind();

	// Get the attribs locations of the vertex shader
	gp_aPos = glGetAttribLocation(gPass_program->programId(), "aPos");
	gp_aNormal = glGetAttribLocation(gPass_program->programId(), "aNormal");
	gp_aTexCoords = glGetAttribLocation(gPass_program->programId(), "aTexCoords");
	gp_model = glGetAttribLocation(gPass_program->programId(), "model");
	gp_view = glGetAttribLocation(gPass_program->programId(), "view");
	gp_projection = glGetAttribLocation(gPass_program->programId(), "projection");

	// Get the uniforms locations of the fragment shader
	//gp_texcoords = glGetAttribLocation(gPass_program->programId(), "TexCoords");
	//gp_fragPos = glGetAttribLocation(gPass_program->programId(), "FragPos");
	//gp_normal = glGetAttribLocation(gPass_program->programId(), "Normal");
}

void SSOWidget::loadSSAOShader()
{
	// Declaration of the shaders
	QOpenGLShader vs(QOpenGLShader::Vertex, this);
	QOpenGLShader fs(QOpenGLShader::Fragment, this);

	// Load and compile the shaders
	vs.compileSourceFile("./shaders/ssao.vert");
	fs.compileSourceFile("./shaders/ssao.frag");

	// Create the program
	ssao_program = new QOpenGLShaderProgram;

	// Add the shaders
	ssao_program->addShader(&fs);
	ssao_program->addShader(&vs);

	// Link the program
	ssao_program->link();

	// Bind the program (we are gonna use this program)
	ssao_program->bind();

	// Get the attribs locations of the vertex shader
	ssao_aPos = glGetAttribLocation(ssao_program->programId(), "aPos");
	ssao_aTexCoords = glGetAttribLocation(ssao_program->programId(), "aTexCoords");

	ssao_gPos = glGetAttribLocation(ssao_program->programId(), "gPosition");
	ssao_gNormal = glGetAttribLocation(ssao_program->programId(), "gNormal");
	ssao_texNoise = glGetAttribLocation(ssao_program->programId(), "texNoise");
	ssao_samples = glGetAttribLocation(ssao_program->programId(), "samples");
	ssao_screenWidth = glGetAttribLocation(ssao_program->programId(), "screenWidth");
	ssao_screenHeight = glGetAttribLocation(ssao_program->programId(), "screenHeight");
	ssao_tileSize = glGetAttribLocation(ssao_program->programId(), "tileSize");
	ssao_projection = glGetAttribLocation(ssao_program->programId(), "projection");
}

void SSOWidget::initCameraParams()
{
	m_camPos = glm::vec3(0.0f, 0.0f, -2.0f * m_sceneRadius);
	m_zNear = m_sceneRadius;
	m_zFar = 3.0f * m_sceneRadius;
}

void SSOWidget::projectionTransform()
{
	// Set the camera type
	glm::mat4 proj(1.0f);

	proj = glm::perspective(m_fov, m_ar, m_zNear, m_zFar);

	// Send the matrix to the shader
	glUniformMatrix4fv(gp_projection, 1, GL_FALSE, &proj[0][0]);

}

void SSOWidget::resetCamera()
{
	makeCurrent();
	m_radsZoom = 0.0f;
	m_xPan = 0.0f;
	m_yPan = 0.0f;
	m_camPos = glm::vec3(0.0f, 0.0f, -2.0f * m_sceneRadius);
	m_zNear = m_sceneRadius;
	m_zFar = 3.0f * m_sceneRadius;

	if (m_ar < 1.0f) {
		m_fov = 2.0f*atan(tan(m_fovIni / 2.0f) / m_ar) + m_radsZoom;
	}
	else {
		m_fov = m_fovIni + m_radsZoom;
	}

	m_xRot = 0.0f;
	m_yRot = 0.0f;
	m_xRotCam = 0.0f;
	m_yRotCam = 0.0f;

	projectionTransform();
	viewTransform();
	update();
}

void SSOWidget::viewTransform()
{
	glm::mat4 view(1.0f);

	view = glm::translate(view, m_sceneCenter + m_camPos);
	view = glm::translate(view, glm::vec3(m_xPan, -m_yPan, 0.0f));
	view = glm::translate(view, -m_sceneCenter);

	// Send the matrix to the shader
	glUniformMatrix4fv(gp_view, 1, GL_FALSE, &view[0][0]);
}

void SSOWidget::changeBackgroundColor() {

	m_bkgColor = QColorDialog::getColor();
	repaint();
}

void SSOWidget::computeCenterRadiusScene()
{
	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);

	// In this case, we just load one model
	m_sceneRadius = m_modelRadius;
}

void SSOWidget::createBuffersModel()
{
	// Load the OBJ model - BEFORE creating the buffers!
	m_model.load(m_modelFilename.toStdString());

	// VAO creation
	glGenVertexArrays(1, &m_VAOModel);
	glBindVertexArray(m_VAOModel);

	// VBO Vertices
	glGenBuffers(1, &m_VBOModelVerts);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOModelVerts);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_model.faces().size() * 3 * 3, m_model.VBO_vertices(), GL_STATIC_DRAW);

	// Enable the attribute m_vertexLoc
	glVertexAttribPointer(gp_aPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(gp_aPos);

	// VBO Normals
	glGenBuffers(1, &m_VBOModelNorms);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOModelNorms);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_model.faces().size() * 3 * 3, m_model.VBO_normals(), GL_STATIC_DRAW);

	// Enable the attribute m_normalLoc
	glVertexAttribPointer(gp_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(gp_aNormal);

	// Instead of colors, we pass the materials 
	// VBO Ambient component
	glGenBuffers(1, &m_VBOModelMatAmb);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOModelMatAmb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_model.faces().size() * 3 * 3, m_model.VBO_matamb(), GL_STATIC_DRAW);

	// Enable the attribute m_matAmbLoc
	glVertexAttribPointer(m_matAmbLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_matAmbLoc);

	// VBO Diffuse component
	glGenBuffers(1, &m_VBOModelMatDiff);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOModelMatDiff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_model.faces().size() * 3 * 3, m_model.VBO_matdiff(), GL_STATIC_DRAW);

	// Enable the attribute m_matDiffLoc
	glVertexAttribPointer(m_matDiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_matDiffLoc);

	// VBO Specular component
	glGenBuffers(1, &m_VBOModelMatSpec);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOModelMatSpec);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_model.faces().size() * 3 * 3, m_model.VBO_matspec(), GL_STATIC_DRAW);

	// Enable the attribute m_matSpecLoc
	glVertexAttribPointer(m_matSpecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_matSpecLoc);

	// VBO Shininess component
	glGenBuffers(1, &m_VBOModelMatShin);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOModelMatShin);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_model.faces().size() * 3, m_model.VBO_matshin(), GL_STATIC_DRAW);

	// Enable the attribute m_matShinLoc
	glVertexAttribPointer(m_matShinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_matShinLoc);

	glBindVertexArray(0);

	// The model has been loaded
	m_modelLoaded = true;
}

void SSOWidget::cleanBuffersModel()
{
	makeCurrent();

	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &m_VBOModelVerts);
	glDeleteBuffers(1, &m_VBOModelNorms);
	glDeleteBuffers(1, &m_VBOModelMatAmb);
	glDeleteBuffers(1, &m_VBOModelMatDiff);
	glDeleteBuffers(1, &m_VBOModelMatSpec);
	glDeleteBuffers(1, &m_VBOModelMatShin);
	glDeleteVertexArrays(1, &m_VAOModel);

	m_modelLoaded = false;

	doneCurrent();
}

void SSOWidget::computeBBoxModel()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = maxX = m_model.vertices()[0];
	minY = maxY = m_model.vertices()[1];
	minZ = maxZ = m_model.vertices()[2];

	for (size_t i = 3; i < m_model.vertices().size(); i += 3)
	{
		if (m_model.vertices()[i + 0] < minX)
			minX = m_model.vertices()[i + 0];
		if (m_model.vertices()[i + 0] > maxX)
			maxX = m_model.vertices()[i + 0];
		if (m_model.vertices()[i + 1] < minY)
			minY = m_model.vertices()[i + 1];
		if (m_model.vertices()[i + 1] > maxY)
			maxY = m_model.vertices()[i + 1];
		if (m_model.vertices()[i + 2] < minZ)
			minZ = m_model.vertices()[i + 2];
		if (m_model.vertices()[i + 2] > maxZ)
			maxZ = m_model.vertices()[i + 2];
	}

	m_modelCenter = glm::vec3((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, (maxZ + minZ) / 2.0f);
	glm::vec3 radiusModel(maxX - m_modelCenter.x, maxY - m_modelCenter.y, maxZ - m_modelCenter.z);
	m_modelRadius = sqrt(radiusModel.x*radiusModel.x + radiusModel.y*radiusModel.y + radiusModel.z*radiusModel.z);
}

void SSOWidget::modelTransform()
{
	glm::mat4 geomTransform(1.0f);

	geomTransform = glm::translate(geomTransform, m_sceneCenter);
	geomTransform = glm::rotate(geomTransform, m_xRot, glm::vec3(1.0f, 0.0f, 0.0f));
	geomTransform = glm::rotate(geomTransform, m_yRot, glm::vec3(0.0f, 1.0f, 0.0f));
	geomTransform = glm::translate(geomTransform, -m_modelCenter);

	// Send the matrix to the shader
	glUniformMatrix4fv(gp_model, 1, GL_FALSE, &geomTransform[0][0]);
}

void SSOWidget::computeFps()
{

	// TO DO: Compute the FPS


}

void SSOWidget::showFps()
{
	// TO DO: Show the FPS
}

void SSOWidget::GeometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// projection
	// view

	//use shader geometrypass

	//model matrix

	//render scene

	gPass_program->bind();
	// Bind the VAO to draw the model
	glBindVertexArray(m_VAOModel);

	// Apply the geometric transforms to the model (position/orientation)
	modelTransform();

	// Draw the model
	glDrawArrays(GL_TRIANGLES, 0, m_model.faces().size() * 3);

	// Unbind the vertex array
	glBindVertexArray(0);

	gPass_program->release();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSOWidget::GenSSAOTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	ssao_program->bind();
	
	glUniform3fv(ssao_samples, 64, &ssaoKernel[0][0]);
	
	projectionTransform();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssao_gPos);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ssao_gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ssao_texNoise);

	// Render Quad
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	ssao_program->release();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSOWidget::createQuadBuffers()
{
	float quadVert[] =
	{
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVert), &quadVert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void SSOWidget::createGBuffers()
{
	glGenBuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	// gPosition
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// gNormal
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);


	// gAlbedoSpecular
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
	
	// Color attachments to use in this framebuffer
	attachments[0] = GL_COLOR_ATTACHMENT0;
	attachments[1] = GL_COLOR_ATTACHMENT1;
	attachments[2] = GL_COLOR_ATTACHMENT2;
	glDrawBuffers(3, attachments);

	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Noise Texture
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// SSAO FBO
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	// SSAO Color Buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
}

void SSOWidget::createSSAOKernels()
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / 64.0f;
		scale = 0.1f + (scale * scale) * (1.0f - 0.1f); //lerp
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	for (unsigned int i = 0; i < 16; ++i)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			0.0f);
		ssaoNoise.push_back(noise);
	}
}

void SSOWidget::setLighting()
{
	// Light source attached to the camera
	m_lightPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4fv(m_lightPosLoc, 1, &m_lightPos[0]);

	m_lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(m_lightColLoc, 1, &m_lightCol[0]);
}