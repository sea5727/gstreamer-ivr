#pragma once

#include "ms_include.hpp"

namespace MediaServer
{

    template<typename TcpHandler>
    class TcpListener
    {
    private:
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::io_service & io_service;
        uint16_t port;
    public: 
        // TcpGenericServer() = default;
        TcpListener(boost::asio::io_service & io_service, uint16_t port)
            : io_service{io_service}
            , acceptor{io_service}
            , port{port}
        { }

        void run() {
            listen();
        }    

        void
        listen(){
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();

            auto handler = std::make_shared<TcpHandler>(io_service);
            
            acceptor.async_accept( handler->socket(), 
                                    [=](const boost::system::error_code & error) {
                                        handle_new_connection(handler, error);
                                    });

            auto timer_test_1 = std::make_shared<boost::asio::deadline_timer>(io_service);
            timer_test_1->expires_from_now(boost::posix_time::seconds(1));
            timer_test_1->async_wait([timer_test_1](const boost::system::error_code & error){
                std::cout << "timer_test_1 start" << std::endl;
            });

            auto timer_test_2 = std::make_shared<boost::asio::deadline_timer>(io_service);
            timer_test_2->expires_from_now(boost::posix_time::seconds(2));
            timer_test_2->async_wait([timer_test_2](const boost::system::error_code & error){
                std::cout << "timer_test_2 start" << std::endl;
            });

        }


    private:
        void 
        handle_new_connection(std::shared_ptr<TcpHandler> handler, const boost::system::error_code & error) {
            if(error) {
                throw std::runtime_error("TODO handle_new_connection error ");
            }
            handler->start();
            auto new_handler = std::make_shared<TcpHandler>(io_service);

            std::cout << "try new accept" << std::endl;
            acceptor.async_accept( new_handler->socket(), 
                                    [=](const boost::system::error_code & error_code) {
                                        handle_new_connection(new_handler, error_code);
                                    });
        }

        
    };
}