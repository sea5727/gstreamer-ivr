
// #include "tcp_acceptor.hpp"


// #include "event.hpp"
// #include "event_loop.hpp"
// #include "logger.hpp"
// #include "exception.hpp"
// #include "tcp_session.hpp"

// #include <unistd.h>
// #include <functional>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <string.h>


// jdin::oop::tcp::TcpAcceptor::TcpAcceptor(jdin::oop::log::Logger logger)
//     : jdin::oop::event::Event()
//     , _ip("")
//     , _port(0)
//     , _logger(logger)
// {
//     _logger(LOG_5, "[%s:%d] TcpAcceptor init start\n", 
//         __func__,
//         __LINE__);
// }


// jdin::oop::tcp::TcpAcceptor::TcpAcceptor(std::string ip, uint16_t port, jdin::oop::log::Logger logger)
//     : jdin::oop::event::Event()
//     , _ip(ip)
//     , _port(port)
//     , _logger(logger)
// {
//     _logger(LOG_5, "[%s:%d] TcpAcceptor init start\n", 
//         __func__,
//         __LINE__);
//     _event_fd = -1;
//     _loop = NULL;
//     _server_addr.sin_family = AF_INET;
//     if(_ip.empty())
//         _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     else
//         _server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // TODO BIND IP 할당 필요
//     _server_addr.sin_port = htons(_port);


//     int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if(socket_fd == -1)
//     {
//         throw jdin::oop::exception::RuntimeException("socket fail", strerror(errno) , __FILE__, __func__, __LINE__);
//     }

//     int opt_value = 1;
//     setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_value, sizeof(opt_value));

//     if(bind(socket_fd, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) == -1)
//     {
//         close(socket_fd);
//         throw jdin::oop::exception::RuntimeException(
//                     "bind fail" + ip + std::to_string(_port), 
//                     strerror(errno) , 
//                     __FILE__, 
//                     __func__, 
//                     __LINE__);
//     }
    
//     _event_fd = socket_fd;
//     _logger(LOG_2, "[%s:%d] TcpAcceptor create socket_fd:%d\n", 
//         __func__,
//         __LINE__,
//         _event_fd);
// }


// jdin::oop::tcp::TcpAcceptor::~TcpAcceptor()
// {

// }

// int
// jdin::oop::tcp::TcpAcceptor::make_fd(std::string ip, uint16_t port)
// {
//     _logger(LOG_5, "[%s:%d] TcpAcceptor make_fd\n", 
//         __func__,
//         __LINE__);
        
//     _ip = ip;
//     _port = port;

//     _server_addr.sin_family = AF_INET;
//     if(_ip.empty())
//         _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     else
//         _server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // TODO BIND IP 할당 필요
//     _server_addr.sin_port = htons(_port);


//     int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if(socket_fd == -1)
//     {
//         throw jdin::oop::exception::RuntimeException("socket fail", strerror(errno) , __FILE__, __func__, __LINE__);
//     }

//     int opt_value = 1;
//     setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_value, sizeof(opt_value));

//     if(bind(socket_fd, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) == -1)
//     {
//         close(socket_fd);
//         throw jdin::oop::exception::RuntimeException(
//                     "bind fail" + ip + std::to_string(_port), 
//                     strerror(errno) , 
//                     __FILE__, 
//                     __func__, 
//                     __LINE__);
//     }
    
//     _event_fd = socket_fd;
//     _logger(LOG_2, "[%s:%d] TcpAcceptor create socket_fd:%d\n", 
//         __func__,
//         __LINE__,
//         _event_fd);
// }

// int jdin::oop::tcp::TcpAcceptor::start()
// {
//     if(listen(_event_fd, 5) == -1)
//     {
//         close(_event_fd);
//         _event_fd = -1;
//         throw jdin::oop::exception::RuntimeException(
//                     "listen fail", 
//                     strerror(errno) , 
//                     __FILE__, 
//                     __func__, 
//                     __LINE__);
//     }
//     return 0;

// }

// int jdin::oop::tcp::TcpAcceptor::add_to(jdin::oop::event::EventLoop * loop)
// {
//     //TODO
//     return 0;
// }
// int jdin::oop::tcp::TcpAcceptor::del_from(jdin::oop::event::EventLoop * loop)
// {
//     //TODO
//     return 0;
// }

// int jdin::oop::tcp::TcpAcceptor::pop(struct epoll_event ev)
// {
//     struct sockaddr_in client_addr;
//     socklen_t len = sizeof(struct sockaddr_in);
//     int socket_fd = accept(_event_fd, (struct sockaddr *)&client_addr, &len);
//     if(socket_fd == -1)
//     {
//         throw jdin::oop::exception::RuntimeException("accept fail", strerror(errno), __FILE__, __func__, __LINE__);
//     }

//     char *ip = inet_ntoa(client_addr.sin_addr);
//     uint16_t port = htons(client_addr.sin_port);

//     _logger(LOG_4, "[%s:%d] tcp accept %s:%d \n", __func__, __LINE__, ip, port);


//     if(_cb != nullptr)
//         _cb(this, std::string(ip), port, _ip, _port, socket_fd);

//     return 0;
// }


// int jdin::oop::tcp::TcpAcceptor::clear()
// {
//     _logger(LOG_2, "[%s:%d] 1 TcpAcceptor clear.. name:%s fd:%d\n", 
//         __func__, 
//         __LINE__, 
//         _name.c_str(),
//         _event_fd);
//     if(_loop != nullptr)
//     {
//         _logger(LOG_5, "[%s:%d] 2 TcpAcceptor del_event.. loop:%s, name:%s, socket_fd:%d\n", 
//             __func__, 
//             __LINE__, 
//             _loop->get_name().c_str(),
//             _name.c_str(),
//             _event_fd);
//         _loop->del_event(this);
//         _loop = nullptr;
//     }
//     if(_event_fd > 0)
//     {
//         _logger(LOG_5, "[%s:%d] 3 close listen.. name:%s, socket fd:%d\n", 
//             __func__, 
//             __LINE__, 
//             _name.c_str(),
//             _event_fd);
//         close(_event_fd);
//         _event_fd = -1;
//     }
//     return 0;
// }