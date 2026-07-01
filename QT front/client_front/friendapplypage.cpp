#include "friendapplypage.h"
#include "ui_friendapplypage.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

FriendApplyPage::FriendApplyPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendApplyPage)
{
    ui->setupUi(this);
    setWindowTitle("好友申请");

    ui->titleLabel->setText("好友申请");
    ui->sectionLabel->setText("搜索好友");
    ui->requestTitleLabel->setText("申请列表 / 搜索结果");
    ui->accountLineEdit->setPlaceholderText("输入用户名、邮箱或 uid");
    ui->noteTextEdit->setPlaceholderText("验证消息");
    ui->sendApplyButton->setText("搜索");
    ui->closeButton->setText("关闭");

    connect(ui->sendApplyButton, &QPushButton::clicked, this, &FriendApplyPage::slot_send_apply);
    connect(ui->closeButton, &QPushButton::clicked, this, &FriendApplyPage::close);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &FriendApplyPage::slot_user_search);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_friend_result, this, &FriendApplyPage::slot_add_friend_result);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_friend_result, this, &FriendApplyPage::slot_auth_friend_result);
}

FriendApplyPage::~FriendApplyPage()
{
    delete ui;
}

void FriendApplyPage::set_pending_applies(const QVector<std::shared_ptr<FriendApplyInfo>>& applies)
{
    _pendingApplies = applies;
    render_list();
}

void FriendApplyPage::append_pending_apply(const std::shared_ptr<FriendApplyInfo>& apply)
{
    for (const auto& item : _pendingApplies) {
        if (item->_fromUid == apply->_fromUid) {
            return;
        }
    }
    _pendingApplies.push_back(apply);
    render_list();
}

QVector<std::shared_ptr<FriendApplyInfo>> FriendApplyPage::pending_applies() const
{
    return _pendingApplies;
}

void FriendApplyPage::slot_send_apply()
{
    const QString keyword = ui->accountLineEdit->text().trimmed();
    if (keyword.isEmpty()) {
        return;
    }

    QJsonObject jsonObj;
    jsonObj["keyword"] = keyword;
    QJsonDocument doc(jsonObj);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,
                                              QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void FriendApplyPage::slot_user_search(QVector<std::shared_ptr<SearchInfo>> results)
{
    _searchResults = results;
    render_list();
}

void FriendApplyPage::slot_add_friend_result(int error, int toUid, QString toName)
{
    if (error != ErrorCodes::SUCCESS) {
        QMessageBox::warning(this, "发送失败", QString("好友申请发送失败，错误码：%1").arg(error));
        return;
    }

    for (int i = 0; i < _pendingApplies.size(); ++i) {
        if (_pendingApplies[i]->_fromUid == 0 && _pendingApplies[i]->_toUid == toUid) {
            _pendingApplies.removeAt(i);
            break;
        }
    }

    const QString time = QDateTime::currentDateTime().toString("HH:mm");
    auto info = std::make_shared<FriendApplyInfo>(0, toUid, toName, QString("已发送于 %1").arg(time));
    _pendingApplies.push_back(info);
    render_list();
    ui->accountLineEdit->clear();
    ui->noteTextEdit->clear();
}

void FriendApplyPage::slot_auth_friend_result(int error, int fromUid)
{
    if (error != ErrorCodes::SUCCESS) {
        QMessageBox::warning(this, "操作失败", QString("同意好友申请失败，错误码：%1").arg(error));
        return;
    }

    QString fromName;
    for (const auto& apply : _pendingApplies) {
        if (apply->_fromUid == fromUid) {
            fromName = apply->_fromName;
            break;
        }
    }

    remove_pending_apply(fromUid);
    emit sig_apply_accepted(fromUid, fromName);
}

void FriendApplyPage::render_list()
{
    ui->requestList->clear();

    add_section_row("未添加为好友");
    bool has_pending = false;
    for (const auto& info : _searchResults) {
        if (!info->_isFriend) {
            add_search_row(info);
            has_pending = true;
        }
    }
    if (!has_pending) {
        QListWidgetItem *item = new QListWidgetItem("没有未添加的搜索结果");
        item->setFlags(Qt::NoItemFlags);
        ui->requestList->addItem(item);
    }

    add_section_row("已经添加为好友");
    bool has_friend = false;
    for (const auto& info : _searchResults) {
        if (info->_isFriend) {
            add_search_row(info);
            has_friend = true;
        }
    }
    if (!has_friend) {
        QListWidgetItem *item = new QListWidgetItem("没有已添加的搜索结果");
        item->setFlags(Qt::NoItemFlags);
        ui->requestList->addItem(item);
    }

    bool has_received = false;
    for (const auto& apply : _pendingApplies) {
        if (apply->_fromUid > 0) {
            has_received = true;
            break;
        }
    }
    if (has_received) {
        add_section_row("收到的好友申请");
        for (const auto& apply : _pendingApplies) {
            if (apply->_fromUid > 0) {
                add_apply_row(apply);
            }
        }
    }

    bool has_sent = false;
    for (const auto& apply : _pendingApplies) {
        if (apply->_fromUid == 0) {
            has_sent = true;
            break;
        }
    }
    if (has_sent) {
        add_section_row("已发送的好友申请");
        for (const auto& apply : _pendingApplies) {
            if (apply->_fromUid == 0) {
                add_apply_row(apply);
            }
        }
    }
}

void FriendApplyPage::add_section_row(const QString& title)
{
    QListWidgetItem *item = new QListWidgetItem(title);
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(QSize(460, 30));
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);
    ui->requestList->addItem(item);
}

