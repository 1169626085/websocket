#include "mainwindow.h"
#include "loginlog.h"
#include "registerlog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginLog(new Loginlog(this))
    , registerLog(new Registerlog(this))
    , chatLog(new ChatDialog(this))
{
    ui->setupUi(this);

    ui->authStackedWidget->addWidget(loginLog);
    ui->authStackedWidget->addWidget(registerLog);
    ui->authStackedWidget->addWidget(chatLog);
    ui->authStackedWidget->setCurrentWidget(loginLog);

    connect(loginLog, &Loginlog::showRegisterRequested, this, [this]() {
        ui->authStackedWidget->setCurrentWidget(registerLog);
        setWindowTitle("注册");
    });

    connect(registerLog, &Registerlog::backToLoginRequested, this, [this]() {
        ui->authStackedWidget->setCurrentWidget(loginLog);
        setWindowTitle("登入");
    });
    connect(loginLog, &Loginlog::sig_connect_tcp,
            TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success,
            loginLog, &Loginlog::slot_tcp_con_finish);
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_swich_chatdlg, this, &MainWindow::SlotSwitchChat);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchChat()
{
    ui->authStackedWidget->setCurrentWidget(chatLog);
    setWindowTitle("Chat");
    setMinimumSize(QSize(1050, 700));
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    resize(1050, 700);
}
