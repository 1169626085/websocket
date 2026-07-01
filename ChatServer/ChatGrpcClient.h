#pragma once

#include "ChatConPool.h"
#include "Singleton.h"
#include "const.h"

using message::AddFriendReq;
using message::AddFriendRsp;
using message::AuthFriendReq;
using message::AuthFriendRsp;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;

class ChatGrpcClient: public Singleton<ChatGrpcClient>
{
    friend class Singleton<ChatGrpcClient>;
public:
    ~ChatGrpcClient() {}
    AddFriendRsp NotifyAddFriend(const std::string& server_name, const AddFriendReq& req);
    AuthFriendRsp NotifyAuthFriend(const std::string& server_name, const AuthFriendReq& req);
    TextChatMsgRsp NotifyTextChatMsg(const std::string& server_name, const TextChatMsgReq& req);

private:
    ChatGrpcClient();
    std::unordered_map<std::string, std::unique_ptr<ChatConPool>> _pools;
};
