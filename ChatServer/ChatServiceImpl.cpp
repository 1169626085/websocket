#include "ChatServiceImpl.h"

#include "CSession.h"
#include "UserMgr.h"

ChatServiceImpl::ChatServiceImpl()
{
}

Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* request, AddFriendRsp* reply)
{
    (void)context;
    Json::Value notify;
    notify["error"] = ErrorCodes::Success;
    notify["from_uid"] = request->from_uid();
    notify["to_uid"] = request->to_uid();
    notify["from_name"] = request->from_name();
    notify["message"] = request->message();

    auto session = UserMgr::GetInstance()->GetSession(request->to_uid());
    if (session != nullptr) {
        session->Send(notify.toStyledString(), MSG_NOTIFY_ADD_FRIEND_REQ);
    }

    reply->set_error(ErrorCodes::Success);
    return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context, const AuthFriendReq* request, AuthFriendRsp* response)
{
    (void)context;
    Json::Value notify;
    notify["error"] = ErrorCodes::Success;
    notify["from_uid"] = request->from_uid();
    notify["to_uid"] = request->to_uid();
    notify["from_name"] = request->from_name();

    auto session = UserMgr::GetInstance()->GetSession(request->from_uid());
    if (session != nullptr) {
        session->Send(notify.toStyledString(), MSG_NOTIFY_AUTH_FRIEND_REQ);
    }

    response->set_error(ErrorCodes::Success);
    return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(ServerContext* context, const TextChatMsgReq* request, TextChatMsgRsp* response)
{
    (void)context;
    (void)request;
    response->set_error(ErrorCodes::Success);
    return Status::OK;
}
