#include "phongglwidget.h"
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QColorDialog>
#include <QMessageBox>
#include <QPainter>
#include <math.h>

#include <iostream>

PhongGLWidget::PhongGLWidget(QString modelFilename, bool showFps, QWidget *parent) : QOpenGLWidget(parent)
{
	// To receive key events
	setFocusPolicy(Qt::StrongFocus);

	this->installEventFilter(this);

	// Attributes initialization
	// Screen
	m_width = 500;
	m_height = 500;

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

PhongGLWidget::~PhongGLWidget()
{
    cleanup();
}

QSize PhongGLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize PhongGLWidget::sizeHint() const
{
    return QSize(m_width, m_height);
}

void PhongGLWidget::sceneCameraType(int type)
{
	makeCurrent();
	if (cam)
		cam->SetType(type);

	if (cam->GetType() == 0)
	{
		setMouseTracking(false);
	}
	else
	{
		setMouseTracking(true);
	}
	update();
}

void PhongGLWidget::cleanup()
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

void PhongGLWidget::initializeGL()
{
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &PhongGLWidget::cleanup);
    initializeOpenGLFunctions();
 	loadShaders();
	createBuffersModel();
	computeBBoxModel ();
	computeCenterRadiusScene();
	
	cam = new Camera(m_width, m_height, glm::vec3(0.0f, 0.0f, -2.0f * m_sceneRadius), m_sceneRadius, 0);
	
	if (cam->GetType() == 0)
	{
		setMouseTracking(false);
	}
	else
	{
		setMouseTracking(true);
	}


	projectionTransform();
	viewTransform();
	setLighting();
}

void PhongGLWidget::paintGL()
{
	// FPS computation
	computeFps();

	// Paint the scene
	glClearColor(m_bkgColor.red() / 255.0f, m_bkgColor.green() / 255.0f, m_bkgColor.blue() / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (m_backFaceCulling)
		glEnable(GL_CULL_FACE);

	// Bind the VAO to draw the model
	glBindVertexArray(m_VAOModel);

	// Apply the geometric transforms to the model (position/orientation)
	modelTransform();

	// Draw the model
	glDrawArrays(GL_TRIANGLES, 0, m_model.faces().size() * 3);

	// Unbind the vertex array
	glBindVertexArray(0);

	// Show FPS if they are enabled 
	if (m_showFps)
		showFps();
}

void PhongGLWidget::resizeGL(int w, int h)
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
	cam->ResizeCamera(m_fov, m_width, m_height);
	projectionTransform();
}

void PhongGLWidget::keyPressEvent(QKeyEvent *event)
{
	makeCurrent();

	cam->Update();
	switch (event->key()) {


		case Qt::Key_W:
			std::cout << "-- AGEn message --: Going forward" << std::endl;
			
			cam->Move((MovementType)0);
			viewTransform();
			
			break;

		case Qt::Key_S:
			std::cout << "-- AGEn message --: Going backwards" << std::endl;
			
			cam->Move((MovementType)1);
			viewTransform();
			
			break;

		case Qt::Key_D:
			std::cout << "-- AGEn message --: Going right" << std::endl;
			
			cam->Move((MovementType)2);
			viewTransform();
			
			break;

		case Qt::Key_A:
			std::cout << "-- AGEn message --: Going left" << std::endl;
			
			cam->Move((MovementType)3);
			viewTransform();
			
			break;
		case Qt::Key_B:
			// Change the background color
			std::cout << "-- AGEn message --: Change background color" << std::endl;
			changeBackgroundColor();
			break;
		case Qt::Key_C:
			// Set the camera at the center of the scene
			
			cam->Center();
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
	update();
}

void PhongGLWidget::mousePressEvent(QMouseEvent *event)
{
	m_xClick = event->x();
	m_yClick = event->y();

	if (event->buttons() & Qt::LeftButton) {
		m_doingInteractive = ROTATE;
	}
	else if (event->buttons() & Qt::RightButton){
		m_doingInteractive = PAN;
	}
}

void PhongGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	makeCurrent();

	if (cam->GetType() == 1)
	{
		cam->Rotate(0.005 * (m_width/2 - (event->x() * PI / 180.0f)), 0.005 * (m_height / 2 - (event->y() * PI / 180.0f)));
		viewTransform();
	}
	
	else
	{
		if (m_doingInteractive == ROTATE)
		{
			m_yRot += (event->x() - m_xClick) * PI / 180.0f;
			m_xRot += (event->y() - m_yClick) * PI / 180.0f;
		}
		else if (m_doingInteractive == PAN) {
			cam->Pan((event->x() - m_xClick)*m_sceneRadius * 0.005f, (event->y() - m_yClick)*m_sceneRadius * 0.005f);
			viewTransform();
		}

		m_xClick = event->x();
		m_yClick = event->y();
	}

	
	update();
}

void PhongGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_doingInteractive = NONE;
	event->ignore();
}

