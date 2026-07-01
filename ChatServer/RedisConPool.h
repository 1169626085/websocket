#pragma once
#include "const.h"
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>

#if __has_include(<hiredis/hiredis.h>)
#include <hiredis/hiredis.h>
#elif __has_include(<hiredis.h>)
#include <hiredis.h>
#else
struct redisContext;
struct redisReply;
#endif

class RedisConPool {
public:
    RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
        : poolSize_(poolSize), host_(host == nullptr ? "" : host), port_(port), b_stop_(false)
    {
        for (size_t i = 0; i < poolSize_; ++i) {
            auto* context = redisConnect(host, port);
            if (context == nullptr || context->err != 0) {
                if (context != nullptr) {
                    redisFree(context);
                }
                continue;
            }

            if (pwd != nullptr && std::strlen(pwd) > 0) {
                auto* reply = static_cast<redisReply*>(redisCommand(context, "AUTH %s", pwd));
                if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                    if (reply != nullptr) {
                        freeReplyObject(reply);
                    }
                    redisFree(context);
                    continue;
                }
                freeReplyObject(reply);
            }

            connections_.push(context);
        }
    }

    ~RedisConPool()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!connections_.empty()) {
            redisFree(connections_.front());
            connections_.pop();
        }
    }

    redisContext* getConnection()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            return b_stop_ || !connections_.empty();
        });

        if (b_stop_) {
            return nullptr;
        }

        auto* context = connections_.front();
        connections_.pop();
        return context;
    }

    void returnConnection(redisContext* context)
    {
        if (context == nullptr) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (b_stop_) {
            redisFree(context);
            return;
        }
        connections_.push(context);
        cond_.notify_one();
    }

    void Close()
    {
        b_stop_ = true;
        cond_.notify_all();
    }

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    int port_;
    std::queue<redisContext*> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
