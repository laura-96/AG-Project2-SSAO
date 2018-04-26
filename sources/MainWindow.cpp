#include "MainWindow.h"
#include "Window.h"
#include "basicwindow.h"
#include "phongwindow.h"
#include "texturingwindow.h"
#include "normalmapwindow.h"
#include "ssowindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>

MainWindow::MainWindow()
{
	//Menu bar
	QMenuBar *menuBar = new QMenuBar;
	QMenu *menuWindow = menuBar->addMenu(tr("&Window"));
	
	//Options
	QAction *addNew = new QAction(menuWindow);
	addNew->setText(tr("Add new logo"));
	menuWindow->addAction(addNew);
	
	QAction *addNewQuad = new QAction(menuWindow);
	addNewQuad->setText(tr("Add new quad"));
	menuWindow->addAction(addNewQuad);

	QAction *addNewModel = new QAction(menuWindow);
	addNewModel->setText(tr("Add new model"));
	menuWindow->addAction(addNewModel);


	QAction *addNewTexture = new QAction(menuWindow);
	addNewTexture->setText(tr("Add new texture"));
	menuWindow->addAction(addNewTexture);

	QAction *addNewNormalMap = new QAction(menuWindow);
	addNewNormalMap->setText(tr("Add new normal map"));
	menuWindow->addAction(addNewNormalMap);

	QAction *addNewSSO = new QAction(menuWindow);
	addNewSSO->setText(tr("Add new SSO"));
	menuWindow->addAction(addNewSSO);

	//Connects
	connect(addNew, &QAction::triggered, this, &MainWindow::onAddNewLogo);
	connect(addNewQuad, &QAction::triggered, this, &MainWindow::onAddNewQuad);
	connect(addNewModel, &QAction::triggered, this, &MainWindow::onAddNewModel);
	connect(addNewTexture, &QAction::triggered, this, &MainWindow::onAddNewTexture);
	connect(addNewNormalMap, &QAction::triggered, this, &MainWindow::onAddNewNormalMap);
	connect(addNewSSO, &QAction::triggered, this, &MainWindow::onAddNewSSO);

	setMenuBar(menuBar);

	//onAddNewQuad();
	onAddNewModel();
}

void MainWindow::onAddNewLogo()
{
	if (!centralWidget())
		setCentralWidget(new Window(this));
	else
		QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}


void MainWindow::onAddNewQuad()
{
	if (!centralWidget())
		setCentralWidget(new BasicWindow(this));
	else
		QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}

void MainWindow::onAddNewModel()
{
	if (!centralWidget())
		setCentralWidget(new PhongWindow(this));
	else
		QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}

void MainWindow::onAddNewTexture()
{
	if (!centralWidget())
		setCentralWidget(new TexturingWindow(this));
	else
		QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}

void MainWindow::onAddNewNormalMap()
{
	if (!centralWidget())
		setCentralWidget(new NormalMapWindow(this));
	else
		QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}

void MainWindow::onAddNewSSO()
{
	if (!centralWidget())
		setCentralWidget(new SSOWindow(this));
	else
		QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}
