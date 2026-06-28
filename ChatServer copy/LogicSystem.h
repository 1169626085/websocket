#pragma once
#include "Singleton.h"
#include "CSession.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "const.h"

class LogicNode {
public:
    LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recv_node)
        : _session(std::move(session)), _recv_node(std::move(recv_node)) {}

    std::shared_ptr<CSession> _session;
    std::shared_ptr<RecvNode> _recv_node;
};

class LogicSystem: public Singleton<LogicSystem>{
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem();
    void RegisterCallBacks(){
        _fun_callbacks[MSG_CHAT_LOGIN]=std::bind(&LogicSystem::LoginHandler,this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    void LoginHandler(std::shared_ptr<CSession> session, const short &msg_id, const std::string &msg_data);
    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
    void DealMsg();

private:
    LogicSystem();
    bool _b_stop;
    std::thread _worker_thread;
    std::mutex _mutex;
    std::condition_variable _consume;
    std::queue<std::shared_ptr<LogicNode>> _msg_que;
    std::map<short, std::function<void(std::shared_ptr<CSession>, const short&, const std::string&)>> _fun_callbacks;
    std::map<int, std::shared_ptr<UserInfo>> _users;
 };
