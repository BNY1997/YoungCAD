#include "MainWindow.h"
#include "OSGWidget.h"

#include <QDebug>
#include <QMdiSubWindow>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
{
	QMenuBar* menuBar = this->menuBar();
	m_osgWidget = new OSGWidget(this);
	this->setCentralWidget(m_osgWidget);
}

MainWindow::~MainWindow()
{
}

