#ifndef __MS_CORE_HPP__
#define __MS_CORE_HPP__



#include "ms_core_gst.hpp"
#include "ms_core_gst_builder.hpp"
#include "ms_thread_pool.hpp"

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <utility>

#define MAX_GST_CORE             3000
#define MAX_POOL_SIZE            10
#define MAX_MSCORE_WORKER_THREAD 10
namespace MsCore
{
    class MsCore 
    {
    private:
        static std::vector<std::shared_ptr<GstCore>>    _gst_core_container;
        static std::shared_ptr<MsThreadPool>            _thread_pool;
        static std::atomic<int>                         _next_index;
        int                                             _current_index;
    public:
        MsCore() = default;

        static void init()
        {
            _next_index = 0;
            _gst_core_container.resize(MAX_GST_CORE);
            _thread_pool = std::make_shared<MsThreadPool>(MAX_POOL_SIZE, MAX_MSCORE_WORKER_THREAD);
            _thread_pool->start();
        }

        inline static std::shared_ptr<MsThreadPool> get_thread_pool()
        {
            return _thread_pool;
        }

        inline static MsCore create()
        {
            return MsCore();
        }

        inline GstCoreBuilder builder()
        {
            return GstCoreBuilder::create();
        }

        inline GstCoreBuilder make_resource()
        {
            _current_index = _next_index++;
            std::cout << "current index : " << _current_index << std::endl;
            if(_current_index >= MAX_GST_CORE)
            {
                _next_index = _next_index % MAX_GST_CORE;
                _current_index = 0;
            }

            auto gstreamer_core = GstCoreBuilder::create();

            _gst_core_container[_current_index] = gstreamer_core.get();
            return gstreamer_core;

        }

        inline GstCoreBuilder get_resource(int index)
        {
            std::cout << "get_resource index : " << index << std::endl;
            std::cout << "get_resource remote_rtp_ip : " << _gst_core_container[_current_index]->_remote_rtp_ip << std::endl;
            
            return GstCoreBuilder(_gst_core_container[_current_index]);
        }
    };
}

std::atomic<int>                                MsCore::MsCore::_next_index(0);
std::vector<std::shared_ptr<MsCore::GstCore>>   MsCore::MsCore::_gst_core_container(0);
std::shared_ptr<MsThreadPool>                   MsCore::MsCore::_thread_pool;
#endif