#ifndef LOGINLOG_H
#define LOGINLOG_H

#include <QWidget>
#include "global.h"
#include <httpmgr.h>
#include "tcpmgr.h"
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
    void slot_login_failed(int err);
    void slot_tcp_con_finish(bool bsuccess);
signals:
    void showRegisterRequested();
    void sig_connect_tcp(ServerInfo si);

private slots:
    void on_loginButton_clicked();
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);

private:
    Ui::Loginlog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    int _uid;
    QString _token;

};

#endif // LOGINLOG_H
