#pragma once 
#include"const.h"
class ChatServiceImpl final : public ChatService::Service
{
public:
    ChatServiceImpl();
    Status NotifyAddFriend(ServerContext* context, const AddFriendReq* request,
    AddFriendRsp* reply) override;

    Status NotifyAuthFriend(ServerContext* context,
    const AuthFriendReq* request, AuthFriendRsp* response) override;

    Status NotifyTextChatMsg(::grpc::ServerContext* context,
    const TextChatMsgReq* request, TextChatMsgRsp* response) override;

    bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
private:

};