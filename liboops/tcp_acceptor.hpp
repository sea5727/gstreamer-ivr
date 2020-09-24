// #ifndef __TCP_ACCEPTOR_HPP__
// #define __TCP_ACCEPTOR_HPP__

// #include <unistd.h>
// #include <functional>


// namespace jdin
// {
//     namespace oop
//     {
//         namespace tcp
//         {
//             class TcpAcceptor : public jdin::oop::event::Event
//             {
//             private:
//                 jdin::oop::log::Logger      _logger;
//                 struct sockaddr_in          _server_addr;
//             public:
//                 std::string                 _ip;
//                 uint16_t                    _port;

//                 std::function<void( TcpAcceptor *   tcp_acceptor, 
//                                     std::string     remote_ip, 
//                                     uint16_t        remote_port, 
//                                     std::string     local_ip, 
//                                     uint16_t        local_port, 
//                                     int             socket_fd)>       _cb;
//             public:
//                 TcpAcceptor(jdin::oop::log::Logger logger = jdin::oop::log::Logger());
//                 TcpAcceptor(std::string ip, uint16_t port, jdin::oop::log::Logger logger = jdin::oop::log::Logger());
//                 ~TcpAcceptor();

//                 int start();
//                 int make_fd(std::string ip, uint16_t port);
//                 int add_to(jdin::oop::event::EventLoop * loop);
//                 int del_from(jdin::oop::event::EventLoop * loop);
//                 int pop(struct epoll_event ev);
//                 int clear();
//                 inline TcpAcceptor * set_callback(std::function<void(   TcpAcceptor *   tcp_acceptor, 
//                                                                         std::string     remote_ip, 
//                                                                         uint16_t        remote_port, 
//                                                                         std::string     local_ip, 
//                                                                         uint16_t        local_port, 
//                                                                         int             socket_fd)>  cb) 
//                 { 
//                     _cb = cb; 
//                     return this;
//                 }
//                 struct epoll_event get_event_st()
//                 {
//                     struct epoll_event ev;
//                     memset(&ev, 0x00, sizeof(ev));
//                     ev.data.ptr = this;
//                     ev.events = EPOLLIN;
//                     return ev;
//                 }
//             };         
//         }
//     }
// }


// #endif