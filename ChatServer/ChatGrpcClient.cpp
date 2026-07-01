#include "ChatGrpcClient.h"

#include "ConfigMgr.h"
#include <sstream>

using grpc::ClientContext;
using grpc::Status;

ChatGrpcClient::ChatGrpcClient()
{
    auto& cfg = ConfigMgr::Inst();
    auto server_list = cfg["PeerServer"]["Servers"];

    std::vector<std::string> words;
    std::stringstream ss(server_list);
    std::string word;
    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }

    for (auto& server_key : words) {
        if (cfg[server_key]["Name"].empty()) {
            continue;
        }
        _pools[cfg[server_key]["Name"]] = std::make_unique<ChatConPool>(5, cfg[server_key]["Host"], cfg[server_key]["Port"]);
    }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(const std::string& server_name, const AddFriendReq& req)
{
    AddFriendRsp rsp;
    auto iter = _pools.find(server_name);
    if (iter == _pools.end()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    auto stub = iter->second->getConnection();
    if (stub == nullptr) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    ClientContext context;
    Status status = stub->NotifyAddFriend(&context, req, &rsp);
    iter->second->returnConnection(std::move(stub));
    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
    }
    return rsp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(const std::string& server_name, const AuthFriendReq& req)
{
    AuthFriendRsp rsp;
    auto iter = _pools.find(server_name);
    if (iter == _pools.end()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    auto stub = iter->second->getConnection();
    if (stub == nullptr) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    ClientContext context;
    Status status = stub->NotifyAuthFriend(&context, req, &rsp);
    iter->second->returnConnection(std::move(stub));
    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
    }
    return rsp;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(const std::string& server_name, const TextChatMsgReq& req)
{
    TextChatMsgRsp rsp;
    auto iter = _pools.find(server_name);
    if (iter == _pools.end()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    auto stub = iter->second->getConnection();
    if (stub == nullptr) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    ClientContext context;
    Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
    iter->second->returnConnection(std::move(stub));
    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
    }
    return rsp;
}
