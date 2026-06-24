#pragma once
#include<boost/asio.hpp>
#include<string>
#include<iostream>
#include<thread>
#include<memory>
#include<map>
#include "Session.h"
struct messbody{
    int id;
    std::string message;
};
class Server{
public:
    Server(boost::asio::io_context& io,unsigned short port);
    ~Server();
    void do_accept();
    void handle_msg_from_client(const std::string& msg);
    messbody parse_string_get_int(const std::string& msg);

private:

    std::map<int,std::shared_ptr<Session>> clients;
    int next_client_id=1;
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

};







