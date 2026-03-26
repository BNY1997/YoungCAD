#ifndef MainWindow_h__
#define MainWindow_h__

#include <QMainWindow>
#include <QString>
#include <QResizeEvent>
#include "OSGWidget.h"
#include "DataExchange.h"
#include "ShapePool.h"
#include "ProjectTreeDock.h"

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
	void onTreeModelSelected(int modelId);
	void onViewportModelPicked(int modelId);
	void onDeleteModelRequested(int modelId);
	void registerModel(int modelId, const QString& displayName, bool hasOccShape);
	void tagNodeWithModelId(int modelId);
	void createViewControlPanel();
	void positionViewControlPanel();
	void resizeEvent(QResizeEvent* event) override;
private:
	OSGWidget* m_osgWidget;
	ProjectTreeDock* m_projectTreeDock;
	QWidget* m_viewControlPanel{ nullptr };
	DataExchangeOptions m_dataExchangeOptions;
	ShapePool m_shapePool;
	Ui::MainWindow* ui;
};

#endif
