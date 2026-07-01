#include "LogicSystem.h"
#include "ChatGrpcClient.h"
#include "ConfigMgr.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"
#include "UserMgr.h"

LogicSystem::LogicSystem() : _b_stop(false)
{
    RegisterCallBacks();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _b_stop = true;
    }
    _consume.notify_one();
    if (_worker_thread.joinable()) {
        _worker_thread.join();
    }
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _msg_que.push(std::move(msg));
    _consume.notify_one();
}

void LogicSystem::DealMsg()
{
    for (;;) {
        std::unique_lock<std::mutex> lock(_mutex);
        _consume.wait(lock, [this]() {
            return _b_stop || !_msg_que.empty();
        });

        if (_b_stop && _msg_que.empty()) {
            return;
        }

        auto msg_node = _msg_que.front();
        _msg_que.pop();
        lock.unlock();

        short msg_id = static_cast<short>(msg_node->_recv_node->_msg_id);
        auto iter = _fun_callbacks.find(msg_id);
        if (iter == _fun_callbacks.end()) {
            std::cout << "not found callback for msg id " << msg_id << std::endl;
            continue;
        }

        std::string msg_data(msg_node->_recv_node->_data, msg_node->_recv_node->_cur_len);
        iter->second(msg_node->_session, msg_id, msg_data);
    }
}

void LogicSystem::LoginHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
    (void)msg_id;

    Json::Value root;
    if (!ParseJson(msg_data, root)) {
        Json::Value rtvalue;
        rtvalue["error"] = ErrorCodes::Error_Json;
        session->Send(rtvalue.toStyledString(), MSG_CHAT_LOGIN_RSP);
        return;
    }

    const auto token = root["token"].asString();
    const auto uid = root["uid"].asInt();
    std::cout << "user login uid is " << uid << " user token is " << token << std::endl;

    auto rsp = StatusGrpcClient::GetInstance()->Login(uid, token);
    Json::Value rtvalue;
    Defer defer([&rtvalue, session]() {
        session->Send(rtvalue.toStyledString(), MSG_CHAT_LOGIN_RSP);
    });

    rtvalue["error"] = rsp.error();
    if (rsp.error() != ErrorCodes::Success) {
        return;
    }

    std::string uid_str = std::to_string(uid);
    std::string token_value;
    if (!RedisMgr::GetInstance()->Get(USERTOKENPREFIX + uid_str, token_value)) {
        rtvalue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    if (token_value != token) {
        rtvalue["error"] = ErrorCodes::TokenInvalid;
        return;
    }

    auto find_iter = _users.find(uid);
    std::shared_ptr<UserInfo> user_info = nullptr;
    if (find_iter == _users.end()) {
        user_info = MysqlMgr::GetInstance()->GetUser(uid);
        if (user_info == nullptr) {
            rtvalue["error"] = ErrorCodes::UidInvalid;
            return;
        }
        _users[uid] = user_info;
    } else {
        user_info = find_iter->second;
    }

    rtvalue["error"] = ErrorCodes::Success;
    rtvalue["uid"] = uid;
    rtvalue["token"] = rsp.token();
    rtvalue["pwd"] = user_info->pwd;
    rtvalue["name"] = user_info->name;
    rtvalue["email"] = user_info->email;
    rtvalue["nick"] = user_info->nick;
    rtvalue["desc"] = user_info->desc;
    rtvalue["sex"] = user_info->sex;
    rtvalue["icon"] = user_info->icon;

    auto server_name = ConfigMgr::Inst()["SelfServer"]["Name"];
    auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
    int count = 0;
    if (!rd_res.empty()) {
        count = std::stoi(rd_res);
    }
    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, std::to_string(count + 1));

    session->SetUserId(uid);
    RedisMgr::GetInstance()->Set(USERIPPREFIX + uid_str, server_name);
    UserMgr::GetInstance()->SetUserSession(uid, session);
}

void LogicSystem::SearchUserHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
    (void)msg_id;

    Json::Value root;
    Json::Value rtvalue;
    rtvalue["error"] = ErrorCodes::Success;
    rtvalue["users"] = Json::arrayValue;

    Defer defer([&rtvalue, session]() {
        session->Send(rtvalue.toStyledString(), MSG_SEARCH_USER_RSP);
    });

    if (!ParseJson(msg_data, root)) {
        rtvalue["error"] = ErrorCodes::Error_Json;
        return;
    }

    const auto keyword = root.isMember("keyword") ? root["keyword"].asString() : root["user"].asString();
    if (keyword.empty()) {
        return;
    }

    auto users = MysqlMgr::GetInstance()->SearchUser(keyword, session->GetUserId());
    for (const auto& user : users) {
        rtvalue["users"].append(UserToJson(user));
    }
}

