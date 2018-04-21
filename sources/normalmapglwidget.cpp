#include "../headers/normalmapglwidget.h"
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QColorDialog>
#include <QPainter>
#include <math.h>

#include <iostream>


NormalMapGLWidget::NormalMapGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
	// To receive key events
	setFocusPolicy(Qt::StrongFocus);

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

	// Scene
	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	m_sceneRadius = 50.0f;
	m_bkgColor = Qt::black;
	m_backFaceCulling = false;

	// Mouse
	m_xRot = 0.0f;
	m_yRot = 0.0f;
	m_xClick = 0;
	m_yClick = 0;
	m_doingInteractive = NONE;
	
	// Shaders
	m_program = nullptr;

	// Color and Normal map
	m_tex1Tex = nullptr; 
	m_tex2Tex = nullptr;
	m_tex1Loaded = false; 
	m_tex2Loaded = false;
}

NormalMapGLWidget::~NormalMapGLWidget()
{
    cleanup();
}

QSize NormalMapGLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize NormalMapGLWidget::sizeHint() const
{
    return QSize(m_width, m_height);
}

void NormalMapGLWidget::cleanup()
{
	makeCurrent();
	
	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &m_VBOVerts);
	glDeleteBuffers(1, &m_VBONorms);
	glDeleteBuffers(1, &m_VBOCols);
	glDeleteBuffers(1, &m_VBOTexCoords);
	glDeleteBuffers(1, &m_VBOTangents);
	glDeleteBuffers(1, &m_VBOBitangents);
	glDeleteVertexArrays(1, &m_VAO);
	
	if (m_tex1Tex != nullptr)
	{
		m_tex1Tex->destroy();
		m_tex1Tex = nullptr;
	}

	if (m_tex2Tex != nullptr)
	{
		m_tex2Tex->destroy();
		m_tex2Tex = nullptr;
	}

	if (m_program == nullptr)
        return;
    
	delete m_program;
    m_program = 0;
    
	doneCurrent();
}

void NormalMapGLWidget::initializeGL()
{
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &NormalMapGLWidget::cleanup);
    initializeOpenGLFunctions();
 	loadShaders();
	createBuffersScene();
	computeBBoxScene();
	projectionTransform();
	viewTransform();
	setLighting();

}

void NormalMapGLWidget::paintGL()
{
	// Paint the scene
	glClearColor(m_bkgColor.red() / 255.0f, m_bkgColor.green() / 255.0f, m_bkgColor.blue() / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	if (m_backFaceCulling)
		glEnable(GL_CULL_FACE);

	// Bind the VAO to draw the scene
	glBindVertexArray(m_VAO);

	// Apply the geometric transforms to the scene (position/orientation)
	sceneTransform();

	// Draw the scene
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Unbind the vertex array
	glBindVertexArray(0);

}

void NormalMapGLWidget::resizeGL(int w, int h)
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

void NormalMapGLWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
		case Qt::Key_B:
			// Change the background color
			std::cout << "-- AGEn message --: Change background color" << std::endl;
			changeBackgroundColor();
			break;
		case Qt::Key_H:
			// Show the help message
			std::cout << "-- AGEn message --: Help" << std::endl;
			std::cout << std::endl;
			std::cout << "Keys used in the application:" << std::endl;
			std::cout << std::endl;
			std::cout << "-B:  change background color" << std::endl;
			std::cout << "-H:  show this help" << std::endl;
			std::cout << "-R:  reset the camera parameters" << std::endl;
			std::cout << std::endl;
			std::cout << "IMPORTANT: the focus must be set to the glwidget to work" << std::endl;
			std::cout << std::endl;
			break;
		case Qt::Key_R:
			// Reset the camera and scene parameters
			std::cout << "-- AGEn message --: Reset camera" << std::endl;
			resetCamera();
			break;
		default:
			event->ignore();
			break;
	}
}

void NormalMapGLWidget::mousePressEvent(QMouseEvent *event)
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

void NormalMapGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	makeCurrent();

	if (m_doingInteractive == ROTATE)
	{
		m_yRot += (event->x() - m_xClick) * PI / 180.0f;
		m_xRot += (event->y() - m_yClick) * PI / 180.0f;
	}
	else if (m_doingInteractive == PAN) {
		m_xPan += (event->x() - m_xClick)*0.1f;
		m_yPan += (event->y() - m_yClick)*0.1f;
		viewTransform();
	}

	m_xClick = event->x();
	m_yClick = event->y();

	update();
}

void NormalMapGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_doingInteractive = NONE;
	event->ignore();
}

void NormalMapGLWidget::wheelEvent(QWheelEvent* event)
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
		projectionTransform();
		update();
	}

	event->accept();
}

