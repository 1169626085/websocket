#ifndef FRIENDAPPLYPAGE_H
#define FRIENDAPPLYPAGE_H

#include <QWidget>
#include <QVector>
#include "global.h"
#include "tcpmgr.h"

namespace Ui {
class FriendApplyPage;
}

class FriendApplyPage : public QWidget
{
    Q_OBJECT

public:
    explicit FriendApplyPage(QWidget *parent = nullptr);
    ~FriendApplyPage();

    void set_pending_applies(const QVector<std::shared_ptr<FriendApplyInfo>>& applies);
    void append_pending_apply(const std::shared_ptr<FriendApplyInfo>& apply);
    QVector<std::shared_ptr<FriendApplyInfo>> pending_applies() const;

signals:
    void sig_apply_accepted(int fromUid, QString fromName);

private slots:
    void slot_send_apply();
    void slot_user_search(QVector<std::shared_ptr<SearchInfo>> results);
    void slot_add_friend_result(int error, int toUid, QString toName);
    void slot_auth_friend_result(int error, int fromUid);

private:
    void render_list();
    void add_section_row(const QString& title);
    void add_search_row(const std::shared_ptr<SearchInfo>& info);
    void add_apply_row(const std::shared_ptr<FriendApplyInfo>& apply);
    void send_add_friend(const std::shared_ptr<SearchInfo>& info);
    void send_auth_friend(int fromUid);
    void remove_pending_apply(int fromUid);

    Ui::FriendApplyPage *ui;
    QVector<std::shared_ptr<FriendApplyInfo>> _pendingApplies;
    QVector<std::shared_ptr<SearchInfo>> _searchResults;
};

#endif // FRIENDAPPLYPAGE_H
