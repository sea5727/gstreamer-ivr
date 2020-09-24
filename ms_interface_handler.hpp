#ifndef __MS_INTERFACE_HANDLER_HPP__
#define __MS_INTERFACE_HANDLER_HPP__

#include "boost/asio/ip/tcp.hpp"

class InterfaceHandler
{
public:
    virtual void start() = 0;
    virtual boost::asio::ip::tcp::socket & socket() = 0;
};


#endif