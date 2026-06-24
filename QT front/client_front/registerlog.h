#ifndef REGISTERLOG_H
#define REGISTERLOG_H

#include <QWidget>
#include "global.h"
#include "httpmgr.h"
namespace Ui {
class Registerlog;
}

class Registerlog : public QWidget
{
    Q_OBJECT

public:
    explicit Registerlog(QWidget *parent = nullptr);
    ~Registerlog();
    void showTip(QString str,bool b_ok);
    void slot_reg_mod_finish(ReqId id,QString res,ErrorCodes err);
    void initHttpHandlers();

signals:
    void backToLoginRequested();

private slots:
    void on_registerButton_clicked();
    void on_getVarifyCodeButton_clicked();

private:
    Ui::Registerlog *ui;
    QMap<ReqId,std::function<void(const QJsonObject&)>> _handlers;
};

#endif // REGISTERLOG_H
