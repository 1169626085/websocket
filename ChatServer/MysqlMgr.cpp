#include "MysqlMgr.h"


MysqlMgr::~MysqlMgr() {

}

int MysqlMgr::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    return _dao.RegUser(name, email, pwd);
}

bool MysqlMgr::CheckPwd(const std::string &name, const std::string &pwd, UserInfo &userInfo)
{
    return _dao.CheckPwd(name, pwd, userInfo);
}

MysqlMgr::MysqlMgr() {
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid)
{
    return _dao.GetUser(uid);
}

std::vector<UserInfo> MysqlMgr::SearchUser(const std::string& key, int self_uid)
{
    return _dao.SearchUser(key, self_uid);
}

bool MysqlMgr::IsFriend(int self_uid, int friend_uid)
{
    return _dao.IsFriend(self_uid, friend_uid);
}

bool MysqlMgr::AddFriendApply(int from_uid, int to_uid, const std::string& message)
{
    return _dao.AddFriendApply(from_uid, to_uid, message);
}

bool MysqlMgr::AuthFriendApply(int from_uid, int to_uid)
{
    return _dao.AuthFriendApply(from_uid, to_uid);
}

std::vector<UserInfo> MysqlMgr::GetFriendList(int uid)
{
    return _dao.GetFriendList(uid);
}
