#include "ssowidget.h"
#include "ssowindow.h"
#include "mainwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>


SSOWindow::SSOWindow(MainWindow* mw) :m_mainWindow(mw)
{
	m_ui.setupUi(this);

	// Insert the m_glWidget in the GUI
	m_glWidget = new SSOWidget("./models/Patricio.obj", false);
	layoutFrame = new QVBoxLayout(m_ui.qGLFrame);
	layoutFrame->setMargin(0);
	layoutFrame->addWidget(m_glWidget);
	m_glWidget->show();

	m_ui.selectCamera->addItem("Static", 0);
	m_ui.selectCamera->addItem("First Person", 1);

	connect(m_ui.qUndockButton, SIGNAL(clicked()), this, SLOT(dockUndock()));
	connect(m_ui.qLoadModelButton, SIGNAL(clicked()), this, SLOT(loadModel()));
	connect(m_ui.selectCamera, SIGNAL(activated(QString)), this, SLOT(loadCamera(QString)));
}

SSOWindow::~SSOWindow()
{
	if (m_glWidget != nullptr) {
		delete m_glWidget;
		m_glWidget = nullptr;
	}

}

void SSOWindow::dockUndock()
{
	if (parent()) {
		setParent(0);
		setAttribute(Qt::WA_DeleteOnClose);
		move(QApplication::desktop()->width() / 2 - width() / 2,
			QApplication::desktop()->height() / 2 - height() / 2);
		m_ui.qUndockButton->setText(tr("Dock"));
		show();
	}
	else {
		if (!m_mainWindow->centralWidget()) {
			if (m_mainWindow->isVisible()) {
				setAttribute(Qt::WA_DeleteOnClose, false);
				m_ui.qUndockButton->setText(tr("Undock"));
				m_mainWindow->setCentralWidget(this);
				show();
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

void SSOWindow::loadModel()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Load Model"),
		"./models/", tr("3D Models (*.obj)"));

	bool showFps = false;
	if (filename.size() != 0)
	{
		// We delete the glWidget and create another one to restart the GLContext
		// Otherwise, the painter does not work and the fps are not shown
		if (m_glWidget != nullptr) {
			delete m_glWidget;
			m_glWidget = nullptr;
		}

		m_glWidget = new SSOWidget(filename, showFps);
		layoutFrame->addWidget(m_glWidget);
		m_glWidget->show();
	}



}

void SSOWindow::loadCamera(QString cam_type) {

	QVariant type = m_ui.selectCamera->currentData();

	int camera_type = type.value<int>();
	switch (camera_type)
	{
		// Static camera's value is 0
	case 0:
		break;

		// First Person Camera's value is 1
	case 1:
		break;

	default:
		break;

	}

}
