#include "RedisMgr.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
class RedisConnectionGuard {
public:
    explicit RedisConnectionGuard(RedisConPool* pool) : pool_(pool), context_(nullptr)
    {
        if (pool_ != nullptr) {
            context_ = pool_->getConnection();
        }
    }

    ~RedisConnectionGuard()
    {
        if (pool_ != nullptr && context_ != nullptr) {
            pool_->returnConnection(context_);
        }
    }

    redisContext* get() const
    {
        return context_;
    }

private:
    RedisConPool* pool_;
    redisContext* context_;
};
}

RedisMgr::RedisMgr() : _connect(nullptr), _reply(nullptr)
{
    auto& gCfgMgr = ConfigMgr::Inst();
    auto host = gCfgMgr["Redis"]["Host"];
    auto port = gCfgMgr["Redis"]["Port"];
    auto pwd = gCfgMgr["Redis"]["Passwd"];
    _con_pool.reset(new RedisConPool(5, host.c_str(), std::atoi(port.c_str()), pwd.c_str()));
}

RedisMgr::~RedisMgr()
{
    Close();
}

bool RedisMgr::Connect(const std::string& host, int port)
{
    if (_connect != nullptr) {
        redisFree(_connect);
        _connect = nullptr;
    }

    _connect = redisConnect(host.c_str(), port);
    if (_connect != nullptr && _connect->err) {
        std::cout << "connect error " << _connect->errstr << std::endl;
        redisFree(_connect);
        _connect = nullptr;
        return false;
    }

    return _connect != nullptr;
}

bool RedisMgr::Get(const std::string& key, std::string& value)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "GET %s", key.c_str()));
    if (reply == nullptr) {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(reply);
        return false;
    }

    value = reply->str;
    freeReplyObject(reply);
    std::cout << "Succeed to execute command [ GET " << key << " ]" << std::endl;
    return true;
}

bool RedisMgr::Set(const std::string& key, const std::string& value)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "SET %s %s", key.c_str(), value.c_str()));
    if (reply == nullptr) {
        std::cout << "Execute command [ SET " << key << " " << value << " ] failed" << std::endl;
        return false;
    }

    bool ok = reply->type == REDIS_REPLY_STATUS
        && (std::strcmp(reply->str, "OK") == 0 || std::strcmp(reply->str, "ok") == 0);
    freeReplyObject(reply);

    if (!ok) {
        std::cout << "Execute command [ SET " << key << " " << value << " ] failed" << std::endl;
        return false;
    }

    std::cout << "Execute command [ SET " << key << " " << value << " ] success" << std::endl;
    return true;
}

bool RedisMgr::Auth(const std::string& password)
{
    if (_connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(_connect, "AUTH %s", password.c_str()));
    if (reply == nullptr) {
        return false;
    }

    bool ok = reply->type != REDIS_REPLY_ERROR;
    freeReplyObject(reply);
    return ok;
}

bool RedisMgr::LPush(const std::string& key, const std::string& value)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str()));
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisMgr::LPop(const std::string& key, std::string& value)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "LPOP %s", key.c_str()));
    if (reply == nullptr || reply->type != REDIS_REPLY_STRING) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    value = reply->str;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str()));
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "RPOP %s", key.c_str()));
    if (reply == nullptr || reply->type != REDIS_REPLY_STRING) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    value = reply->str;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::HSet(const std::string& key, const std::string& hkey, const std::string& value)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str()));
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    const char* argv[4] = { "HSET", key, hkey, hvalue };
    size_t argvlen[4] = { 4, std::strlen(key), std::strlen(hkey), hvaluelen };
    auto reply = static_cast<redisReply*>(redisCommandArgv(connect, 4, argv, argvlen));
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    freeReplyObject(reply);
    return true;
}

std::string RedisMgr::HGet(const std::string& key, const std::string& hkey)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return "";
    }

    const char* argv[3] = { "HGET", key.c_str(), hkey.c_str() };
    size_t argvlen[3] = { 4, key.length(), hkey.length() };
    auto reply = static_cast<redisReply*>(redisCommandArgv(connect, 3, argv, argvlen));
    if (reply == nullptr || reply->type != REDIS_REPLY_STRING) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return "";
    }

    std::string value = reply->str;
    freeReplyObject(reply);
    return value;
}

bool RedisMgr::Del(const std::string& key)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "DEL %s", key.c_str()));
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisMgr::ExistsKey(const std::string& key)
{
    RedisConnectionGuard guard(_con_pool.get());
    auto connect = guard.get();
    if (connect == nullptr) {
        return false;
    }

    auto reply = static_cast<redisReply*>(redisCommand(connect, "EXISTS %s", key.c_str()));
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        return false;
    }

    bool exists = reply->integer > 0;
    freeReplyObject(reply);
    return exists;
}

void RedisMgr::Close()
{
    if (_connect != nullptr) {
        redisFree(_connect);
        _connect = nullptr;
    }

    if (_con_pool) {
        _con_pool->Close();
    }
}