void PhongGLWidget::wheelEvent(QWheelEvent* event)
{
	// In many mouses, each step of the wheel is considered 15 degs.
	int numDegrees = event->delta() / 8;
	
	// Number of steps of the wheel
	//int numSteps = numDegrees / 15;

	// For each step, we zoom in or out 7.5 degrees
	float numRads = DEG2RAD(numDegrees/2.0f);

	float maxFov = DEG2RAD(175.0f);
	float minFov = DEG2RAD(15.0f);

	if (m_fov >= minFov && m_fov <= maxFov) {
		
		makeCurrent();
		
		float fovBeforeZoom = m_fov;
		
		float newFov = m_fov + numRads/2.0f;
		newFov = MIN(newFov, maxFov);
		m_fov = MAX(newFov, minFov);

		// Total change of the fov to resize well the screen
		m_radsZoom += m_fov - fovBeforeZoom;

		// After modifying the parameters, we update the camera projection
		cam->ResizeCamera(m_fov, m_width, m_height);
		projectionTransform();
		update();
	}

	event->accept();
}

void PhongGLWidget::loadShaders()
{
	// Declaration of the shaders
	QOpenGLShader vs(QOpenGLShader::Vertex, this);
	QOpenGLShader fs(QOpenGLShader::Fragment, this);

	// Load and compile the shaders
	vs.compileSourceFile("./shaders/phong.vert");
	fs.compileSourceFile("./shaders/phong.frag");

	// Create the program
	m_program = new QOpenGLShaderProgram;

	// Add the shaders
	m_program->addShader(&fs);
	m_program->addShader(&vs);

	// Link the program
	m_program->link();

	// Bind the program (we are gonna use this program)
	m_program->bind();

	// Get the attribs locations of the vertex shader
	m_vertexLoc = glGetAttribLocation(m_program->programId(), "vertex");
	m_normalLoc = glGetAttribLocation(m_program->programId(), "normal");
	m_matAmbLoc = glGetAttribLocation(m_program->programId(), "matamb");
	m_matDiffLoc = glGetAttribLocation(m_program->programId(), "matdiff");
	m_matSpecLoc = glGetAttribLocation(m_program->programId(), "matspec");
	m_matShinLoc = glGetAttribLocation(m_program->programId(), "matshin");

	// Get the uniforms locations of the vertex shader
	m_transLoc = glGetUniformLocation(m_program->programId(), "sceneTransform");
	m_projLoc = glGetUniformLocation(m_program->programId(), "projTransform");
	m_viewLoc = glGetUniformLocation(m_program->programId(), "viewTransform");
	m_lightPosLoc = glGetUniformLocation(m_program->programId(), "lightPos");
	m_lightColLoc = glGetUniformLocation(m_program->programId(), "lightCol");
	
}

void PhongGLWidget::reloadShaders()
{
	// TO DO: Insert your code here to reload the shaders and update the view

}

void PhongGLWidget::projectionTransform()
{
	glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, &cam->GetProj()[0][0]);
}

void PhongGLWidget::resetCamera()
{
	makeCurrent();
	
	cam->Reset();

	projectionTransform();
	viewTransform();
	update();
}

void PhongGLWidget::viewTransform()
{

	// Send the matrix to the shader
	glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, &cam->GetView()[0][0]);
}

void PhongGLWidget::changeBackgroundColor() {
	
	m_bkgColor = QColorDialog::getColor();
	repaint();
}

void PhongGLWidget::computeCenterRadiusScene() 
{
	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	
	// In this case, we just load one model
	m_sceneRadius = m_modelRadius;
}

void PhongGLWidget::createBuffersModel()
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
	glVertexAttribPointer(m_vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_vertexLoc);

	// VBO Normals
	glGenBuffers(1, &m_VBOModelNorms);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOModelNorms);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*m_model.faces().size() * 3 * 3, m_model.VBO_normals(), GL_STATIC_DRAW);

	// Enable the attribute m_normalLoc
	glVertexAttribPointer(m_normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_normalLoc);

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

void PhongGLWidget::cleanBuffersModel()
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

void PhongGLWidget::computeBBoxModel()
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

void PhongGLWidget::modelTransform()
{
	glm::mat4 geomTransform(1.0f);

	geomTransform = glm::translate(geomTransform, m_sceneCenter);
	geomTransform = glm::rotate(geomTransform, m_xRot, glm::vec3(1.0f, 0.0f, 0.0f));
	geomTransform = glm::rotate(geomTransform, m_yRot, glm::vec3(0.0f, 1.0f, 0.0f));
	geomTransform = glm::translate(geomTransform, -m_modelCenter);

	// Send the matrix to the shader
	glUniformMatrix4fv(m_transLoc, 1, GL_FALSE, &geomTransform[0][0]);
}

void PhongGLWidget::computeFps() 
{
	
	// TO DO: Compute the FPS


}

void PhongGLWidget::showFps()
{
	// TO DO: Show the FPS
}

void PhongGLWidget::setLighting() 
{
	// Light source attached to the camera
	m_lightPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4fv(m_lightPosLoc, 1, &m_lightPos[0]);

	m_lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(m_lightColLoc, 1, &m_lightCol[0]);
}