#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <csignal>
#include <functional>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class ConfigMgr;
extern ConfigMgr gCfgMgr;

enum ErrorCodes{
    Success = 0,
    Error_Json = 1001,
    RPCFailed = 1002,
    PasswdErr = 1003,
    VarifyExpired = 1004,
    VarifyCodeErr = 1005,
    UserExist = 1006,
    PasswdInvalid = 1007,
    RPCGetFailed = 1008,
    UidInvalid = 1009,
    TokenInvalid = 1010,
    MysqlFailed = 1011,
    UserNotFound = 1012,
    AlreadyFriend = 1013,
    SelfAddInvalid = 1014,
    UidInvaild = UidInvalid,
    TokenInvaild = TokenInvalid,
};

struct UserInfo {
    int uid = 0;
    std::string name;
    std::string email;
    std::string pwd;
    std::string nick;
    std::string desc;
    int sex = 0;
    std::string icon;
    bool is_friend = false;
};

class Defer {
public:
    explicit Defer(std::function<void()> func) : func_(std::move(func)) {}
    ~Defer() {
        if (func_) {
            func_();
        }
    }

private:
    std::function<void()> func_;
};

#define CODEPREFIX  "code_"
#define USERTOKENPREFIX "utoken_"
#define USERIPPREFIX "uip_"
#define LOGIN_COUNT "login_count"
#define USER_BASE_INFO "ubaseinfo_"
#define HEAD_TOTAL_LEN 6
#define HEAD_ID_LEN 2
#define HEAD_DATA_LEN 4
#define MAX_LENGTH 1024
#define MSG_CHAT_LOGIN 1004
#define MSG_CHAT_LOGIN_RSP 1005
#define MSG_SEARCH_USER_REQ 1006
#define MSG_SEARCH_USER_RSP 1007
#define MSG_ADD_FRIEND_REQ 1008
#define MSG_ADD_FRIEND_RSP 1009
#define MSG_NOTIFY_ADD_FRIEND_REQ 1010
#define MSG_AUTH_FRIEND_REQ 1011
#define MSG_AUTH_FRIEND_RSP 1012
#define MSG_NOTIFY_AUTH_FRIEND_REQ 1013
