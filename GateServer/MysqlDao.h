#pragma once
#include "const.h"
#include "MysqlPool.h"
class MysqlDao
{
public:
    MysqlDao();
    ~MysqlDao();
    int RegUser(const std::string& name,const std::string& email,const std::string& pwd);
    bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
private:   
    std::unique_ptr<MysqlPool> pool_;
};
