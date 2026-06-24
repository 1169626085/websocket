#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket))
{
}

void HttpConnection::start()
{
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, request_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            try {
                if (ec) {
                    std::cout << "http read err is " << ec.message() << std::endl;
                    return;
                }

                boost::ignore_unused(bytes_transferred);
                self->HandleReq();
                self->CheckDeadline();
            }
            catch (std::exception& exp) {
                std::cout << "exception is " << exp.what() << std::endl;
            }
        });
}

void HttpConnection::CheckDeadline()
{
    auto self = shared_from_this();
    deadline_.async_wait(
        [self](beast::error_code ec) {
            if (!ec) {
                self->socket_.close(ec);
            }
        }
    );
}

void HttpConnection::PreParseGetParam()
{
    auto uri=request_.target();
    auto query_pos = uri.find('?');
    if(query_pos == std::string::npos){
        get_url_=uri;
        return;
    }
    get_url_=uri.substr(0,query_pos);
    std::string query_string = uri.substr(query_pos+1);
    std::string key;
    std::string value;
    size_t pos=0;
    while((pos = query_string.find('&'))!=std::string::npos){
        auto pair = query_string.substr(0,pos);
        size_t eq_pos = pair.find('=');
        if(eq_pos != std::string::npos){
            key = UrlDecode(pair.substr(0,eq_pos));
            value = UrlDecode(pair.substr(eq_pos+1));
            get_params_[key]=value;
        }
        query_string.erase(0,pos+1);
    }
      if (!query_string.empty()) {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, eq_pos));
            value = UrlDecode(query_string.substr(eq_pos + 1));
            get_params_[key] = value;
        }
    }

}

void HttpConnection::WriteResponse()
{
    auto self = shared_from_this();
    response_.content_length(response_.body().size());
    http::async_write(
        socket_,
        response_,
        [self](beast::error_code ec, std::size_t) {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline_.cancel();
        });
}

void HttpConnection::HandleReq()
{
    response_.version(request_.version());
    response_.keep_alive(false);

    if (request_.method() == http::verb::get) {
        PreParseGetParam();
        bool success = LogicSystem::GetInstance()->HandleGet(std::string(request_.target()), shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }
    if (request_.method() == http::verb::post) {
        bool success = LogicSystem::GetInstance()->HandlePost(request_.target(), shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }

    response_.result(http::status::bad_request);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body()) << "unsupported request-method\r\n";
    WriteResponse();
}

std::string HttpConnection::UrlDecode(const std::string &str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        
        if (str[i] == '+') strTemp += ' ';
        //
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}

unsigned char HttpConnection::FromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string HttpConnection::UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ') //
            strTemp += "+";
        else
        {
            //
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] & 0x0F);
        }
    }
    return strTemp;
}

unsigned char HttpConnection::ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}
