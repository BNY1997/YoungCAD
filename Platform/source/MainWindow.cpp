#include "MainWindow.h"
#include "OSGWidget.h"
#include <osg/Material>

#include <QDebug>
#include <QMdiSubWindow>
#include <QMenuBar>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_MainWindow.h"
#include "BusyTodoDialog.h"
#include <QThread>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
// move conversion implementation to DataExchange module
#include "DataExchange.h"
#include "DataIO.h"


MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), ui(new Ui::MainWindow)
{
	m_dataExchangeOptions.meshDeflection = 0.1;
	m_dataExchangeOptions.buildEdges = true;
	m_dataExchangeOptions.smoothNormals = true;
	m_dataExchangeOptions.edgeAngularDeflection = 0.1;
	m_dataExchangeOptions.edgeCurvatureDeflection = 0.1;

	ui->setupUi(this);
	QMenuBar* menuBar = this->menuBar();
	m_osgWidget = new OSGWidget(this);
	this->setCentralWidget(m_osgWidget);
	connect(ui->actionObj_File, &QAction::triggered, this, &MainWindow::onOpenObjFile);
	connect(ui->action_cube, &QAction::triggered, this, &MainWindow::onCreateOCCCube);
	connect(ui->action_cone, &QAction::triggered, this, &MainWindow::onCreateOCCCone);
	connect(ui->action_sphere, &QAction::triggered, this, &MainWindow::onCreateOCCSphere);

	// create action for TXT point cloud import and add to existing '打开' menu if available

	connect(ui->action_text, &QAction::triggered, this, &MainWindow::onOpenTxtFile);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onOpenObjFile()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), QString(), tr("OBJ Files (*.obj)"));
	if (!filePath.isEmpty())
	{
		bool success = BusyTodoDialog::executeWithBusyDialog(
			this,
			[&]() {
                readObjModel(filePath.toStdString());
			}, "正在处理数据..."
		);

	}
}

void MainWindow::onOpenTxtFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Points TXT"), QString(), tr("Text Files (*.txt *.csv)"));
    if (filePath.isEmpty())
        return;

    // Read and convert in background to avoid blocking UI
    bool success = BusyTodoDialog::executeWithBusyDialog(
        this,
        [&]() {
            TriangleMeshDataIO io(createDataExchangeOptions());
            osg::ref_ptr<osg::Node> node = io.readData(filePath.toStdString());
            if (node && m_osgWidget) {
                // forward to UI thread
                QMetaObject::invokeMethod(m_osgWidget, [this, node]() {
                    m_osgWidget->setSceneData(node);
                }, Qt::QueuedConnection);
            }
        }, "正在加载点云并构建三角网..."
    );
}

DataExchangeOptions MainWindow::createDataExchangeOptions() const
{
	return m_dataExchangeOptions;
}

void MainWindow::readObjModel(const std::string& filePath)
{
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filePath);
	if (!node)
	{
		QMessageBox::warning(this, tr("Error"), tr("Failed to read STL file: %1").arg(QString::fromStdString(filePath)));
		return;
	}
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}

void MainWindow::onCreateOCCCube()
{
	TopoDS_Shape shape = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape, createDataExchangeOptions());
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}

void MainWindow::onCreateOCCCone()
{
	// Radius1, Radius2, Height
	TopoDS_Shape shape = BRepPrimAPI_MakeCone(5.0, 0.0, 10.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape, createDataExchangeOptions());
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}

void MainWindow::onCreateOCCSphere()
{
	TopoDS_Shape shape = BRepPrimAPI_MakeSphere(5.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape, createDataExchangeOptions());
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}


