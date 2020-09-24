#ifndef __MS_HTTP_HANDLER_HPP__
#define __MS_HTTP_HANDLER_HPP__

#include <cpprest/http_listener.h>
#include <cpprest/json.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "ms_core.hpp"
#include "ms_thread_pool.hpp"
#include "ms_message_route.hpp"
#include "ms_core_gst_builder.hpp"

class HttpInterface
{
private:
    std::string _name;
    web::uri addr;
    web::http::experimental::listener::http_listener m_listener;
public:
    HttpInterface() = default;
    HttpInterface(std::string address) 
        : _name("")
        , addr(web::uri_builder(address).to_uri())
        , m_listener(addr.to_string())
    {


        m_listener.support(web::http::methods::GET, std::bind(&HttpInterface::handle_get, this, std::placeholders::_1));
        m_listener.support(web::http::methods::PUT, std::bind(&HttpInterface::handle_put, this, std::placeholders::_1));
        m_listener.support(web::http::methods::POST, std::bind(&HttpInterface::handle_post, this, std::placeholders::_1));
        m_listener.support(web::http::methods::DEL, std::bind(&HttpInterface::handle_delete, this, std::placeholders::_1));
        
    }
    ~HttpInterface()
    {
        std::cout << "~HttpInterface start\n";
    }
    web::uri get_addr() { return addr; }
    pplx::task<void> open()
    {
        return m_listener.open();
    }

    void set_name(std::string name)
    {
        _name = name;
    }

