#pragma once
#include<boost/asio.hpp>
#include<string>
#include<iostream>
#include<thread>
#include<memory>
#include<map>
#include "CSession.h"
using namespace std;
using boost::asio::ip::tcp;
struct messbody{
    int id;
    std::string message;
};
class CServer{
public:
    CServer(boost::asio::io_context& io,unsigned short port);
    ~CServer();
    void ClearSession(int uuid);
  

private:
    void HandleAccept(shared_ptr<CSession>,const boost::system::error_code & error);
    void StartAccept();
    boost::asio::io_context& _io_context;
    short _port;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<int,std::shared_ptr<CSession>> _sessions;
    int next_client_id=1;
    std::mutex _mutex;
  

};





