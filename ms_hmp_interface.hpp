#ifndef __MS_INTERFACE_TCP_HPP__
#define __MS_INTERFACE_TCP_HPP__

#include <iostream>
#include <string>
#include <sstream>
#include <istream>

#include <functional>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "ms_hmp_memory.hpp"
#include "ms_interface_handler.hpp"
#include "ms_hmp_service.hpp"
#include "ms_core_gst.hpp"

using std::string;
using read_function = std::function<void(const boost::system::error_code & error_code, std::size_t read_size)>;
using ptr_io_service = std::shared_ptr<boost::asio::io_service>;

_TOOL	_HMP_TOOLS[MAX_TOOL];


class HmpInterface : public std::enable_shared_from_this<HmpInterface>
{
private:
    char buffer[8092]; 
    ptr_io_service                      _io_service;
    boost::asio::ip::tcp::socket        _socket;
    boost::asio::io_service::strand     _strand;
    string                              _msg_handshake;
    uint32_t                            _next_length;
public:
    HmpInterface(ptr_io_service io_service)
        : _io_service(io_service)
        , _socket(*io_service)
        , _strand(*io_service)
    {
        _msg_handshake      = "surfapi";
        _next_length        = 0;        
    }

    ~HmpInterface() {}

    void start()
    {
        auto read_done_callback = std::bind(&HmpInterface::read_handshake_done, this, std::placeholders::_1, std::placeholders::_2);
        async_read(_msg_handshake.length(), read_done_callback);
    }

    boost::asio::ip::tcp::socket & socket()
    {
        return _socket;
    }


private:
    void async_read(size_t read_length, read_function callback)
    {
        std::cout << "transfer_exactly : " << read_length << std::endl;
        memset(buffer, 0x00, sizeof(buffer));
        boost::asio::async_read(_socket,
                                boost::asio::buffer(buffer),
                                boost::asio::transfer_exactly(read_length),
                                [me=shared_from_this(), callback](const boost::system::error_code & error_code, std::size_t read_size){
                                    std::cout << "async_read call!!" << std::endl;
                                    if(me->proc_read_error(error_code))
                                        return;
                                    me->async_read_done(error_code, read_size, callback);
                                });
    }

    void async_read_done(const boost::system::error_code & error_code, std::size_t read_size, read_function callback)
    {
        std::cout << "async_read_done... read_size : " << read_size << std::endl;
        callback(error_code, read_size);
    }

    bool proc_read_error(const boost::system::error_code & error_code)
    {
        if(!error_code) return false;

        std::cerr << "read_packet_done error.. : " << error_code.message() << std::endl;
        try
        {
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            _socket.close();
        }
        catch(std::exception const &e)
        {
            std::cerr << "catch.. e.what : " << e.what() << std::endl;
        }
        catch(...)
        {
            std::cerr << "catch oops.." << std::endl;
        }
        return true;

    }

    void read_handshake_done(const boost::system::error_code & error_code, std::size_t read_size)
    {
        string recv_data = string(buffer, read_size);

        if(recv_data == _msg_handshake)
        {
            boost::asio::async_write(_socket, 
                                    boost::asio::buffer(_msg_handshake, _msg_handshake.length()),
                                    boost::bind(&HmpInterface::handle_write, this, boost::asio::placeholders::error)); 
        }

        std::cout << buffer << std::endl;
        std::cout << "[DEBUG] sizeof(uint32_t):" << sizeof(_next_length) << std::endl;
        
        auto read_done_callback = std::bind(&HmpInterface::read_length_done, this, std::placeholders::_1, std::placeholders::_2);
        async_read(sizeof(_next_length), read_done_callback);
    }
    
    void read_length_done(const boost::system::error_code & error_code, std::size_t read_size)
    {
        memcpy(&_next_length, buffer, sizeof(uint32_t));

        std::cout << "recv _next_length : " << _next_length << std::endl;
        auto read_done_callback = std::bind(&HmpInterface::read_surfapi_done, this, std::placeholders::_1, std::placeholders::_2);
        async_read(_next_length, read_done_callback);

    }
    
