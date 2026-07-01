#include "MysqlDao.h"
#include "ConfigMgr.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/exception.h>
#include <iostream>

using std::cout;
using std::endl;
using std::unique_ptr;

namespace {
std::string SafeGetString(sql::ResultSet* res, const std::string& col)
{
    try {
        return res->getString(col);
    }
    catch (sql::SQLException&) {
        return "";
    }
}

int SafeGetInt(sql::ResultSet* res, const std::string& col)
{
    try {
        return res->getInt(col);
    }
    catch (sql::SQLException&) {
        return 0;
    }
}
}

MysqlDao::MysqlDao()
{
    auto & cfg = ConfigMgr::Inst();
    const auto& host = cfg["Mysql"]["Host"];
    const auto& port = cfg["Mysql"]["Port"];
    const auto& pwd = cfg["Mysql"]["Passwd"];
    const auto& schema = cfg["Mysql"]["Schema"];
    const auto& user = cfg["Mysql"]["User"];
    pool_.reset(new MysqlPool(host+":"+port, user, pwd, schema, 5));
    InitFriendTables();
}

MysqlDao::~MysqlDao(){
    pool_->Close();
}
int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            pool_->returnConnection(std::move(con));
            return false;
        }
        // 准备调用存储过程
        unique_ptr < sql::PreparedStatement > stmt(con->prepareStatement("CALL reg_user(?,?,?,@result)"));
        // 设置输入参数
        stmt->setString(1, name);
        stmt->setString(2, email);
        stmt->setString(3, pwd);

        // 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

          // 执行存储过程
        stmt->execute();
        // 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
       // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
       unique_ptr<sql::Statement> stmtResult(con->createStatement());
      unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
      if (res->next()) {
           int result = res->getInt("result");
          cout << "Result: " << result << endl;
          pool_->returnConnection(std::move(con));
          return result;
      }
      pool_->returnConnection(std::move(con));
        return -1;
    }
    catch (sql::SQLException& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return -1;
    }
}

bool MysqlDao::CheckPwd(const std::string &name, const std::string &pwd, UserInfo &userInfo)
{
    auto con = pool_->getConnection();
    Defer defer([this,&con](){
        pool_->returnConnection(std::move(con));
    });
    try{
        if(con == nullptr){
            return false;
        }
        //准备SQL语句
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM user WHERE name = ?"));
        pstmt->setString(1,name);
        // 执行查询
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::string origin_pwd = "";
        // 遍历结果集
        while (res->next()) {
            origin_pwd = res->getString("pwd");
            userInfo.name = name;
            userInfo.email = res->getString("email");
            userInfo.uid = res->getInt("uid");
            userInfo.pwd = origin_pwd;
            // 输出查询到的密码
            std::cout << "Password: " << origin_pwd << std::endl;
            break;
    }
     if (pwd != origin_pwd) {
            return false;
        }
        return true;
    }
    catch(sql::SQLException& e){
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

std::shared_ptr<UserInfo> MysqlDao::GetUser(int uid)
{
    auto con = pool_->getConnection();
    if (con == nullptr) {
        return nullptr;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
        });

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
        pstmt->setInt(1, uid);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::shared_ptr<UserInfo> user_ptr = nullptr;
        while (res->next()) {
            user_ptr.reset(new UserInfo);
            user_ptr->pwd = res->getString("pwd");
            user_ptr->email = res->getString("email");
            user_ptr->name = res->getString("name");
            user_ptr->uid = uid;
            user_ptr->nick = SafeGetString(res.get(), "nick");
            user_ptr->desc = SafeGetString(res.get(), "desc");
            user_ptr->sex = SafeGetInt(res.get(), "sex");
            user_ptr->icon = SafeGetString(res.get(), "icon");
            break;
        }
        return user_ptr;
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return nullptr;
    }
}

void MysqlDao::InitFriendTables()
{
    auto con = pool_->getConnection();
    if (con == nullptr) {
        return;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
    });

    try {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute("CREATE TABLE IF NOT EXISTS friend_apply ("
                      "id INT AUTO_INCREMENT PRIMARY KEY,"
                      "from_uid INT NOT NULL,"
                      "to_uid INT NOT NULL,"
                      "message VARCHAR(512) DEFAULT '',"
                      "status TINYINT NOT NULL DEFAULT 0,"
                      "create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                      "update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                      "UNIQUE KEY uniq_apply(from_uid, to_uid))");
        stmt->execute("CREATE TABLE IF NOT EXISTS friend ("
                      "id INT AUTO_INCREMENT PRIMARY KEY,"
                      "self_uid INT NOT NULL,"
                      "friend_uid INT NOT NULL,"
                      "create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                      "UNIQUE KEY uniq_friend(self_uid, friend_uid))");
    }
    catch (sql::SQLException& e) {
        std::cerr << "Init friend tables failed: " << e.what() << std::endl;
    }
}

