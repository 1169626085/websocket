#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QWidget>

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

private:
    void load_conversation_list();
    void load_contact_list();
    void load_messages(const QString &name);
    void add_conversation_item(const QString &name, const QString &message, const QString &time, int unread);
    void add_contact_item(const QString &name, const QString &status);
    void add_message_bubble(const QString &sender, const QString &message, const QString &time, bool self);
    void clear_messages();

    Ui::ChatDialog *ui;
    FriendApplyPage *friendApplyPage;
};

#endif // CHATDIALOG_H
