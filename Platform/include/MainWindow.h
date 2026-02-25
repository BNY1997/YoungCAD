#ifndef MainWindow_h__
#define MainWindow_h__

#include <QMainWindow>
#include "OSGWidget.h"

namespace Ui
{
	class MainWindow;
}
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~MainWindow();
	
	void onOpenObjFile();
private:
	void readObjModel(const std::string& filePath);

private:
	OSGWidget* m_osgWidget;
	Ui::MainWindow* ui;
};

#endif
