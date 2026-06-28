#include "friendapplypage.h"
#include "ui_friendapplypage.h"

#include <QDateTime>

FriendApplyPage::FriendApplyPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendApplyPage)
{
    ui->setupUi(this);
    setWindowTitle("Friend Requests");

    ui->requestList->addItem("Ada  wants to add you  09:20");
    ui->requestList->addItem("Ming  waiting for approval  Yesterday");
    ui->requestList->addItem("Sent to Lin  waiting");

    connect(ui->sendApplyButton, &QPushButton::clicked, this, &FriendApplyPage::slot_send_apply);
    connect(ui->closeButton, &QPushButton::clicked, this, &FriendApplyPage::close);
}

FriendApplyPage::~FriendApplyPage()
{
    delete ui;
}

void FriendApplyPage::slot_send_apply()
{
    const QString account = ui->accountLineEdit->text().trimmed();
    if (account.isEmpty()) {
        return;
    }

    const QString time = QDateTime::currentDateTime().toString("HH:mm");
    ui->requestList->insertItem(0, QString("Sent to %1  %2").arg(account, time));
    ui->accountLineEdit->clear();
    ui->noteTextEdit->clear();
}