void FriendApplyPage::add_search_row(const std::shared_ptr<SearchInfo>& info)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(460, 62));
    ui->requestList->addItem(item);

    QWidget *row = new QWidget(ui->requestList);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(10);

    QLabel *avatar = new QLabel(info->_name.left(1).toUpper(), row);
    avatar->setFixedSize(34, 34);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background:#2f80ed;color:white;border-radius:17px;font-weight:700;");

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(3);
    QLabel *nameLabel = new QLabel(QString("%1  uid:%2").arg(info->_name).arg(info->_uid), row);
    nameLabel->setStyleSheet("font-weight:700;color:#1d2733;");
    QLabel *descLabel = new QLabel(info->_desc.isEmpty() ? info->_email : info->_desc, row);
    descLabel->setStyleSheet("color:#708090;font-size:12px;");
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(descLabel);

    QPushButton *addButton = new QPushButton(info->_isFriend ? "已添加" : "添加", row);
    addButton->setEnabled(!info->_isFriend);
    addButton->setFixedWidth(76);
    addButton->setStyleSheet(info->_isFriend
                                 ? "background:#e7edf5;color:#708090;border:none;border-radius:7px;min-height:30px;"
                                 : "background:#2f80ed;color:white;border:none;border-radius:7px;min-height:30px;");
    connect(addButton, &QPushButton::clicked, this, [this, info]() {
        send_add_friend(info);
    });

    layout->addWidget(avatar);
    layout->addLayout(textLayout, 1);
    layout->addWidget(addButton);

    ui->requestList->setItemWidget(item, row);
}

void FriendApplyPage::add_apply_row(const std::shared_ptr<FriendApplyInfo>& apply)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(460, 68));
    ui->requestList->addItem(item);

    QWidget *row = new QWidget(ui->requestList);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(10);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4);
    const bool sentApply = apply->_fromUid == 0;
    QLabel *title = new QLabel(sentApply
                                   ? QString("已向 %1 发送好友申请").arg(apply->_fromName)
                                   : QString("%1 请求添加你为好友").arg(apply->_fromName),
                               row);
    title->setStyleSheet("font-weight:700;color:#1d2733;");
    QLabel *message = new QLabel(apply->_message.isEmpty() ? "对方没有填写验证消息" : apply->_message, row);
    message->setStyleSheet("color:#708090;font-size:12px;");
    message->setWordWrap(true);
    textLayout->addWidget(title);
    textLayout->addWidget(message);

    QPushButton *agreeButton = new QPushButton(sentApply ? "等待" : "同意", row);
    agreeButton->setFixedWidth(76);
    agreeButton->setEnabled(!sentApply);
    agreeButton->setStyleSheet(sentApply
                                   ? "background:#e7edf5;color:#708090;border:none;border-radius:7px;min-height:30px;"
                                   : "background:#24a148;color:white;border:none;border-radius:7px;min-height:30px;");
    connect(agreeButton, &QPushButton::clicked, this, [this, apply]() {
        send_auth_friend(apply->_fromUid);
    });

    layout->addLayout(textLayout, 1);
    layout->addWidget(agreeButton);

    ui->requestList->setItemWidget(item, row);
}

void FriendApplyPage::send_add_friend(const std::shared_ptr<SearchInfo>& info)
{
    const QString message = ui->noteTextEdit->toPlainText().trimmed();
    const auto ret = QMessageBox::question(this, "发送好友申请",
                                           QString("向 %1 发送好友申请？").arg(info->_name),
                                           QMessageBox::Ok | QMessageBox::Cancel);
    if (ret != QMessageBox::Ok) {
        return;
    }

    QJsonObject jsonObj;
    jsonObj["to_uid"] = info->_uid;
    jsonObj["message"] = message;
    QJsonDocument doc(jsonObj);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_ADD_FRIEND_REQ,
                                              QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void FriendApplyPage::send_auth_friend(int fromUid)
{
    QJsonObject jsonObj;
    jsonObj["from_uid"] = fromUid;
    QJsonDocument doc(jsonObj);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_AUTH_FRIEND_REQ,
                                              QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void FriendApplyPage::remove_pending_apply(int fromUid)
{
    for (int i = 0; i < _pendingApplies.size(); ++i) {
        if (_pendingApplies[i]->_fromUid == fromUid) {
            _pendingApplies.removeAt(i);
            break;
        }
    }
    render_list();
}
