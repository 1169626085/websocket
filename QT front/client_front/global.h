#ifndef GLOBAL_H
#define GLOBAL_H

#include <QJsonObject>
#include <QMetaType>
#include <QStyle>
#include <QString>
#include <QVector>
#include <QWidget>
#include <functional>
#include <memory>

extern std::function<void(QWidget*)> repolish;

class global
{
public:
};

#define gate_url_prefix "http://localhost:8080"

enum ReqId{
    ID_GET_VARIFY_CODE = 1001,
    ID_REG_USER = 1002,
    ID_LOGIN_USER = 1003,
    ID_CHAT_LOGIN = 1004,
    ID_CHAT_LOGIN_RSP = 1005,
    ID_SEARCH_USER_REQ = 1006,
    ID_SEARCH_USER_RSP = 1007,
    ID_ADD_FRIEND_REQ = 1008,
    ID_ADD_FRIEND_RSP = 1009,
    ID_NOTIFY_ADD_FRIEND_REQ = 1010,
    ID_AUTH_FRIEND_REQ = 1011,
    ID_AUTH_FRIEND_RSP = 1012,
    ID_NOTIFY_AUTH_FRIEND_REQ = 1013,
};

enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1,
    ERR_NETWORK = 2,
    ERR_SERVER = 3,
};

enum Modules{
    REGISTERMOD = 0,
    LOGINMOD = 1,
};

struct ServerInfo
{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

class SearchInfo {
public:
    SearchInfo();
    SearchInfo(int uid, const QString& name, const QString& email, const QString& nick,
               const QString& desc, int sex, const QString& icon, bool isFriend);

    int _uid;
    QString _name;
    QString _email;
    QString _nick;
    QString _desc;
    int _sex;
    QString _icon;
    bool _isFriend;
};

class FriendApplyInfo {
public:
    FriendApplyInfo();
    FriendApplyInfo(int fromUid, int toUid, const QString& fromName, const QString& message);

    int _fromUid;
    int _toUid;
    QString _fromName;
    QString _message;
};

Q_DECLARE_METATYPE(std::shared_ptr<SearchInfo>)
Q_DECLARE_METATYPE(QVector<std::shared_ptr<SearchInfo>>)
Q_DECLARE_METATYPE(std::shared_ptr<FriendApplyInfo>)

#endif // GLOBAL_H
