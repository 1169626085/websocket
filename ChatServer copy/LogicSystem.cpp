#include "LogicSystem.h"
#include "MysqlMgr.h"
#include "StatusGrpcClient.h"

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
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);

    auto uid = root["uid"].asInt();
    std::cout << "user login uid is " << uid << " user token is "
              << root["token"].asString() << std::endl;

    auto rsp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
    Json::Value rtvalue;
    Defer defer([&rtvalue, session, msg_id]() {
        std::string return_str = rtvalue.toStyledString();
        session->Send(return_str, MSG_CHAT_LOGIN_RSP);
    });

    rtvalue["error"] = rsp.error();
    if (rsp.error() != ErrorCodes::Success) {
        return;
    }

    auto find_iter = _users.find(uid);
    std::shared_ptr<UserInfo> user_info = nullptr;
    if (find_iter == _users.end()) {
        user_info = MysqlMgr::GetInstance()->GetUser(uid);
        if (user_info == nullptr) {
            rtvalue["error"] = ErrorCodes::UidInvaild;
            return;
        }
        _users[uid] = user_info;
    } else {
        user_info = find_iter->second;
    }

    rtvalue["uid"] = uid;
    rtvalue["token"] = rsp.token();
    rtvalue["name"] = user_info->name;
}