    void read_surfapi_done(const boost::system::error_code & error_code, std::size_t read_size)
    {
        std::cout << "read_packet_done  read_size : " << read_size << std::endl;
        string surfapi = string(buffer, read_size);
        std::cout << "read_packet_done  : " << surfapi << std::endl;

        rapidjson::Document doc;
        doc.Parse(surfapi.c_str());

        rapidjson::StringBuffer s;
        if(doc.HasMember("connect"))
        {
            
            if(HmpService::get_connect_message(s))
            {
                const char * message = s.GetString();
                uint32_t message_size = s.GetSize();
                boost::asio::async_write(_socket, 
                                        boost::asio::buffer(&message_size, sizeof(uint32_t)),
                                        boost::bind(&HmpInterface::handle_write, this, boost::asio::placeholders::error)); 

                boost::asio::async_write(_socket, 
                                        boost::asio::buffer(message, message_size),
                                        boost::bind(&HmpInterface::handle_write, this, boost::asio::placeholders::error)); 
            }
        }
        else if(doc.HasMember("tool_req"))
        {
            auto tool_req = doc["tool_req"].GetObject();
            if(tool_req.HasMember("tool_id"))
            {
                auto tool_id = tool_req["tool_id"].GetInt();
                auto req_id = tool_req["req_id"].GetInt();
                auto req_type = tool_req["req_type"].GetString();

                
                if(strcmp(req_type, "set_config") == 0)
                {
                    if(!_HMP_TOOLS[tool_id].use)
                    {
                        _HMP_TOOLS[tool_id].req_id = req_id;
                        _HMP_TOOLS[tool_id].use = true;
                        
                        if(tool_req.HasMember("data"))
                        {
                            auto data = tool_req["data"].GetObject();
                            auto tool_type = data["tool_type"].GetString();
                            auto backend_tool_id = data["backend_tool_id"].GetInt();
                            auto input_from_RTP = data["input_from_RTP"].GetBool();
                            auto output_to_RTP = data["output_to_RTP"].GetBool();
                            std::cout << "[" << tool_id << "] tool_type : " << tool_type << std::endl;
                            std::cout << "backend_tool_id : " << backend_tool_id << std::endl;
                            std::cout << "input_from_RTP : " << input_from_RTP << std::endl;
                            std::cout << "output_to_RTP : " << output_to_RTP << std::endl;

                            _HMP_TOOLS[tool_id].data.tool_type = std::string(tool_type);
                            _HMP_TOOLS[tool_id].data.backend_tool_id = backend_tool_id;
                            _HMP_TOOLS[tool_id].data.input_from_RTP = input_from_RTP;
                            _HMP_TOOLS[tool_id].data.output_to_RTP = output_to_RTP;
                            

                            auto decoder_type = std::string("");
                            if(data.HasMember("decoder"))
                            {
                                auto decoder = data["decoder"].GetObject();
                                decoder_type = std::string(decoder["type"].GetString());
                                std::cout << "decoder_type : " << decoder_type << std::endl;
                            }
                            _HMP_TOOLS[tool_id].data.decoder.type = decoder_type;

                            auto encoder_type = std::string("");
                            if(data.HasMember("encoder"))
                            {
                                auto encoder = data["encoder"].GetObject();
                                encoder_type = std::string(encoder["type"].GetString());
                                std::cout << "encoder_type : " << encoder_type << std::endl;
                            }
                            _HMP_TOOLS[tool_id].data.encoder.type = encoder_type;

                            if(data.HasMember("RTP"))
                            {
                                auto RTP = data["RTP"].GetObject();
                                auto local_udp_port = RTP["local_udp_port"].GetInt();
                                auto remote_udp_port = RTP["remote_udp_port"].GetInt();
                                auto remote_ip = RTP["remote_ip"].GetString();
                                auto in_payload_type = RTP["in_payload_type"].GetInt();
                                auto out_payload_type = RTP["out_payload_type"].GetInt();
                                auto dtmf_out_payload_type = RTP["dtmf_out_payload_type"].GetInt();
                                auto dtmf_in_payload_type = RTP["dtmf_in_payload_type"].GetInt();
                                std::cout << "local_udp_port : " << local_udp_port << std::endl;
                                std::cout << "remote_udp_port : " << remote_udp_port << std::endl;
                                std::cout << "remote_ip : " << remote_ip << std::endl;
                                std::cout << "in_payload_type : " << in_payload_type << std::endl;
                                std::cout << "out_payload_type : " << out_payload_type << std::endl;
                                std::cout << "dtmf_out_payload_type : " << dtmf_out_payload_type << std::endl;
                                std::cout << "dtmf_in_payload_type : " << dtmf_in_payload_type << std::endl;

                                _HMP_TOOLS[tool_id].data.RTP.local_udp_port = local_udp_port;
                                _HMP_TOOLS[tool_id].data.RTP.remote_udp_port = remote_udp_port;
                                _HMP_TOOLS[tool_id].data.RTP.remote_ip = std::string(remote_ip);
                                _HMP_TOOLS[tool_id].data.RTP.in_payload_type = in_payload_type;
                                _HMP_TOOLS[tool_id].data.RTP.out_payload_type = out_payload_type;
                                _HMP_TOOLS[tool_id].data.RTP.dtmf_out_payload_type = dtmf_out_payload_type;
                                _HMP_TOOLS[tool_id].data.RTP.dtmf_in_payload_type = dtmf_in_payload_type;

                            }

                            if(data.HasMember("events"))
                            {
                                auto events = data["events"].GetArray();
                                for(int i = 0 ; i < events.Size() ; ++i)
                                {
                                    auto event = events[i].GetObject();
                                    auto type = event["type"].GetString();
                                    auto enabled = event["enabled"].GetBool();
                                    std::cout << "events[" << i << "] : type" << type << std::endl;
                                    std::cout << "events[" << i << "] : enabled" << enabled << std::endl;

                                    _HMP_TOOLS[tool_id].data.events.push_back({ std::string(type), enabled });
                                }
                                
                            }

                            if(data.HasMember("EVD"))
                            {
                                auto EVD = data["EVD"].GetObject();
                                if(EVD.HasMember("decoder_side"))
                                {
                                    auto decoder_side = EVD["decoder_side"].GetObject();
                                    auto enabled = decoder_side["enabled"].GetBool();
                                    std::cout << "EVD{decoder_side{enabled : " << enabled << std::endl;
                                    auto events = decoder_side["events"].GetArray();

                                    for(int i = 0 ; i < events.Size() ; ++i)
                                    {
                                        auto event = events[i].GetString();
                                        std::cout << "EVD{decoder_side{events[" << i << "] : event" << event << std::endl;
                                        _HMP_TOOLS[tool_id].data.EVD.decoder_side.events.push_back(std::string(event));
                                    }
                                    auto tone_suppression = decoder_side["tone_suppression"].GetString();
                                    std::cout << "EVD{decoder_side{tone_suppression : " << tone_suppression << std::endl;

                                    _HMP_TOOLS[tool_id].data.EVD.decoder_side.enabled = enabled;
                                    _HMP_TOOLS[tool_id].data.EVD.decoder_side.tone_suppression = std::string(tone_suppression);
                                    
                                }
                            }

                            if(data.HasMember("EVG"))
                            {
                                auto EVG = data["EVG"].GetObject();
                                auto enabled = EVG["enabled"].GetBool();
                                std::cout << "EVG{{enabled : " << enabled << std::endl;
                                _HMP_TOOLS[tool_id].data.EVG.enabled = enabled;

                            }
                        }
                    }

                    // if(_HMP_TOOLS[tool_id].data.tool_type.compare("file_reader") == 0)
                    // {
                    //     auto io_service = _message_router->get_route_io(MessageRouter::ROUTING_TABLE::CORE, tool_id);
                    //     if(io_service != nullptr)
                    //     {
                    //         (*io_service).post([=]{
                    //             std::cout << _HMP_TOOLS[tool_id].data.RTP.remote_ip << std::endl;
                    //             std::cout << _HMP_TOOLS[tool_id].data.RTP.remote_udp_port << std::endl;
                    //         });
                    //     }
                    //     else
                    //     {
                    //         std::cout << "io_service is nullptr" << std::endl;
                    //     }
                    // }



                    HmpService::ERROR_CODE error = HmpService::ERROR_CODE::OK;
                    if(HmpService::get_tool_ans_message(s, tool_id, req_id, req_type, error, error.to_desc().c_str()))
                    {
                        const char * message = s.GetString();
                        uint32_t message_size = s.GetSize();
                        boost::asio::async_write(_socket, 
                                                boost::asio::buffer(&message_size, sizeof(uint32_t)),
                                                boost::bind(&HmpInterface::handle_write, this, boost::asio::placeholders::error)); 
                        boost::asio::async_write(_socket, 
                                                boost::asio::buffer(message, message_size),
                                                boost::bind(&HmpInterface::handle_write, this, boost::asio::placeholders::error)); 
                    }

                }
                else if(strcmp(req_type, "command") == 0)
                {
                    if(!_HMP_TOOLS[tool_id].use)
                    {
                        HmpService::ERROR_CODE error = HmpService::ERROR_CODE::TOOL_DOES_NOT_EXIST;
                        if(HmpService::get_tool_ans_message(s, tool_id, req_id, req_type, error, error.to_desc().c_str()))
                        {
                            const char * message = s.GetString();
                            uint32_t message_size = s.GetSize();
                            boost::asio::async_write(_socket, 
                                                    boost::asio::buffer(&message_size, sizeof(uint32_t)),
                                                    boost::bind(&HmpInterface::handle_write, this, boost::asio::placeholders::error)); 
                            boost::asio::async_write(_socket, 
                                                    boost::asio::buffer(message, message_size),
                                                    boost::bind(&HmpInterface::handle_write, this, boost::asio::placeholders::error)); 
                        }

                    }
                    else
                    {

                    }
                }
                else if(strcmp(req_type, "remove") == 0)
                {

                }
                else
                {
                    
                }





            }
            


            
        }

        auto read_done_callback = std::bind(&HmpInterface::read_length_done, this, std::placeholders::_1, std::placeholders::_2);
        async_read(sizeof(_next_length), read_done_callback);
    }

    void handle_write(const boost::system::error_code& error){
        if(error){
            std::cerr << "error occur : " << error.message() << std::endl; 
            return;
        }
        std::cout << "send packet  : " << std::endl;
    }


};


#endif