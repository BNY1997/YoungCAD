//#include <QApplication>
#include <osgViewer/Viewer>
int main(int argc, char* argv[])
{
	//QApplication app(argc, argv);
  auto viewer = new osgViewer::Viewer;
  viewer->run();
	//app.exec();
}