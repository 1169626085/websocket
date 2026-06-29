#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"
#include "RedisMgr.h"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string generate_unique_string(){
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    // 将UUID转换为字符串
    std::string unique_string = to_string(uuid);

    return unique_string;
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
    std::string prefix("llfc status server has received :  ");
    const auto& server = getChatServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    insertToken(request->uid(),reply->token());
    return Status::OK;
}
ChatServer StatusServiceImpl::getChatServer()
{
    std::lock_guard<std::mutex> guard(_server_mtx);
    auto minServer = _servers.begin()->second;
    auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
    if (count_str.empty()) {
        //不存在则默认设置为最大
        minServer.con_count = INT_MAX;
    }
    else {
        minServer.con_count = std::stoi(count_str);
    }


    // 使用范围基于for循环
    for (auto& server : _servers) {

        if (server.second.name == minServer.name) {
            continue;
        }

        auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
        if (count_str.empty()) {
            server.second.con_count = INT_MAX;
        }
        else {
            server.second.con_count = std::stoi(count_str);
        }

        if (server.second.con_count < minServer.con_count) {
            minServer = server.second;
        }
    }

    return minServer;
}
void StatusServiceImpl::insertToken(int uid, std::string token)
{
    std::lock_guard<std::mutex> guard(_token_mtx);
    _tokens[uid] = token;
    std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	RedisMgr::GetInstance()->Set(token_key, token);
}
Status StatusServiceImpl::Login(ServerContext *context, const LoginReq *request, LoginRsp *reply)
{
    auto uid =request->uid();
    auto token =request->token();
        std::lock_guard<std::mutex> guard(_token_mtx);
    auto iter = _tokens.find(uid);
    if (iter == _tokens.end()) {
        reply->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
    }
    if (iter->second != token) {
        reply->set_error(ErrorCodes::TokenInvalid);
        return Status::OK;
    }
    reply->set_error(ErrorCodes::Success);
    reply->set_uid(uid);
    reply->set_token(token);
    return Status::OK;
    
}
StatusServiceImpl::StatusServiceImpl() : _server_index(0)
{
    auto& cfg = ConfigMgr::Inst();
    ChatServer server;
    server.port = cfg["ChatServer1"]["Port"];
    server.host = cfg["ChatServer1"]["Host"];
    _servers.push_back(server);

    server.port = cfg["ChatServer2"]["Port"];
    server.host = cfg["ChatServer2"]["Host"];
    _servers.push_back(server);
}
