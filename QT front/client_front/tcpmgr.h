#ifndef TCPMGR_H
#define TCPMGR_H
#include <QTcpSocket>
#include <QVector>
#include "singleton.h"
#include "global.h"
#include "usermgr.h"
#include "loginlog.h"
class TcpMgr:public QObject,public Singleton<TcpMgr>,public enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    TcpMgr();
private:
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint32 _message_len;
    void initHandlers();
    void handleMSg(ReqId id,int len,QByteArray data);
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
public slots:
    void slot_tcp_connect(ServerInfo);
    void slot_send_data(ReqId reqId, QString data);
signals:
    void sig_con_success(bool bsuccess);
    void sig_send_data(ReqId reqId, QString data);
    void sig_login_failed(int);
    void sig_swich_chatdlg();
    void sig_user_search(QVector<std::shared_ptr<SearchInfo>>);
    void sig_add_friend_result(int error, int toUid, QString toName);
    void sig_friend_apply_notify(std::shared_ptr<FriendApplyInfo>);
    void sig_auth_friend_result(int error, int fromUid);
    void sig_friend_auth_notify(int fromUid, int toUid, QString fromName);


};

#endif // TCPMGR_H
