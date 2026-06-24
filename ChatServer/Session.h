#pragma once
#include<boost/asio.hpp>
#include<string>
#include<iostream>
#include<thread>
#include<memory>
class Server;
class Session:public std::enable_shared_from_this<Session>{
public:
    Session(boost::asio::ip::tcp::socket socket,int id,Server& server):socket_(std::move(socket)),id_(id),server_(server)
    {}

    void start()
    {
        std::cout<<"client #"<<id_<<"connected\n";
        do_read();
    }
    int id() const
    {
        return id_;
    }
    void send(const std::string& msg);

private:
    void do_read();
    
    
private:
        boost::asio::ip::tcp::socket socket_;
        int id_;
        char buffer_[1024];
        Server& server_;


};