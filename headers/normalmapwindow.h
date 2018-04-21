#include <QWidget>
#include "ui_normalmapwindow.h"

class MainWindow;
class NormalMapGLWidget;

class NormalMapWindow : public QWidget
{
	Q_OBJECT

public:
	NormalMapWindow(MainWindow* mw);
	~NormalMapWindow();

private slots:
	void dockUndock();
	void loadTex1();
	void deleteTex1();
	void loadTex2();
	void deleteTex2();
	
private:

	void initGUI();
	
	
	Ui::NormalMapWindow m_ui;
	MainWindow* m_mainWindow;
	NormalMapGLWidget* m_glWidget;
	
	QString m_filenameTex1;
	QString m_filenameTex2;

};
