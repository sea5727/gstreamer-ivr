#ifndef __OOP_EVENT_HPP__
#define __OOP_EVENT_HPP__

#include <sys/epoll.h>
#include <string.h>
#include <functional>
#include <string>


namespace oop
{
    namespace event
    {

        class EventLoop;
        class Event
        {
            int                     _event_fd;
            EventLoop *             _loop;
            std::string             _name;
        public:
            Event() 
                :  _event_fd(-1)
                , _loop(nullptr)
                , _name("Default")
            {
            } 
            virtual ~Event() {}

            virtual int add_to(EventLoop * loop) = 0;
            virtual int del_from(EventLoop * loop) = 0;
            virtual int pop(struct epoll_event ev) = 0;
            virtual int clear() = 0;

            inline Event * set_name(std::string name){ _name = name; return this;}
            inline const char * get_name(){ return _name.c_str(); }

            int get_event_fd() { return _event_fd; }
            virtual struct epoll_event get_event_st() = 0;
            

            
        };
    }
}


#endif