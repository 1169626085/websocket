#pragma once
#include "const.h"

class HttpConnection: public std::enable_shared_from_this<HttpConnection>
{
    friend class LogicSystem;
public:
    HttpConnection(tcp::socket socket);
    void start();
private:
    void CheckDeadline();
    void PreParseGetParam();
    void WriteResponse();
    void HandleReq();
    std::string UrlDecode(const std::string& str);
    unsigned char FromHex(unsigned char x);
    std::string UrlEncode(const std::string &str);
    unsigned char ToHex(unsigned char x);
    tcp::socket socket_;
    beast::flat_buffer buffer_{8192};

    http::request<http::dynamic_body> request_;

    http::response<http::dynamic_body> response_;

    net::steady_timer deadline_{
        socket_.get_executor(), std::chrono::seconds(60)
    };

    std::string get_url_;
    std::unordered_map<std::string,std::string> get_params_;
};
