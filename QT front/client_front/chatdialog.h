#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QWidget>
#include "global.h"

class QLabel;
class QListWidgetItem;

namespace Ui {
class ChatDialog;
}

class FriendApplyPage;

class ChatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();

private slots:
    void slot_send_message();
    void slot_receive_message();
    void slot_show_friend_apply();
    void slot_switch_conversation(QListWidgetItem *current);
    void slot_open_contact_conversation(QListWidgetItem *item);
    void slot_friend_apply_notify(std::shared_ptr<FriendApplyInfo> apply);
    void slot_friend_auth_notify(int fromUid, int toUid, QString fromName);

private:
    void sync_pending_applies_to_page();
    void load_conversation_list();
    void load_contact_list();
    void load_messages(const QString &name);
    void add_conversation_item(const QString &name, const QString &message, const QString &time, int unread);
    void add_contact_item(const QString &name, const QString &status);
    void add_message_bubble(const QString &sender, const QString &message, const QString &time, bool self);
    void clear_messages();
    void set_apply_red_dot(bool visible);
    void remove_pending_apply(int fromUid);
    bool has_conversation(const QString& name) const;
    bool has_contact(const QString& name) const;
    void open_or_create_conversation(const QString& name, const QString& message = QString("开始聊天"));

    Ui::ChatDialog *ui;
    FriendApplyPage *friendApplyPage;
    QLabel *friendApplyRedDot;
    QVector<std::shared_ptr<FriendApplyInfo>> pendingFriendApplies;
    bool hasFriendApplyUnread;
};

#endif // CHATDIALOG_H
