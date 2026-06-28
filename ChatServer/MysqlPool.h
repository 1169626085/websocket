#pragma once
#include "const.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>

class MysqlPool{
public:
    MysqlPool(const std::string& url,const std::string& user,const std::string& pass,const std::string& schema,int poolSize)
        :url_(url),user_(user),pass_(pass),schema_(schema),poolSize_(poolSize),b_stop_(false){
            try{
                for(int i=0;i<poolSize_;++i){
                    auto con = createConnection();
                    if (con) {
                        pool_.push(std::move(con));
                    }
                }
            }
            catch(sql::SQLException& e){
                std::cout<<"mysql pool init failed"<<std::endl;
            }
        }
    std::unique_ptr<sql::Connection> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!b_stop_) {
            if (pool_.empty()) {
                lock.unlock();
                auto new_con = createConnection();
                lock.lock();
                if (new_con) {
                    return new_con;
                }
                cond_.wait_for(lock, std::chrono::seconds(1));
                continue;
            }

            std::unique_ptr<sql::Connection> con(std::move(pool_.front()));
            pool_.pop();
            lock.unlock();

            if (checkConnection(con.get())) {
                return con;
            }
        }
        return nullptr;
    }
        void returnConnection(std::unique_ptr<sql::Connection> con) {
        if (!con) {
            return;
        }
        if (!checkConnection(con.get())) {
            return;
        }
        std::unique_lock<std::mutex> lock(mutex_);
        if (b_stop_) {
            return;
        }
        pool_.push(std::move(con));
        cond_.notify_one();
    }

    void Close() {
        b_stop_ = true;
        cond_.notify_all();
    }

    ~MysqlPool() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!pool_.empty()) {
            pool_.pop();
        }
    }
private:
    std::unique_ptr<sql::Connection> createConnection() {
        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect(url_, user_, pass_));
            con->setSchema(schema_);
            return con;
        }
        catch (sql::SQLException& e) {
            std::cout << "mysql create connection failed: " << e.what() << std::endl;
            return nullptr;
        }
    }

    bool checkConnection(sql::Connection* con) {
        if (con == nullptr) {
            return false;
        }
        try {
            if (con->isClosed()) {
                return false;
            }
            if (con->isValid()) {
                return true;
            }
            return con->reconnect();
        }
        catch (sql::SQLException& e) {
            std::cout << "mysql connection invalid: " << e.what() << std::endl;
            return false;
        }
    }

     std::string url_;
    std::string user_;
    std::string pass_;
    std::string schema_;
    int poolSize_;
    std::queue<std::unique_ptr<sql::Connection>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> b_stop_;

};
