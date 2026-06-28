#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <queue>
#include <thread>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class ConfigMgr;
extern ConfigMgr gCfgMgr;
enum ErrorCodes{
    Success=0,
    Error_Json =1001,
    RPCFailed=1002,
    PasswdErr=1003,
    VarifyExpired=1004,
    VarifyCodeErr=1005,
    UserExist=1006,
    PasswdInvalid=1007,
    RPCGetFailed=1008,
    UidInvaild=1009,
    TokenInvaild=1010,
};

struct UserInfo {
    int uid = 0;
    std::string name;
    std::string email;
    std::string pwd;
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
#define HEAD_TOTAL_LEN 6
#define HEAD_ID_LEN 2
#define HEAD_DATA_LEN 4
#define MAX_LENGTH 1024
#define MSG_CHAT_LOGIN 1004
#define MSG_CHAT_LOGIN_RSP 1005