void NormalMapGLWidget::loadShaders()
{
	// Declaration of the shaders
	QOpenGLShader vs(QOpenGLShader::Vertex, this);
	QOpenGLShader fs(QOpenGLShader::Fragment, this);

	// Load and compile the shaders
	vs.compileSourceFile("./shaders/phongnormal.vert");
	fs.compileSourceFile("./shaders/phongnormal.frag");

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
	m_colorLoc = glGetAttribLocation(m_program->programId(), "color");
	m_texCoordsLoc = glGetAttribLocation(m_program->programId(), "texCoords");

	m_tangentLoc = glGetAttribLocation(m_program->programId(), "tangent");
	m_bitangentLoc = glGetAttribLocation(m_program->programId(), "bitangent");

	// Get the uniforms locations of the vertex shader
	m_transLoc = glGetUniformLocation(m_program->programId(), "sceneTransform");
	m_projLoc = glGetUniformLocation(m_program->programId(), "projTransform");
	m_viewLoc = glGetUniformLocation(m_program->programId(), "viewTransform");

	// Get the uniforms locations of the fragmenr shader
	m_tex1LoadedLoc = glGetUniformLocation(m_program->programId(), "tex1Loaded");
	m_tex1TextureLoc = glGetUniformLocation(m_program->programId(), "colorTexture");
	m_tex2LoadedLoc = glGetUniformLocation(m_program->programId(), "tex2Loaded");
	m_tex2TextureLoc = glGetUniformLocation(m_program->programId(), "normalTexture");
	m_lightPosLoc = glGetUniformLocation(m_program->programId(), "lightPos");
	m_lightColLoc = glGetUniformLocation(m_program->programId(), "lightCol");
}

void NormalMapGLWidget::projectionTransform()
{
	// Set the camera type
	glm::mat4 proj(1.0f);
	m_zNear = m_sceneRadius;
	m_zFar = 3.0f * m_sceneRadius;

	proj = glm::perspective(m_fov, m_ar, m_zNear, m_zFar);

	// Send the matrix to the shader
	glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, &proj[0][0]);

}

void NormalMapGLWidget::resetCamera()
{
	makeCurrent();
	m_radsZoom = 0.0f;
	m_xPan = 0.0f;
	m_yPan = 0.0f;

	if (m_ar < 1.0f) {
		m_fov = 2.0f*atan(tan(m_fovIni / 2.0f) / m_ar) + m_radsZoom;
	}
	else {
		m_fov = m_fovIni + m_radsZoom;
	}

	m_xRot = 0.0f;
	m_yRot = 0.0f;

	projectionTransform();
	viewTransform();
	update();
}

void NormalMapGLWidget::viewTransform()
{
	// Set the camera position
	glm::mat4 view(1.0f);

	view = glm::translate(view, m_sceneCenter + glm::vec3(0.0f, 0.0f, -2.0f * m_sceneRadius));
	view = glm::translate(view, glm::vec3(m_xPan, -m_yPan, 0.0f));
	view = glm::translate(view, -m_sceneCenter);
	
	/*else {
		glm::vec3 obs(0.0f, 0.0f, -2.0f * m_sceneRadius);
		glm::vec3 vrp = m_sceneCenter;
		glm::vec3 vup(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(obs, vrp, vup);
	}*/
	
	// Send the matrix to the shader
	glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, &view[0][0]);
}

void NormalMapGLWidget::changeBackgroundColor() {
	
	m_bkgColor = QColorDialog::getColor();
	repaint();
}

