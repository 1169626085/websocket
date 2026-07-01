#pragma once
#include "const.h"
#include "MysqlDao.h"
#include "Singleton.h"

class MysqlMgr:public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    int RegUser(const std::string& name,const std::string& email,const std::string& pwd);
    bool CheckPwd(const std::string& name,const std::string& pwd,UserInfo& userInfo);
    std::shared_ptr<UserInfo> GetUser(int uid);
    std::vector<UserInfo> SearchUser(const std::string& key, int self_uid);
    bool IsFriend(int self_uid, int friend_uid);
    bool AddFriendApply(int from_uid, int to_uid, const std::string& message);
    bool AuthFriendApply(int from_uid, int to_uid);
    std::vector<UserInfo> GetFriendList(int uid);
private:
    MysqlMgr();
    MysqlDao _dao;
};
