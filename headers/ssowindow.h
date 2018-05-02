#include <QWidget>
#include "ui_phongwindow.h"

class MainWindow;
class SSOWidget;

class SSOWindow : public QWidget
{
	Q_OBJECT

public:
	SSOWindow(MainWindow* mw);
	~SSOWindow();

	private slots:
	void dockUndock();
	void loadModel();
	void loadCamera(QString cam_type);
	void activeSSAO(bool active);
	void changeSSAOIntensity(double value);

private:
	Ui::PhongWindow m_ui;
	MainWindow* m_mainWindow;
	SSOWidget* m_glWidget;

	QVBoxLayout* layoutFrame;
};