#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Loginlog;
class Registerlog;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Loginlog *loginLog;
    Registerlog *registerLog;
};
#endif // MAINWINDOW_H
