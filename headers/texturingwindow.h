#include <QWidget>
#include "ui_texturingwindow.h"

class MainWindow;
class TexturingGLWidget;

class TexturingWindow : public QWidget
{
	Q_OBJECT

public:
	TexturingWindow(MainWindow* mw);
	~TexturingWindow();

private slots:
	void dockUndock();
	void loadTex1();
	void deleteTex1();
	void loadTex2();
	void deleteTex2();
	
private:

	void initGUI();

	Ui::TexturingWindow m_ui;
	MainWindow* m_mainWindow;
	TexturingGLWidget* m_glWidget;
	
	QString m_filenameTex1;
	QString m_filenameTex2;

};
