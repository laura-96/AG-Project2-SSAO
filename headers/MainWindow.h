#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

	private slots:
	void onAddNewLogo();
	void onAddNewQuad();
	void onAddNewModel();
	void onAddNewTexture();
	void onAddNewNormalMap();
};

#endif