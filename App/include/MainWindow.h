#ifndef YMAINWINDOW_H
#define YMAINWINDOW_H

#include <QMainWindow>
namespace Ui
{
	class MainWindow;
}
class MainWindow : public QMainWindow
{
public:
	MainWindow();
public:
	Ui::MainWindow *m_ui;
};

#endif