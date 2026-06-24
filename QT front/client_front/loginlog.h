#ifndef LOGINLOG_H
#define LOGINLOG_H

#include <QWidget>
#include "global.h"
#include <httpmgr.h>
namespace Ui {
class Loginlog;
}

class Loginlog : public QWidget
{
    Q_OBJECT

public:
    explicit Loginlog(QWidget *parent = nullptr);
    ~Loginlog();
    void showTip(QString str,bool b_ok);

    bool checkUserValid();
    bool checkPwdValid();
    void initHttpHandlers();
signals:
    void showRegisterRequested();

private slots:
    void on_loginButton_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);

private:
    Ui::Loginlog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;

};

#endif // LOGINLOG_H
