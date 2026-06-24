#pragma once
#include "const.h"
#include "HttpConnection.h"
class CServer:public std::enable_shared_from_this<CServer>
{
public:
    CServer(net::io_context& ioc, unsigned short port);
    void Start();
private:
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
};
