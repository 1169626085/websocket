#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatdialog.h"
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
    void SlotSwitchChat();

private:
    Ui::MainWindow *ui;
    Loginlog *loginLog;
    Registerlog *registerLog;
    ChatDialog *chatLog;
};
#endif // MAINWINDOW_H