void LogicSystem::AddFriendHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
    (void)msg_id;

    Json::Value root;
    Json::Value rtvalue;
    rtvalue["error"] = ErrorCodes::Success;

    Defer defer([&rtvalue, session]() {
        session->Send(rtvalue.toStyledString(), MSG_ADD_FRIEND_RSP);
    });

    if (!ParseJson(msg_data, root)) {
        rtvalue["error"] = ErrorCodes::Error_Json;
        return;
    }

    const int from_uid = session->GetUserId();
    const int to_uid = root.isMember("to_uid") ? root["to_uid"].asInt() : root["uid"].asInt();
    const std::string message = root["message"].asString();

    if (from_uid <= 0 || to_uid <= 0) {
        rtvalue["error"] = ErrorCodes::UidInvalid;
        return;
    }
    if (from_uid == to_uid) {
        rtvalue["error"] = ErrorCodes::SelfAddInvalid;
        return;
    }
    if (MysqlMgr::GetInstance()->IsFriend(from_uid, to_uid)) {
        rtvalue["error"] = ErrorCodes::AlreadyFriend;
        return;
    }

    auto from_user = MysqlMgr::GetInstance()->GetUser(from_uid);
    auto to_user = MysqlMgr::GetInstance()->GetUser(to_uid);
    if (from_user == nullptr || to_user == nullptr) {
        rtvalue["error"] = ErrorCodes::UserNotFound;
        return;
    }

    if (!MysqlMgr::GetInstance()->AddFriendApply(from_uid, to_uid, message)) {
        rtvalue["error"] = ErrorCodes::MysqlFailed;
        return;
    }

    rtvalue["to_uid"] = to_uid;
    rtvalue["to_name"] = to_user->name;
    NotifyAddFriend(from_uid, to_uid, from_user->name, message);
}

void LogicSystem::AuthFriendHandler(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
    (void)msg_id;

    Json::Value root;
    Json::Value rtvalue;
    rtvalue["error"] = ErrorCodes::Success;

    Defer defer([&rtvalue, session]() {
        session->Send(rtvalue.toStyledString(), MSG_AUTH_FRIEND_RSP);
    });

    if (!ParseJson(msg_data, root)) {
        rtvalue["error"] = ErrorCodes::Error_Json;
        return;
    }

    const int from_uid = root["from_uid"].asInt();
    const int to_uid = session->GetUserId();
    if (from_uid <= 0 || to_uid <= 0) {
        rtvalue["error"] = ErrorCodes::UidInvalid;
        return;
    }

    if (!MysqlMgr::GetInstance()->AuthFriendApply(from_uid, to_uid)) {
        rtvalue["error"] = ErrorCodes::MysqlFailed;
        return;
    }

    auto to_user = MysqlMgr::GetInstance()->GetUser(to_uid);
    rtvalue["from_uid"] = from_uid;
    if (to_user != nullptr) {
        rtvalue["to_name"] = to_user->name;
        NotifyAuthFriend(from_uid, to_uid, to_user->name);
    }
}

bool LogicSystem::NotifyAddFriend(int from_uid, int to_uid, const std::string& from_name, const std::string& message)
{
    Json::Value notify;
    notify["error"] = ErrorCodes::Success;
    notify["from_uid"] = from_uid;
    notify["to_uid"] = to_uid;
    notify["from_name"] = from_name;
    notify["message"] = message;

    const auto self_server = ConfigMgr::Inst()["SelfServer"]["Name"];
    std::string target_server;
    if (!RedisMgr::GetInstance()->Get(USERIPPREFIX + std::to_string(to_uid), target_server)) {
        return false;
    }

    if (target_server == self_server) {
        auto session = UserMgr::GetInstance()->GetSession(to_uid);
        if (session != nullptr) {
            session->Send(notify.toStyledString(), MSG_NOTIFY_ADD_FRIEND_REQ);
            return true;
        }
        return false;
    }

    message::AddFriendReq req;
    req.set_from_uid(from_uid);
    req.set_to_uid(to_uid);
    req.set_from_name(from_name);
    req.set_message(message);
    auto rsp = ChatGrpcClient::GetInstance()->NotifyAddFriend(target_server, req);
    return rsp.error() == ErrorCodes::Success;
}

bool LogicSystem::NotifyAuthFriend(int from_uid, int to_uid, const std::string& from_name)
{
    Json::Value notify;
    notify["error"] = ErrorCodes::Success;
    notify["from_uid"] = from_uid;
    notify["to_uid"] = to_uid;
    notify["from_name"] = from_name;

    const auto self_server = ConfigMgr::Inst()["SelfServer"]["Name"];
    std::string target_server;
    if (!RedisMgr::GetInstance()->Get(USERIPPREFIX + std::to_string(from_uid), target_server)) {
        return false;
    }

    if (target_server == self_server) {
        auto session = UserMgr::GetInstance()->GetSession(from_uid);
        if (session != nullptr) {
            session->Send(notify.toStyledString(), MSG_NOTIFY_AUTH_FRIEND_REQ);
            return true;
        }
        return false;
    }

    message::AuthFriendReq req;
    req.set_from_uid(from_uid);
    req.set_to_uid(to_uid);
    req.set_from_name(from_name);
    auto rsp = ChatGrpcClient::GetInstance()->NotifyAuthFriend(target_server, req);
    return rsp.error() == ErrorCodes::Success;
}

bool LogicSystem::ParseJson(const std::string& msg_data, Json::Value& root)
{
    Json::Reader reader;
    return reader.parse(msg_data, root) && root.isObject();
}

Json::Value LogicSystem::UserToJson(const UserInfo& user)
{
    Json::Value value;
    value["uid"] = user.uid;
    value["name"] = user.name;
    value["email"] = user.email;
    value["nick"] = user.nick;
    value["desc"] = user.desc;
    value["sex"] = user.sex;
    value["icon"] = user.icon;
    value["is_friend"] = user.is_friend;
    return value;
}


