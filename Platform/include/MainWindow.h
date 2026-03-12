#ifndef MainWindow_h__
#define MainWindow_h__

#include <QMainWindow>
#include "OSGWidget.h"
#include "DataExchange.h"
#include "ShapePool.h"

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
    void onOpenTxtFile();
private:
	void readObjModel(const std::string& filePath);
	void onCreateOCCCube();
	void onCreateOCCSphere();
	void onCreateOCCCone();
	DataExchangeOptions createDataExchangeOptions() const;
private:
	OSGWidget* m_osgWidget;
	DataExchangeOptions m_dataExchangeOptions;
	ShapePool m_shapePool;
	Ui::MainWindow* ui;
};

#endif
