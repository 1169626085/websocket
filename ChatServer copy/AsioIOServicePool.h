#pragma once
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIoServicePool:public Singleton<AsioIoServicePool>
{
    friend class Singleton<AsioIoServicePool>;
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::executor_work_guard<IOService::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;
    ~AsioIoServicePool();
    AsioIoServicePool(const AsioIoServicePool&) = delete;
    AsioIoServicePool& operator=(const AsioIoServicePool&) = delete;
    boost::asio::io_context& GetIOService();
    void Stop();
private:
    AsioIoServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _nextIOService;
};
