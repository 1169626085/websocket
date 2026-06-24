#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
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
};
#define CODEPREFIX  "code_"