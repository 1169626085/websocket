#include "mainwindow.h"
#include "loginlog.h"
#include "registerlog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginLog(new Loginlog(this))
    , registerLog(new Registerlog(this))
{
    ui->setupUi(this);

    ui->authStackedWidget->addWidget(loginLog);
    ui->authStackedWidget->addWidget(registerLog);
    ui->authStackedWidget->setCurrentWidget(loginLog);

    connect(loginLog, &Loginlog::showRegisterRequested, this, [this]() {
        ui->authStackedWidget->setCurrentWidget(registerLog);
        setWindowTitle("注册");
    });

    connect(registerLog, &Registerlog::backToLoginRequested, this, [this]() {
        ui->authStackedWidget->setCurrentWidget(loginLog);
        setWindowTitle("登入");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
