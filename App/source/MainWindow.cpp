#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow() : m_ui(new Ui::MainWindow)
{
	m_ui->setupUi(this);
}