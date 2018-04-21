#include "../headers/texturingglwidget.h"
#include "../headers/texturingwindow.h"
#include "../headers/mainwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

TexturingWindow::TexturingWindow(MainWindow* mw) :m_mainWindow(mw)
{
	m_ui.setupUi(this);

	// Insert the m_glWidget in the GUI
	m_glWidget = new TexturingGLWidget;
	QVBoxLayout* layoutFrame = new QVBoxLayout(m_ui.qGLFrame);
	layoutFrame->setMargin(0);
	layoutFrame->addWidget(m_glWidget);
	m_glWidget->show();

	initGUI();
	
	connect(m_ui.qUndockButton, SIGNAL(clicked()), this, SLOT(dockUndock()));
	connect(m_ui.qLoadTex1Button, SIGNAL(clicked()), this, SLOT(loadTex1()));
	connect(m_ui.qDeleteTex1Button, SIGNAL(clicked()), this, SLOT(deleteTex1()));
	connect(m_ui.qLoadTex2Button, SIGNAL(clicked()), this, SLOT(loadTex2()));
	connect(m_ui.qDeleteTex2Button, SIGNAL(clicked()), this, SLOT(deleteTex2()));
}

TexturingWindow::~TexturingWindow()
{
	if (m_glWidget != nullptr) {
		delete m_glWidget;
		m_glWidget = nullptr;
	}
}

void TexturingWindow::initGUI()
{
	QGraphicsScene *scene = new QGraphicsScene();
	scene->clear();
	scene->addText("(Empty)");

	m_ui.qTex1View->resetMatrix();
	m_ui.qTex1View->setScene(scene);
	m_ui.qTex1View->show();
	
	m_ui.qTex2View->resetMatrix();
	m_ui.qTex2View->setScene(scene);
	m_ui.qTex2View->show();
}

void TexturingWindow::dockUndock()
{
	if (parent()) {
		setParent(0);
		setAttribute(Qt::WA_DeleteOnClose);
		move(QApplication::desktop()->width() / 2 - width() / 2,
			QApplication::desktop()->height() / 2 - height() / 2);
		m_ui.qUndockButton->setText(tr("Dock"));
		show();

		if (m_filenameTex1.size() != 0)
			m_glWidget->loadTex1Texture(m_filenameTex1);

		if (m_filenameTex2.size() != 0)
			m_glWidget->loadTex2Texture(m_filenameTex2);
	}
	else {
		if (!m_mainWindow->centralWidget()) {
			if (m_mainWindow->isVisible()) {
				setAttribute(Qt::WA_DeleteOnClose, false);
				m_ui.qUndockButton->setText(tr("Undock"));
				m_mainWindow->setCentralWidget(this);
				show();

				if (m_filenameTex1.size() != 0)
					m_glWidget->loadTex1Texture(m_filenameTex1);

				if (m_filenameTex2.size() != 0)
					m_glWidget->loadTex2Texture(m_filenameTex2);
			}
			else {
				QMessageBox::information(0, tr("Cannot dock"), tr("Main window already closed"));
			}
		}
		else {
			QMessageBox::information(0, tr("Cannot dock"), tr("Main window already occupied"));
		}
	}
}

void TexturingWindow::loadTex1() 
{
	m_filenameTex1 = QFileDialog::getOpenFileName(this, tr("Load Texture"),
		"./textures/", tr("*.png;;*.jpg *.jpeg;;*.bmp;;*.gif;;*.pbm *.pgm *.ppm;;*.xbm *.xpm;;All files (*.*)"));

	if (m_filenameTex1.size() != 0) {
		m_glWidget->loadTex1Texture(m_filenameTex1);

		QGraphicsScene* texture = new QGraphicsScene();
		texture->addPixmap(m_filenameTex1);
		m_ui.qTex1View->setScene(texture);
		m_ui.qTex1View->fitInView(texture->itemsBoundingRect());
		m_ui.qTex1View->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		m_ui.qTex1View->show();
	}
}

void TexturingWindow::deleteTex1() 
{
	m_glWidget->deleteTex1Texture();
	
	QGraphicsScene *scene = new QGraphicsScene();
	scene->clear();
	scene->addText("(Empty)");

	m_ui.qTex1View->resetMatrix();
	m_ui.qTex1View->setScene(scene);
	m_ui.qTex1View->show();

	m_filenameTex1.clear();
}

void TexturingWindow::loadTex2() 
{
	m_filenameTex2 = QFileDialog::getOpenFileName(this, tr("Load Texture"),
		"./textures/", tr("*.png;;*.jpg *.jpeg;;*.bmp;;*.gif;;*.pbm *.pgm *.ppm;;*.xbm *.xpm;;All files (*.*)"));

	if (m_filenameTex2.size() != 0) {
		m_glWidget->loadTex2Texture(m_filenameTex2);

		QGraphicsScene* texture = new QGraphicsScene();
		texture->addPixmap(m_filenameTex2);
		m_ui.qTex2View->setScene(texture);
		m_ui.qTex2View->fitInView(texture->itemsBoundingRect());
		m_ui.qTex2View->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		m_ui.qTex2View->show();
	}
}

void TexturingWindow::deleteTex2()
{
	m_glWidget->deleteTex2Texture();

	QGraphicsScene *scene = new QGraphicsScene();
	scene->clear();
	scene->addText("(Empty)");

	m_ui.qTex2View->resetMatrix();
	m_ui.qTex2View->setScene(scene);
	m_ui.qTex2View->show();

	m_filenameTex2.clear();
}