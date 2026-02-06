#include <QApplication>
#include <osgViewer/Viewer>
#include "MainWindow.h"
int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	MainWindow w;
	w.show();
	app.exec();
}