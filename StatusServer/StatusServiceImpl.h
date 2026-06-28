#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <map>
#include <mutex>
#include <string>
#include <vector>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

struct ChatServer
{
    std::string host;
    std::string port;
};

class StatusServiceImpl final : public StatusService::Service
{
public:
    StatusServiceImpl();
    Status GetChatServer(ServerContext* context,const GetChatServerReq* request,GetChatServerRsp* reply) override;
    void insertToken(int uid,std::string token);
    Status Login(ServerContext* context, const LoginReq* request, LoginRsp* reply) override;
    std::vector<ChatServer> _servers;
    int _server_index;
    std::mutex _token_mtx;
    std::map<int, std::string> _tokens;
};
