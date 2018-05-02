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
	m_radsZoom = 0.0f;
	m_xPan = 0.0f;
	m_yPan = 0.0f;

	cam_type = 0;
	m_xRotPoint = m_yRotPoint = 0;
	// Scene
	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	m_sceneRadius = 50.0f;
	m_bkgColor = Qt::green;
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

void SSOWidget::sceneCameraType(int type)
{
	makeCurrent();

	if (camera)
	{
		camera->SetType(type);

		cam_type = type;
	}
		
	update();
}

void SSOWidget::activateSSAO(bool active)
{
	usingSSAO = active;
	flag_ssao = true;
	repaint();
}

void SSOWidget::setSSAOIntensity(double value)
{
	flag_ssao = true;
	ssao_intensity = (float)value;
	repaint();
}

void SSOWidget::activateDrawOnlySSAO(bool active)
{
	drawSSAO = active;
	flag_ssao = true;
	repaint();
}

void SSOWidget::cleanup()
{
	if (m_modelLoaded)
		cleanBuffersModel();

	makeCurrent();

	/*if (light_program)
		delete light_program;
	if (gPass_program)
		delete gPass_program;

	if (g_fbo)
		delete g_fbo;*/

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


	createBuffersQuad();
	createGBuffers();

	gPass_program->bind();

	createBuffersModel();
	computeBBoxModel();
	computeCenterRadiusScene();
	initCamera();

	projectionTransform();
	viewTransform();
	setLighting();
}

void SSOWidget::paintGL()
{
	GeometryPass();
		
	LightPass();

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


	camera->ResizeCamera(m_fov, m_width, m_height);
	// After modifying the parameters, we update the camera projection
	projectionTransform();
}

void SSOWidget::keyPressEvent(QKeyEvent *event)
{
	makeCurrent();
	switch (event->key()) {
	case Qt::Key_W:
		std::cout << "-- AGEn message --: Going forward" << std::endl;

		camera->Move((MovementType)0);
		viewTransform();

		break;

	case Qt::Key_S:
		std::cout << "-- AGEn message --: Going backwards" << std::endl;

		camera->Move((MovementType)1);
		viewTransform();

		break;

	case Qt::Key_D:
		std::cout << "-- AGEn message --: Going right" << std::endl;

		camera->Move((MovementType)2);
		viewTransform();

		break;

	case Qt::Key_A:
		std::cout << "-- AGEn message --: Going left" << std::endl;

		camera->Move((MovementType)3);
		viewTransform();

		break;
	case Qt::Key_B:
		// Change the background color
		std::cout << "-- AGEn message --: Change background color" << std::endl;
		changeBackgroundColor();
		break;
	case Qt::Key_C:
		// Set the camera at the center of the scene
		camera->Center();

		viewTransform();
		projectionTransform();
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
	update();
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

	if (camera->GetType() == 1)
	{
		camera->Rotate((event->x() - m_width / 2 - m_xRotPoint) * PI / 180.0f, (event->y() - m_height / 2 - m_yRotPoint) * PI / 180.0f);

		viewTransform();

		m_xRotPoint = (event->x() - m_width / 2);
		m_yRotPoint = (event->y() - m_height / 2);
	}

	else
	{
		if (m_doingInteractive == ROTATE)
		{
			m_yRot += (event->x() - m_xClick) * PI / 180.0f;
			m_xRot += (event->y() - m_yClick) * PI / 180.0f;
		}
		else if (m_doingInteractive == PAN) {
			camera->Pan((event->x() - m_xClick)*m_sceneRadius * 0.005f, (event->y() - m_yClick)*m_sceneRadius * 0.005f);

			viewTransform();
		}

		m_xClick = event->x();
		m_yClick = event->y();
	}
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

		camera->ResizeCamera(m_fov, m_width, m_height);
		// After modifying the parameters, we update the camera projection
		projectionTransform();
		update();
	}

	event->accept();
}

void SSOWidget::loadShaders()
{
	loadGShader();
	loadLightShader();

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
	gp_model = glGetUniformLocation(gPass_program->programId(), "model");
	gp_view = glGetUniformLocation(gPass_program->programId(), "view");
	gp_projection = glGetUniformLocation(gPass_program->programId(), "projection");

	m_matAmbLoc = glGetAttribLocation(gPass_program->programId(), "matamb");
	m_matDiffLoc = glGetAttribLocation(gPass_program->programId(), "matdiff");
	m_matSpecLoc = glGetAttribLocation(gPass_program->programId(), "matspec");
	m_matShinLoc = glGetAttribLocation(gPass_program->programId(), "matshin");

	m_lightPosLoc = glGetUniformLocation(gPass_program->programId(), "lightPos");
	m_lightColLoc = glGetUniformLocation(gPass_program->programId(), "lightCol");
}