std::vector<UserInfo> MysqlDao::SearchUser(const std::string& key, int self_uid)
{
    std::vector<UserInfo> users;
    auto con = pool_->getConnection();
    if (con == nullptr) {
        return users;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
    });

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT * FROM user WHERE uid <> ? AND "
                                  "(name LIKE ? OR email LIKE ? OR CAST(uid AS CHAR) = ?) "
                                  "LIMIT 30"));
        const std::string like_key = "%" + key + "%";
        pstmt->setInt(1, self_uid);
        pstmt->setString(2, like_key);
        pstmt->setString(3, like_key);
        pstmt->setString(4, key);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            UserInfo user;
            user.uid = res->getInt("uid");
            user.name = SafeGetString(res.get(), "name");
            user.email = SafeGetString(res.get(), "email");
            user.pwd = SafeGetString(res.get(), "pwd");
            user.nick = SafeGetString(res.get(), "nick");
            user.desc = SafeGetString(res.get(), "desc");
            user.sex = SafeGetInt(res.get(), "sex");
            user.icon = SafeGetString(res.get(), "icon");
            user.is_friend = IsFriend(self_uid, user.uid);
            users.push_back(user);
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "SearchUser SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    return users;
}

bool MysqlDao::IsFriend(int self_uid, int friend_uid)
{
    auto con = pool_->getConnection();
    if (con == nullptr) {
        return false;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
    });

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT 1 FROM friend WHERE self_uid = ? AND friend_uid = ? LIMIT 1"));
        pstmt->setInt(1, self_uid);
        pstmt->setInt(2, friend_uid);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        return res->next();
    }
    catch (sql::SQLException& e) {
        std::cerr << "IsFriend SQLException: " << e.what() << std::endl;
        return false;
    }
}

bool MysqlDao::AddFriendApply(int from_uid, int to_uid, const std::string& message)
{
    auto con = pool_->getConnection();
    if (con == nullptr) {
        return false;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
    });

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO friend_apply(from_uid, to_uid, message, status) "
                                  "VALUES(?, ?, ?, 0) "
                                  "ON DUPLICATE KEY UPDATE message = VALUES(message), status = 0, update_time = CURRENT_TIMESTAMP"));
        pstmt->setInt(1, from_uid);
        pstmt->setInt(2, to_uid);
        pstmt->setString(3, message);
        pstmt->executeUpdate();
        return true;
    }
    catch (sql::SQLException& e) {
        std::cerr << "AddFriendApply SQLException: " << e.what() << std::endl;
        return false;
    }
}

bool MysqlDao::AuthFriendApply(int from_uid, int to_uid)
{
    auto con = pool_->getConnection();
    if (con == nullptr) {
        return false;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
    });

    try {
        con->setAutoCommit(false);

        std::unique_ptr<sql::PreparedStatement> update_stmt(
            con->prepareStatement("UPDATE friend_apply SET status = 1, update_time = CURRENT_TIMESTAMP "
                                  "WHERE from_uid = ? AND to_uid = ?"));
        update_stmt->setInt(1, from_uid);
        update_stmt->setInt(2, to_uid);
        update_stmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> insert_stmt(
            con->prepareStatement("INSERT IGNORE INTO friend(self_uid, friend_uid) VALUES(?, ?)"));
        insert_stmt->setInt(1, from_uid);
        insert_stmt->setInt(2, to_uid);
        insert_stmt->executeUpdate();

        insert_stmt->setInt(1, to_uid);
        insert_stmt->setInt(2, from_uid);
        insert_stmt->executeUpdate();

        con->commit();
        con->setAutoCommit(true);
        return true;
    }
    catch (sql::SQLException& e) {
        try {
            con->rollback();
            con->setAutoCommit(true);
        }
        catch (sql::SQLException&) {
        }
        std::cerr << "AuthFriendApply SQLException: " << e.what() << std::endl;
        return false;
    }
}

std::vector<UserInfo> MysqlDao::GetFriendList(int uid)
{
    std::vector<UserInfo> friends;
        auto con = pool_->getConnection();
    if (con == nullptr) {
        return friends;
    }

    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
    });
     try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT u.uid, u.name, u.email, u.pwd "
                "FROM friend f "
                "JOIN user u ON f.friend_uid = u.uid "
                "WHERE f.self_uid = ? "
                "ORDER BY f.create_time DESC"
            )
        );

        pstmt->setInt(1, uid);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            UserInfo user;
            user.uid = res->getInt("uid");
            user.name = res->getString("name");
            user.email = res->getString("email");
            user.pwd = res->getString("pwd");
            user.is_friend = true;

            friends.push_back(user);
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "GetFriendList SQLException: " << e.what() << std::endl;
    }

    return friends;
}
