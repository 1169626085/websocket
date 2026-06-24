#include "Server.h"
#include <vector>
boost::asio::io_context io_context;

int main()
{
    Server server(io_context,80);
    server.do_accept();
    
    std::vector<std::thread> threads;
    for(int i = 0;i<4;i++){
        threads.emplace_back([]{
           io_context.run();
        });
    }
    for(auto& t :threads){
        t.join();

    }
}