void SSOWidget::loadLightShader()
{
	QOpenGLShader vs2(QOpenGLShader::Vertex, this);
	QOpenGLShader fs2(QOpenGLShader::Fragment, this);

	// Load and compile the shaders
	vs2.compileSourceFile("./shaders/light.vert");
	fs2.compileSourceFile("./shaders/light.frag");

	// Create the program
	light_program = new QOpenGLShaderProgram;

	// Add the shaders
	light_program->addShader(&fs2);
	light_program->addShader(&vs2);

	// Link the program
	light_program->link();

	// Bind the program (we are gonna use this program)
	light_program->bind();

	// Get the attribs locations of the vertex shader
	light_vertex = glGetAttribLocation(light_program->programId(), "vertex");
	light_texcoords = glGetAttribLocation(light_program->programId(), "vertTexCoords");

	gPositionTex = glGetUniformLocation(light_program->programId(), "gPosition");
	gNormalTex = glGetUniformLocation(light_program->programId(), "gNormal");
	gAlbedo = glGetUniformLocation(light_program->programId(), "gAlbedoSpec");
	light_projection = glGetUniformLocation(light_program->programId(), "projection");

	useSSAO = glGetUniformLocation(light_program->programId(), "useSSAO");
	glUniform1i(useSSAO, 0.2);

	drawSSAOLoc = glGetUniformLocation(light_program->programId(), "drawSSAO");
	glUniform1i(drawSSAO, 0);

	ssaoIntensityLoc = glGetUniformLocation(light_program->programId(), "ssaoIntensity");
	glUniform1f(ssaoIntensityLoc, ssao_intensity);

	GLuint screen_width = glGetUniformLocation(light_program->programId(), "screenWidth");
	GLuint screenHeight = glGetUniformLocation(light_program->programId(), "screenHeight");
	GLuint tileSize = glGetUniformLocation(light_program->programId(), "tileSize");
	GLuint samples = glGetUniformLocation(light_program->programId(), "samples");
	texNoise = glGetUniformLocation(light_program->programId(), "texNoise");

	glUniform1f(screen_width, m_width);
	glUniform1f(screenHeight, m_height);
	glUniform1f(tileSize, 4.0);

	createSSAOKernels();

	// Noise Texture
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glUniform3fv(samples, 64, &ssaoKernel[0][0]);
}

void SSOWidget::initCamera()
{
	camera = new Camera(m_width, m_height, glm::vec3(0.0f, 0.0f, -2.0f * m_sceneRadius), m_sceneRadius, cam_type);
}

void SSOWidget::projectionTransform(bool useSSAO)
{
	// Send the matrix to the shader

	light_program->bind();
	glUniformMatrix4fv(light_projection, 1, GL_FALSE, &camera->GetProj()[0][0]);

	gPass_program->bind();
	glUniformMatrix4fv(gp_projection, 1, GL_FALSE, &camera->GetProj()[0][0]);
}

void SSOWidget::resetCamera()
{
	makeCurrent();
	camera->Reset();

	projectionTransform();
	viewTransform();
	update();
}

void SSOWidget::viewTransform()
{
	// Send the matrix to the shader

	light_program->bind();
	glUniformMatrix4fv(light_view, 1, GL_FALSE, &camera->GetView()[0][0]);

	gPass_program->bind();
	glUniformMatrix4fv(gp_view, 1, GL_FALSE, &camera->GetView()[0][0]);
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

void SSOWidget::GeometryPass()
{
	g_fbo->bind();
	GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, bufs);

	// Paint the scene
	glClearColor(m_bkgColor.red() / 255.0f, m_bkgColor.green() / 255.0f, m_bkgColor.blue() / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (m_backFaceCulling)
		glEnable(GL_CULL_FACE);

	gPass_program->bind();
	// Bind the VAO to draw the model
	glBindVertexArray(m_VAOModel);

	// Apply the geometric transforms to the model (position/orientation)
	modelTransform();

	// Draw the model
	glDrawArrays(GL_TRIANGLES, 0, m_model.faces().size() * 3);

	// Unbind the vertex array
	glBindVertexArray(0);
	g_fbo->bindDefault();
}

void SSOWidget::LightPass()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	light_program->bind();

	QVector<GLuint> texIds = g_fbo->textures();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texIds[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glUniform1i(gPositionTex, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texIds[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glUniform1i(gNormalTex, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texIds[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glUniform1i(gAlbedo, 3);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glUniform1i(texNoise, 8);

	if (flag_ssao)
	{	
		int ssaoVal = usingSSAO ? 1 : 0;
		int drawssaoVal = drawSSAO ? 1 : 0;
		glUniform1i(useSSAO, ssaoVal);
		glUniform1f(ssaoIntensityLoc, ssao_intensity);
		glUniform1i(drawSSAOLoc, drawssaoVal);
		flag_ssao = false;
	}
	
	// Bind the VAO to draw the model
	glBindVertexArray(quadVAO);

	// Draw the model
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SSOWidget::createBuffersQuad()
{
	// VAO creation
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	// VBO vertices positions
	glm::vec3 verts[6] = {
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, -1.0f, 0.0f),
		glm::vec3(-1.0f, 1.0f, 0.0f),
		glm::vec3(-1.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f)
	};

	glm::vec2 texCoords[6] = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f)
	};

	// VBO Vertices
	glGenBuffers(1, &quadVBOVert);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBOVert);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glVertexAttribPointer(light_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(light_vertex);

	// VBO Tex Coords
	glGenBuffers(1, &quadVBOTexCoord);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBOTexCoord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

	// Enable the attribute m_vertexLoc
	glVertexAttribPointer(light_texcoords, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(light_texcoords);

	glBindVertexArray(0);

}
void SSOWidget::createGBuffers()
{
	// Init fbo
	QOpenGLFramebufferObjectFormat format;
	format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
	g_fbo = new QOpenGLFramebufferObject(m_width, m_height, format);
	g_fbo->addColorAttachment(m_width, m_height);
	g_fbo->addColorAttachment(m_width, m_height);
	g_fbo->addColorAttachment(m_width, m_height);

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