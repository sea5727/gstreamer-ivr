#ifndef __MS_HMP_IF_HPP__
#define __MS_HMP_IF_HPP__


#include <iostream>
#include <stdio.h>
#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

using ptr_io_service = std::shared_ptr<boost::asio::io_service>;

template<typename TcpHandler>
class TcpGenericServer
{
private:
    int _worker_count;
    std::vector<std::thread>                        _workers;
    std::vector<boost::asio::io_service::work>      _work_container; // io_service::work 는 io_service가 작업이 없더라도 종료되지 않게 해주는 객체이다. local로 선언시 raii 로 인해 사라지므로 vector로 저장
    std::vector<ptr_io_service>                     _io_container; // boost의 io_serivce는 noncopyable 이다. pointer로써 저장해야한다.
    size_t                                          _next_io_service;
public: 
    TcpGenericServer() = default;
    TcpGenericServer(int worker_count)
        : _worker_count(worker_count)
        , _work_container()
        , _io_container()
        , _next_io_service(0)
    {
        for(int i = 0 ; i < _worker_count ; ++i)
        {
            auto io_service = std::make_shared<boost::asio::io_service>(1);
            _work_container.emplace_back(boost::asio::io_service::work(*io_service));
            _io_container.push_back(io_service);
        }
    }


    void run()
    {
        for(auto io_service : _io_container)
        {
            _workers.emplace_back([io_service]{
                io_service->run();
            });
        }
    }    

    ptr_io_service get_io_service() 
    {
        // tcp accept 스레드는 1개이므로 lock 동작은 필요 없을것으로 판단.
        // 만약 tcp accept thread가 늘어날경우 락을 추가해야함.
        auto io_service = _io_container[_next_io_service];
        ++_next_io_service;
        if (_next_io_service == _io_container.size())
            _next_io_service = 0;
        return io_service;
    }

    std::vector<ptr_io_service> & get_route_container()
    {
        return _io_container;
    }

    void tcp_listen(uint16_t port)
    {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
        auto io_service = get_io_service();
        auto acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service);
        acceptor->open(endpoint.protocol());
        acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor->bind(endpoint);
        acceptor->listen();

        auto handler = std::make_shared<TcpHandler>(io_service);
        
        acceptor->async_accept( handler->socket(), 
                                [=](const boost::system::error_code & error_code) {
                                    handle_new_connection(acceptor, handler, error_code);
                                });
    }
private:
    void handle_new_connection( std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor, 
                                std::shared_ptr<TcpHandler>                     handler, 
                                const boost::system::error_code &               error_code)
    {
        if(error_code)
        {
            std::cerr << "handler_new_connection error..: " << error_code.message() << std::endl;
        }
        else
        {
            std::cout << "connection" << std::endl;
            handler->start();
        }
        auto io_service = get_io_service();
        auto new_handler = std::make_shared<TcpHandler> (io_service);

        acceptor->async_accept( new_handler->socket(), 
                                [=](boost::system::error_code error_code) {
                                    handle_new_connection(acceptor, new_handler, error_code);
                                });
        
    }
};

#endif