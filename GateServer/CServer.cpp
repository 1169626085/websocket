#include "CServer.h"

CServer::CServer(net::io_context& ioc, unsigned short port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
{
}

void CServer::Start()
{
    auto self = shared_from_this();
    acceptor_.async_accept([self](beast::error_code ec, tcp::socket socket) {
        try {
            if (ec) {
                self->Start();
                return;
            }

            std::make_shared<HttpConnection>(std::move(socket))->start();
            self->Start();
        }
        catch (std::exception& exp) {
            std::cout << "exception is " << exp.what() << std::endl;
            self->Start();
        }
    });
}