    void handle_get(web::http::http_request message)
    {
        std::cout << "hello get\n";
        std::cout << _name << ": hello get\n";

        auto path = m_listener.uri().path();
        auto scheme = m_listener.uri().scheme();
        auto user_info = m_listener.uri().user_info();
        auto host = m_listener.uri().host();
        auto port = m_listener.uri().port();
        auto query = m_listener.uri().query();
        auto fragment = m_listener.uri().fragment();
        
        std::cout << "path : " << path << std::endl;
        std::cout << "scheme : " <<scheme << std::endl;
        std::cout << "user_info : " <<user_info << std::endl;
        std::cout << "host : " <<host << std::endl;
        std::cout << "port : " <<port << std::endl;
        std::cout << "query : " <<query << std::endl;
        std::cout << "fragment : " <<fragment << std::endl;

        auto relativePath = web::uri::decode(message.relative_uri().path());
        auto request_path = web::uri::split_path(relativePath);        

        if(!request_path.empty())
        {
            for(auto &p : request_path)
            {
                std::cout << "request_path:" << p << std::endl;
            }
        }

        const auto body = message.content_ready().get().extract_utf8string(true).get();

        std::cout << "body : " << body << std::endl;
 
        auto response = web::json::value::object();
        response["version"] = web::json::value::string("0.1.1");
        response["status"] = web::json::value::string("ready!");
        message.reply(web::http::status_codes::OK, response);
    }   
    void handle_put(web::http::http_request message)
    {
        std::cout << "hello put\n";

        auto path = m_listener.uri().path();
        auto scheme = m_listener.uri().scheme();
        auto user_info = m_listener.uri().user_info();
        auto host = m_listener.uri().host();
        auto port = m_listener.uri().port();
        auto query = m_listener.uri().query();
        auto fragment = m_listener.uri().fragment();
        
        std::cout << "path : " << path << std::endl;
        std::cout << "scheme : " <<scheme << std::endl;
        std::cout << "user_info : " <<user_info << std::endl;
        std::cout << "host : " <<host << std::endl;
        std::cout << "port : " <<port << std::endl;
        std::cout << "query : " <<query << std::endl;
        std::cout << "fragment : " <<fragment << std::endl;

        auto relativePath = web::uri::decode(message.relative_uri().path());
        auto request_path = web::uri::split_path(relativePath);        

        if(!request_path.empty())
        {
            for(auto &p : request_path)
            {
                std::cout << "request_path:" << p << std::endl;
            }
        }

        const auto body = message.content_ready().get().extract_utf8string(true).get();

        std::cout << "body : " << body << std::endl;
 
        auto response = web::json::value::object();
        response["version"] = web::json::value::string("0.1.1");
        response["status"] = web::json::value::string("ready!");
        message.reply(web::http::status_codes::OK, response);


        std::cout << "start \n";

        rapidjson::Document doc;
        doc.Parse(body.c_str());

        std::cout << "start type\n";
        if(doc.IsNull())
        {
            std::cout << "No Body\n";
        }
        if(!doc.IsNull() && doc.HasMember("type"))
        {
            std::cout << "start in type\n";
            const auto type = doc["type"].GetString();
            if(strcmp(type, "command") == 0)
            {
                std::cout << "start command\n";


                if(doc.HasMember("data"))
                {
                    std::cout << "start data\n";
                    auto data = doc["data"].GetObject();
                    auto method = data.HasMember("method") ? data["method"].GetString() : "";
                    if(strcmp(method, "crcx_pipeline") == 0)
                    {
                        // proc pipeline 
                        uint16_t        local_rtp_port;
                        std::string     remote_rtp_ip;
                        uint16_t        remote_rtp_port;
                        std::string     remote_rtcp_ip;
                        uint16_t        remote_rtcp_port;
                        uint16_t        audio_payload;

                        std::string     remote_codec;
                        std::string     local_codec;

                        if(data.HasMember("rtp"))
                        {
                            std::cout << "rtp \n";
                            auto rtp = data["rtp"].GetObject();
                            local_rtp_port =        rtp.HasMember("local_rtp_port")     ? rtp["local_rtp_port"].GetInt()    : 0;
                            remote_rtp_ip.assign(   rtp.HasMember("remote_rtp_ip")      ? rtp["remote_rtp_ip"].GetString()  : "");
                            remote_rtp_port =       rtp.HasMember("remote_rtp_port")    ? rtp["remote_rtp_port"].GetInt()   : 0;
                            audio_payload =         rtp.HasMember("audio_payload")      ? rtp["audio_payload"].GetInt()     : 0;
                        }

                        if(data.HasMember("rtcp"))
                        {
                            std::cout << "rtcp \n";
                            auto rtcp = data["rtcp"].GetObject();
                            remote_rtcp_ip.assign(  rtcp.HasMember("remote_rtcp_ip")     ? rtcp["remote_rtcp_ip"].GetString() : "");
                            remote_rtcp_port =      rtcp.HasMember("remote_rtcp_port")   ? rtcp["remote_rtcp_port"].GetInt()  : 0;
                        }
                        if(data.HasMember("codec"))
                        {
                            std::cout << "codec \n";
                            auto codec = data["codec"].GetObject();
                            remote_codec.assign(  codec.HasMember("remote_codec")     ? codec["remote_codec"].GetString() : "");
                            local_codec.assign(  codec.HasMember("local_codec")     ? codec["local_codec"].GetString() : "");
                        }

                        using MsCore::MsCore;
                        if(auto thread_pool = MsCore::get_thread_pool())
                        {
                            if(auto io_service = thread_pool->get_io_service(0) )
                            {
                                (*io_service).post([=]{
                                    std::cout << "io_sevice start\n";
                                    MsCore::create()
                                        .make_resource()
                                        .set_remote_rtp_ip(remote_rtp_ip)
                                        .set_remote_rtp_port(remote_rtp_port)
                                        .set_audio_payload(audio_payload)
                                        .proc_crcx_pipeline();
                                });
                            }
                        }
                    }
                    else if(strcmp(method, "rqnt_tone") == 0)
                    {
                        auto index = data.HasMember("index") ? data["index"].GetInt() : -1;
                        auto wave = data.HasMember("wave") ? data["wave"].GetInt() : -1;
                        using MsCore::MsCore;
                        if(auto thread_pool = MsCore::get_thread_pool())
                        {
                            if(auto io_service = thread_pool->get_io_service(0) )
                            {
                                (*io_service).post([=]{
                                    std::cout << "io_sevice start\n";
                                    MsCore::create()
                                        .get_resource(index)
                                        .proc_rqnt_tone(wave);
                                });
                            }
                        }
                    }
                

                }
            }
        }

        std::cout << "doc end\n";


    }
    void handle_post(web::http::http_request message)
    {
        std::cout << "hello post\n";
        message.reply(web::http::status_codes::NotFound);
    }
    void handle_delete(web::http::http_request message)
    {
        std::cout << "hello del\n";
        message.reply(web::http::status_codes::NotFound);
    }

};

#endif

