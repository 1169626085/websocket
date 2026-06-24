#include <iostream>
#include <string>
#include <boost/asio.hpp>

boost::asio::io_context io_context;
boost::asio::ip::tcp::resolver resolver(io_context);
boost::asio::ip::tcp::socket sock(io_context);
char buffer[4096];

void read_handler(const boost::system::error_code &ec,std::size_t bytes_transferred)
{
    if(!ec)
    {
        std::cout<< std::string(buffer,bytes_transferred)<<std::endl;
        sock.async_read_some(boost::asio::buffer(buffer),read_handler);
    }
}

void connect_handler(const boost::system::error_code &ec, const boost::asio::ip::tcp::endpoint &)
{
    if(!ec)
    {
        boost::asio::write(sock,boost::asio::buffer("GET / HTTP/1.1\r\nHost: www.highscore.de\r\nConnection: close\r\n\r\n"));
        sock.async_read_some(boost::asio::buffer(buffer),read_handler);
    }
}

void resolve_handler(const boost::system::error_code &ec,boost::asio::ip::tcp::resolver::results_type results)
{
    if(!ec)
    {
        boost::asio::async_connect(sock,results,connect_handler);
    }
}

