#include "MainWindow.h"
#include "OSGWidget.h"

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

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	QMenuBar* menuBar = this->menuBar();
	m_osgWidget = new OSGWidget(this);
	this->setCentralWidget(m_osgWidget);
	connect(ui->actionObj_File, &QAction::triggered, this, &MainWindow::onOpenObjFile);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onOpenObjFile()
{
	bool success = BusyTodoDialog::executeWithBusyDialog(
		this, 
		[]() {
			for (int i = 0; i < 100; ++i) {
				qDebug() << "Processing item" << i;
				QThread::msleep(50);
			}
		},"正在处理数据...", true,10000
	);
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), QString(), tr("OBJ Files (*.obj)"));
	if (!filePath.isEmpty())
	{
		readObjModel(filePath.toStdString());
	}
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

