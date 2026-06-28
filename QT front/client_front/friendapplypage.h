#ifndef FRIENDAPPLYPAGE_H
#define FRIENDAPPLYPAGE_H

#include <QWidget>

namespace Ui {
class FriendApplyPage;
}

class FriendApplyPage : public QWidget
{
    Q_OBJECT

public:
    explicit FriendApplyPage(QWidget *parent = nullptr);
    ~FriendApplyPage();

private slots:
    void slot_send_apply();

private:
    Ui::FriendApplyPage *ui;
};

#endif // FRIENDAPPLYPAGE_H
