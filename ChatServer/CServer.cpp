#include "Server.h"


Server::Server(boost::asio::io_context & io, unsigned short port):io_context_(io),acceptor_(io,boost::asio::ip::tcp::endpoint(
    boost::asio::ip::tcp::v4(),port))
{
}
Server::~Server()
{

}
void Server::do_accept()
{
 
     acceptor_.async_accept([this](boost::system::error_code ec,boost::asio::ip::tcp::socket socket){
        if(!ec){
            int id=next_client_id;
            auto client_session=std::make_shared<Session>(std::move(socket),id, *this);
            clients[id]=client_session;
            next_client_id++;
            client_session->start();

        }
        do_accept();
    
     });
    
}

void Server::handle_msg_from_client(const std::string& msg)
{
    auto body=parse_string_get_int(msg);
    int id=body.id;
    std::string msg_temp=body.message;

    auto it=clients.find(id);
    it->second->send(msg_temp);
    

}

 messbody Server::parse_string_get_int(const std::string& msg)
{
    std::string prefix="send";
    std::string marker="to";
    int id_int;
    std::string message;
    if(msg.rfind(prefix,0)==0)
    {
        size_t pos = msg.rfind(marker);
        if(pos!=std::string::npos){
            message=msg.substr(prefix.size(),pos-prefix.size());
            std::string id_text=msg.substr(pos+marker.size());
            id_int=std::stoi(id_text);
        }

    }

return {id_int,message};
}


