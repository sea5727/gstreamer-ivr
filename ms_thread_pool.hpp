#pragma once

#include "ms_singleton.hpp"

#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <boost/asio.hpp>



class MsThreadPool// : public Singleton<MsThreadPool>
{
private:
    std::atomic<int> _round_robin_index;
    size_t _thread_count;
    size_t _io_service_size;
    std::vector<std::shared_ptr<boost::asio::io_service::work>> _work_container;
    std::vector<std::shared_ptr<boost::asio::io_service>> _io_service_container;
    std::vector<std::shared_ptr<std::thread>>             _thread_container;

public:
    MsThreadPool() = default;
    explicit MsThreadPool(size_t pool_size, size_t thread_count)
        : _round_robin_index(0)
        , _thread_count(thread_count)
        , _io_service_size(pool_size)
        , _work_container(0)
        , _io_service_container(0)
        , _thread_container(0)
    {
        if (pool_size == 0)
            throw std::runtime_error("io_service_pool size is 0");
        
        if(pool_size >= thread_count)
            _thread_count = pool_size;
    }
    MsThreadPool & init(size_t pool_size, size_t thread_count)
    {
        if (pool_size == 0)
            throw std::runtime_error("io_service_pool size is 0");
        
        _io_service_size = pool_size;
        _thread_count = thread_count;

        if(pool_size >= thread_count)
            _thread_count = pool_size;
        return *this;
    }
    MsThreadPool & start()
    {
        if (_io_service_size == 0)
            throw std::runtime_error("io_service_pool size is 0");

        std::cout << "thread_pool start\n";
        _io_service_container.clear();
        _thread_container.clear();

        for (size_t i = 0 ; i < _io_service_size ; ++i)
        {
            _io_service_container.emplace_back(std::make_shared<boost::asio::io_service>(1));
            _work_container.emplace_back(std::make_shared<boost::asio::io_service::work>(*_io_service_container[i]));
        }
        
        for (size_t i = 0, io_size = _io_service_size; i < _thread_count ; ++i)
        {
            _thread_container.emplace_back(std::make_shared<std::thread>([=](){
                _io_service_container[i % io_size]->run();
            }));
        }
        std::cout << "thread_pool end\n";
        return *this;
    }

    std::shared_ptr<boost::asio::io_service> get_io_service(int key = -1)
    {
        int index = key % _io_service_size;
        if(key == -1)
            index = (_round_robin_index++) % _round_robin_index;

        if(_round_robin_index >= _round_robin_index)
            _round_robin_index - _round_robin_index;

        return _io_service_container[index];
    }
};