void NormalMapGLWidget::createBuffersScene()
{
	// Simple geometric scene
	// VBO vertices positions
	glm::vec3 verts[6] = {
		glm::vec3(-10.0f, -10.0f, 0.0f),
		glm::vec3(-10.0f, 10.0f, 0.0f),
		glm::vec3(10.0f, -10.0f, 0.0f),
		glm::vec3(10.0f, -10.0f, 0.0f),
		glm::vec3(-10.0f, 10.0f, 0.0f),
		glm::vec3(10.0f, 10.0f, 0.0f)
	};

	// VBO normals
	glm::vec3 normVerts[6] = {
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	};

	// VBO colors
	glm::vec4 colorsVerts[6] = {
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
	};

	glm::vec2 texCoordsVerts[6] = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 1.0f)
	};

	glm::vec3 tangentVerts[6];
	glm::vec3 bitangentVerts[6];

	for (int i = 0; i < 6; i += 3)
	{
		glm::vec3 deltaPos1 = verts[i + 1] - verts[i];
		glm::vec3 deltaPos2 = verts[i + 2] - verts[i];

		glm::vec2 deltaUV1 = texCoordsVerts[i + 1] - texCoordsVerts[i];
		glm::vec2 deltaUV2 = texCoordsVerts[i + 2] - texCoordsVerts[i];

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
		
		tangentVerts[i] = glm::normalize(tangent);
		tangentVerts[i + 1] = glm::normalize(tangent);
		tangentVerts[i + 2] = glm::normalize(tangent);

		bitangentVerts[i] = glm::normalize(bitangent);
		bitangentVerts[i + 1] = glm::normalize(bitangent);
		bitangentVerts[i + 2] = glm::normalize(bitangent);
	}

	// VAO creation
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// VBO Vertices
	glGenBuffers(1, &m_VBOVerts);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOVerts);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	// Enable the attribute m_vertexLoc
	glVertexAttribPointer(m_vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_vertexLoc);

	// VBO Normals
	glGenBuffers(1, &m_VBONorms);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBONorms);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normVerts), normVerts, GL_STATIC_DRAW);

	// Enable the attribute m_normalLoc
	glVertexAttribPointer(m_normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_normalLoc);

	// VBO Colors
	glGenBuffers(1, &m_VBOCols);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOCols);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorsVerts), colorsVerts, GL_STATIC_DRAW);

	// Enable the attribute m_colorLoc
	glVertexAttribPointer(m_colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_colorLoc);

	// VBO Tex Coords
	glGenBuffers(1, &m_VBOTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOTexCoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordsVerts), texCoordsVerts, GL_STATIC_DRAW);

	// Enable the attribute m_vertexLoc
	glVertexAttribPointer(m_texCoordsLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_texCoordsLoc);

	// Enable tangents and bitangents for normal mapping
	glGenBuffers(1, &m_VBOTangents);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOTangents);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tangentVerts), tangentVerts, GL_STATIC_DRAW);

	glVertexAttribPointer(m_tangentLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_tangentLoc);

	glGenBuffers(1, &m_VBOBitangents);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOBitangents);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bitangentVerts), bitangentVerts, GL_STATIC_DRAW);

	glVertexAttribPointer(m_bitangentLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(m_bitangentLoc);

	glBindVertexArray(0);
}

void NormalMapGLWidget::computeBBoxScene()
{
	// Right now we have just a quad of 20x20x0
	m_sceneRadius = sqrt(20 * 20 + 20 * 20)/2.0f;
	m_sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
}

void NormalMapGLWidget::sceneTransform()
{
	glm::mat4 geomTransform(1.0f);

	geomTransform = glm::translate(geomTransform, m_sceneCenter);
	geomTransform = glm::rotate(geomTransform, m_xRot, glm::vec3(-1.0f, 0.0f, 0.0f));
	geomTransform = glm::rotate(geomTransform, m_yRot, glm::vec3(0.0f, 1.0f, 0.0f));
	geomTransform = glm::translate(geomTransform, -m_sceneCenter);

	// Send the matrix to the shader
	glUniformMatrix4fv(m_transLoc, 1, GL_FALSE, &geomTransform[0][0]);
}

void NormalMapGLWidget::loadTex1Texture(QString filename)
{
	makeCurrent();
	m_tex1Tex = new QOpenGLTexture(QImage(filename).mirrored());
	m_tex1Tex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	m_tex1Tex->setMagnificationFilter(QOpenGLTexture::Linear);

	m_tex1Tex->bind(10);
	glUniform1i(m_tex1TextureLoc, 10);
	
	m_tex1Loaded = true;
	glUniform1i(m_tex1LoadedLoc, 1);
	
	update();
}

void NormalMapGLWidget::deleteTex1Texture()
{
	if (m_tex1Tex != nullptr)
	{
		makeCurrent();
		m_tex1Tex->release(10);
		m_tex1Tex->destroy();
		m_tex1Tex = nullptr;
	
		m_tex1Loaded = false;
		glUniform1i(m_tex1LoadedLoc, 0);
		update();
	}
}

void NormalMapGLWidget::loadTex2Texture(QString filename)
{
	makeCurrent();
	m_tex2Tex = new QOpenGLTexture(QImage(filename).mirrored());
	m_tex2Tex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
	m_tex2Tex->setMagnificationFilter(QOpenGLTexture::Linear);

	m_tex2Tex->bind(11);
	glUniform1i(m_tex2TextureLoc, 11);

	m_tex2Loaded = true;
	glUniform1i(m_tex2LoadedLoc, 1);
	update();
}

void NormalMapGLWidget::deleteTex2Texture()
{
	if (m_tex2Tex != nullptr)
	{
		makeCurrent();

		m_tex2Tex->release(11);
		m_tex2Tex->destroy();
		m_tex2Tex = nullptr;
		
		m_tex2Loaded = false;
		glUniform1i(m_tex2LoadedLoc, 0);
		update();
	}
}

void NormalMapGLWidget::setLighting()
{
	// Light source attached to the camera
	m_lightPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4fv(m_lightPosLoc, 1, &m_lightPos[0]);

	m_lightCol = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(m_lightColLoc, 1, &m_lightCol[0]);
}
