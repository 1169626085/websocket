#include "chatdialog.h"
#include "friendapplypage.h"
#include "ui_chatdialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

ChatDialog::ChatDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatDialog)
    , friendApplyPage(nullptr)
{
    ui->setupUi(this);

    load_conversation_list();
    load_contact_list();
    if (ui->conversationList->count() > 0) {
        ui->conversationList->setCurrentRow(0);
        load_messages(ui->conversationList->item(0)->data(Qt::UserRole).toString());
    }

    connect(ui->sendButton, &QPushButton::clicked, this, &ChatDialog::slot_send_message);
    connect(ui->receiveButton, &QPushButton::clicked, this, &ChatDialog::slot_receive_message);
    connect(ui->addToolButton, &QToolButton::clicked, this, &ChatDialog::slot_show_friend_apply);
    connect(ui->newChatButton, &QPushButton::clicked, this, &ChatDialog::slot_show_friend_apply);
    connect(ui->conversationList, &QListWidget::currentItemChanged, this, &ChatDialog::slot_switch_conversation);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::slot_send_message()
{
    const QString message = ui->messageEdit->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    add_message_bubble("Me", message, "now", true);
    ui->messageEdit->clear();
}

void ChatDialog::slot_receive_message()
{
    add_message_bubble(ui->chatTitleLabel->text(), "Got it. I will check it soon.", "now", false);
}

void ChatDialog::slot_show_friend_apply()
{
    if (friendApplyPage == nullptr) {
        friendApplyPage = new FriendApplyPage();
        friendApplyPage->setAttribute(Qt::WA_DeleteOnClose);
        connect(friendApplyPage, &QObject::destroyed, this, [this]() {
            friendApplyPage = nullptr;
        });
    }

    friendApplyPage->show();
    friendApplyPage->raise();
    friendApplyPage->activateWindow();
}

void ChatDialog::slot_switch_conversation(QListWidgetItem *current)
{
    if (current == nullptr) {
        return;
    }

    load_messages(current->data(Qt::UserRole).toString());
}

void ChatDialog::load_conversation_list()
{
    ui->conversationList->clear();
    add_conversation_item("Ada", "The login flow is ready.", "09:42", 2);
    add_conversation_item("Ming", "Can you check the status server?", "08:31", 0);
    add_conversation_item("Qt Group", "UI draft updated.", "Yesterday", 5);
    add_conversation_item("Lin", "Redis is connected now.", "Mon", 0);
}

void ChatDialog::load_contact_list()
{
    ui->contactList->clear();
    add_contact_item("Ada", "online");
    add_contact_item("Ming", "busy");
    add_contact_item("Lin", "offline");
    add_contact_item("Chen", "online");
}

void ChatDialog::load_messages(const QString &name)
{
    clear_messages();

    ui->chatTitleLabel->setText(name);
    ui->chatSubTitleLabel->setText("online");

    add_message_bubble(name, "I prepared the chat view draft.", "09:30", false);
    add_message_bubble("Me", "Great. The list and message area should both load dynamically.", "09:34", true);
    add_message_bubble(name, "Then I will add bubble messages too.", "09:40", false);
}

void ChatDialog::add_conversation_item(const QString &name, const QString &message, const QString &time, int unread)
{
    QListWidgetItem *item = new QListWidgetItem(ui->conversationList);
    item->setData(Qt::UserRole, name);
    item->setSizeHint(QSize(248, 70));

    QWidget *row = new QWidget(ui->conversationList);
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(10, 8, 10, 8);
    rowLayout->setSpacing(10);

    QLabel *avatar = new QLabel(name.left(1).toUpper(), row);
    avatar->setFixedSize(38, 38);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background:#2f80ed;color:white;border-radius:19px;font-weight:700;");

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4);

    QLabel *nameLabel = new QLabel(name, row);
    nameLabel->setStyleSheet("font-weight:700;color:#1d2733;");
    QLabel *messageLabel = new QLabel(message, row);
    messageLabel->setStyleSheet("color:#708090;font-size:12px;");
    messageLabel->setWordWrap(false);
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(messageLabel);

    QVBoxLayout *metaLayout = new QVBoxLayout();
    metaLayout->setContentsMargins(0, 0, 0, 0);
    metaLayout->setSpacing(6);
    QLabel *timeLabel = new QLabel(time, row);
    timeLabel->setAlignment(Qt::AlignRight);
    timeLabel->setStyleSheet("color:#9aa8b6;font-size:11px;");
    metaLayout->addWidget(timeLabel);

    QLabel *unreadLabel = new QLabel(row);
    unreadLabel->setFixedSize(22, 22);
    unreadLabel->setAlignment(Qt::AlignCenter);
    unreadLabel->setText(unread > 0 ? QString::number(unread) : QString());
    unreadLabel->setVisible(unread > 0);
    unreadLabel->setStyleSheet("background:#ef4444;color:white;border-radius:11px;font-size:11px;font-weight:700;");
    metaLayout->addWidget(unreadLabel, 0, Qt::AlignRight);
    metaLayout->addStretch();

    rowLayout->addWidget(avatar);
    rowLayout->addLayout(textLayout, 1);
    rowLayout->addLayout(metaLayout);

    ui->conversationList->addItem(item);
    ui->conversationList->setItemWidget(item, row);
}

