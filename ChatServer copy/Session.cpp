#include "Session.h"
#include "Server.h"
void Session::do_read()
    {
        auto self=shared_from_this();

        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [self](boost::system::error_code ec,std::size_t len){
                if(!ec){
                    std::string msg(self->buffer_,len);
                    std::cout<<"clinet #"<<self->id_
                             <<"says:"
                             <<msg
                             <<std::endl;
                    self->server_.handle_msg_from_client(msg);
                    self->do_read();
                }else {
                    std::cout << "client #" << self->id_
                              << " disconnected\n";
                }
            }
        );
    }

void Session::send(const std::string & msg)
{
    socket_.write_some(boost::asio::buffer(msg));
}
   