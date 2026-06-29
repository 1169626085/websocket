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
    auto token=root["token"].asString();
    auto uid = root["uid"].asInt();
    std::cout << "user login uid is " << uid << " user token is "
              << token << std::endl;

    auto rsp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
    Json::Value rtvalue;
    Defer defer([&rtvalue, session, msg_id]() {
        std::string return_str = rtvalue.toStyledString();
        session->Send(return_str, MSG_CHAT_LOGIN_RSP);
    });

    std::string uid_str = std::to_string(uid);
    std::string token_key = USERTOKENPREFIX + uid_str;
    std::string token_value="";
    bool success = RedisMgr::GetInstance()->Get(token_key,token_value);
    if(!success){
        rtvalue["error"]=ErrorCodes::UidInvaild;
        return;
    }
    if(token_value !=token){
        rtvalue["error"]=ErrorCodes::TokenInvaild;
        return;
    }

    rtvalue["error"]=ErrorCodes::Success;

    std::string base_key = USER_BASE_INFO + uid_str;
    auto user_info = std::make_shared<UserInfo>();
    bool b_base = GetBaseInfo(base_key,uid,user_info);
    if(!b_base){
        rtvalue["error"] = ErrorCodes::UidInvalid;
        return;
    }



    rtvalue["uid"] = uid;
    rtvalue["token"] = rsp.token();
    rtvalue["pwd"] = user_info->pwd;
    rtvalue["name"] = user_info->name;
    rtvalue["email"] = user_info->email;
    rtvalue["nick"] = user_info->nick;
    rtvalue["desc"] = user_info->desc;
    rtvalue["sex"] = user_info->sex;
    rtvalue["icon"] = user_info->icon;

     //从数据库获取申请列表

    //获取好友列表

    auto server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
    //将登录数量增加
    auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
    int count = 0;
    if (!rd_res.empty()) {
        count = std::stoi(rd_res);
    }

    count++;

    auto count_str = std::to_string(count);
    RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);

    //session绑定用户uid
    session->SetUserId(uid);

    //为用户设置登录ip server的名字
    std::string  ipkey = USERIPPREFIX + uid_str;
    RedisMgr::GetInstance()->Set(ipkey, server_name);

    //uid和session绑定管理,方便以后踢人操作
    UserMgr::GetInstance()->SetUserSession(uid, session);

    return;
}