void ChatDialog::add_contact_item(const QString &name, const QString &status)
{
    QListWidgetItem *item = new QListWidgetItem(ui->contactList);
    item->setSizeHint(QSize(248, 48));

    QWidget *row = new QWidget(ui->contactList);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(10);

    QLabel *avatar = new QLabel(name.left(1).toUpper(), row);
    avatar->setFixedSize(30, 30);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background:#24a148;color:white;border-radius:15px;font-weight:700;");

    QLabel *nameLabel = new QLabel(name, row);
    nameLabel->setStyleSheet("font-weight:600;color:#1d2733;");

    QLabel *statusLabel = new QLabel(status, row);
    statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusLabel->setStyleSheet("color:#708090;font-size:12px;");

    layout->addWidget(avatar);
    layout->addWidget(nameLabel, 1);
    layout->addWidget(statusLabel);

    ui->contactList->addItem(item);
    ui->contactList->setItemWidget(item, row);
}

void ChatDialog::add_message_bubble(const QString &sender, const QString &message, const QString &time, bool self)
{
    QWidget *row = new QWidget(ui->messageScrollContent);
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    QLabel *avatar = new QLabel(self ? "ME" : sender.left(1).toUpper(), row);
    avatar->setFixedSize(34, 34);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(self
                              ? "background:#24a148;color:white;border-radius:17px;font-weight:700;"
                              : "background:#2f80ed;color:white;border-radius:17px;font-weight:700;");

    QFrame *bubble = new QFrame(row);
    bubble->setMaximumWidth(440);
    bubble->setStyleSheet(self
                              ? "QFrame{background:#dff6e7;border-radius:8px;}"
                              : "QFrame{background:#ffffff;border:1px solid #dde5ee;border-radius:8px;}");

    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(12, 9, 12, 9);
    bubbleLayout->setSpacing(5);

    QLabel *metaLabel = new QLabel(QString("%1  %2").arg(sender, time), bubble);
    metaLabel->setStyleSheet("color:#708090;font-size:11px;");

    QLabel *messageLabel = new QLabel(message, bubble);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageLabel->setStyleSheet("color:#1d2733;font-size:14px;line-height:150%;");

    bubbleLayout->addWidget(metaLabel);
    bubbleLayout->addWidget(messageLabel);

    if (self) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubble);
        rowLayout->addWidget(avatar);
    } else {
        rowLayout->addWidget(avatar);
        rowLayout->addWidget(bubble);
        rowLayout->addStretch();
    }

    ui->messageListLayout->addWidget(row);
    QTimer::singleShot(0, this, [this]() {
        ui->messageScrollArea->verticalScrollBar()->setValue(ui->messageScrollArea->verticalScrollBar()->maximum());
    });
}

void ChatDialog::clear_messages()
{
    QLayoutItem *item = nullptr;
    while ((item = ui->messageListLayout->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}
