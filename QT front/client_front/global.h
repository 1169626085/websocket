#ifndef GLOBAL_H
#define GLOBAL_H
#include<QWidget>
#include<functional>
#include"QStyle"
#include <QJsonObject>
extern std::function<void(QWidget*)> repolish;
class global
{
public:
};

#define gate_url_prefix "http://localhost:8080"
enum ReqId{
    ID_GET_VARIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //注册用户
    ID_LOGIN_USER=1003, //登入
    ID_CHAT_LOGIN=1004,//登录聊天服务器
    ID_CHAT_LOGIN_RSP= 1005, //登陆聊天服务器回包
};
enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1, //Json解析失败
    ERR_NETWORK = 2,
};
enum Modules{
    REGISTERMOD = 0,
    LOGINMOD=1,

};
struct ServerInfo
{
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};
#endif // GLOBAL_H
