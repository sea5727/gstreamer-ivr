#pragma once

#include "ms_include.hpp"

namespace MediaServer
{
    class HmpServer : public std::enable_shared_from_this<HmpServer> {
        
        boost::asio::ip::tcp::socket sock;
        char buffer[8096];
        uint32_t need;
        const std::string HANDSHAKE;
        const size_t LEN_HANDSHAKE;
        Manager & pm;
        HmpManager & hmp_pm;

        std::array<web::json::value, 10000> & tool_req_list;
        
    public:
        HmpServer(boost::asio::io_service & io_service)
            : sock{io_service}
            , need{0}
            , HANDSHAKE{"surfapi"}
            , LEN_HANDSHAKE{HANDSHAKE.length()}
            , pm{Manager::getInstance()}
            , hmp_pm{HmpManager::getInstance()}
            , tool_req_list{hmp_pm.tool_req_list}
            {}

        void
        start(){
            
            pm.Logger().info("Client Accept");
            async_read_handshake();
        }

        boost::asio::ip::tcp::socket & 
        socket() {
            return sock;
        }

        void
        proc_error(
            const boost::system::error_code & error_code) {

            try {
                sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                sock.close();
            }
            catch(std::exception const &e) {
                std::cerr << "catch.. e.what : " << e.what() << std::endl;
            }
            catch(...) {
                std::cerr << "catch oops.." << std::endl;
            }
        }

        void 
        async_read_handshake() {
            boost::asio::async_read(
                sock, boost::asio::buffer(buffer), boost::asio::transfer_exactly(LEN_HANDSHAKE),
                [me=shared_from_this(), this]
                (const boost::system::error_code & error, std::size_t read_size){
                    if(error){
                        proc_error(error);
                        return;
                    }
                    
                    read_handshake_done(read_size);
                    async_read_length();
                    return;
                });
        }
        void 
        read_handshake_done(
            std::size_t read_size) {

            auto data = std::string(buffer, read_size);

            if(data == HANDSHAKE) {
                pm.Logger().debug("[Recv] Hand Shaekd Success");
                boost::asio::async_write(sock, 
                                        boost::asio::buffer(HANDSHAKE, LEN_HANDSHAKE),
                                        [me=shared_from_this(), this](const boost::system::error_code & error, size_t len){
                                            if(error){
                                                throw std::runtime_error("TODO " + error.message());
                                                return;
                                            }
                                            pm.Logger().debug("[Send] Hand Shaekd Success");
                                        });
            }
        }
        void
        async_read_length(){
            boost::asio::async_read(
                sock, boost::asio::buffer(buffer, sizeof(need)), boost::asio::transfer_exactly(sizeof(need)),
                [me=shared_from_this(), this]
                (const boost::system::error_code & error, std::size_t read_size){
                    if(error){
                        proc_error(error);
                        return;
                    }
                    memcpy(&need, buffer, sizeof(need));
                    pm.Logger().info("[Recv] need length : {}", need);
                    async_read_data(need);
                    return;
                });
        }
        void
        async_read_data(size_t len){
            boost::asio::async_read(
                sock, boost::asio::buffer(buffer), boost::asio::transfer_exactly(len),
                [me=shared_from_this(), this]
                (const boost::system::error_code & error, std::size_t read_size){
                    if(error){
                        proc_error(error);
                        return;
                    }
                    proc(buffer, read_size);
                    async_read_length();
                    return;
                });
        }
        void
        proc(char * buffer, size_t len){
            auto bufferlen = strlen(buffer);
            if(bufferlen < len)
                len = bufferlen;

            pm.Logger().debug("[Recv] data{} : {}", len, buffer);

            auto request = std::string{buffer, len};

            auto surfapi = web::json::value::parse(request);
            pm.Logger().debug("[Recv] parse success");

            auto response = web::json::value::object();

            if(surfapi["connect"].is_object()){
                response["connect"]["api_version"][0] = 1;
                response["connect"]["api_version"][1] = 2;
                goto SendResponse;
            }

            if(surfapi["sys_req"].is_object()){
                response["sys_ans"]["req_id"] = surfapi["sys_req"]["req_id"];
                response["sys_ans"]["req_type"] = surfapi["sys_req"]["req_type"];
                response["sys_ans"]["data"]["error_code"] = web::json::value::number(0);
                response["sys_ans"]["data"]["description"] = web::json::value::string("OK");
                goto SendResponse;
            }

            if(surfapi["tool_req"].is_object()){
                auto req_type = surfapi["tool_req"]["req_type"];
                auto req_id = surfapi["tool_req"]["req_id"];
                auto tool_id = surfapi["tool_req"]["tool_id"];

                int n_tool_id = tool_id.as_integer();
                pm.Logger().debug("[TOOLID] {}", n_tool_id);

                response["tool_ans"]["req_type"] = req_type;
                response["tool_ans"]["req_id"] = req_id;
                response["tool_ans"]["tool_id"] = tool_id;

                if(req_type == web::json::value("set_config")){
                    auto data = surfapi["tool_req"]["data"];
                    auto tool_type = data["tool_type"];
                    if(tool_type == web::json::value("voice_fe_ip")){
                        auto dst_tool_ids = data["dst_tool_ids"][0];
                        if(dst_tool_ids.is_null()){ // first
                            auto dst_tool_id = data["backend_tool_id"].as_integer();
                            pm.Logger().debug("[TOOLID] {} first start" , n_tool_id);
                            MediaServer::HmpParser::ParseForSender(surfapi, sock);
                            MediaServer::HmpParser::MakeRtpSender(surfapi);
                            MediaServer::HmpParser::SetStatePaused(n_tool_id);
                            
                            MediaServer::HmpParser::ParseForReceiver(surfapi, sock);
                            MediaServer::HmpParser::MakeRtpReceiver(surfapi);

                            MediaServer::HmpParser::SetStatePlay(n_tool_id);
                            MediaServer::HmpParser::SetStatePlay(dst_tool_id);
                            tool_req_list[n_tool_id] = surfapi;
                            response["tool_ans"]["data"]["error_code"] = web::json::value::number(0);
                            response["tool_ans"]["data"]["description"] = web::json::value::string("OK");
                        }
                        else{ // second
                            pm.Logger().debug("[TOOLID] {} second start" , n_tool_id);
                            std::cout << data["dst_tool_ids"][0] << std::endl;
                            auto dst_tool_ids = data["dst_tool_ids"][0].as_integer() ;
                            tool_req_list[n_tool_id] = tool_req_list[dst_tool_ids];
                            response["tool_ans"]["data"]["error_code"] = web::json::value::number(0);
                            response["tool_ans"]["data"]["description"] = web::json::value::string("OK");
                        }
                    }
                   else if(tool_type == web::json::value("file_reader")){ // third
                        pm.Logger().debug("[TOOLID] {} third start" , n_tool_id);
                        auto audio_dst_tool_ids = data["audio_dst_tool_ids"][0].as_integer();
                        tool_req_list[n_tool_id] = tool_req_list[audio_dst_tool_ids];
                        response["tool_ans"]["data"]["error_code"] = web::json::value::number(0);
                        response["tool_ans"]["data"]["description"] = web::json::value::string("OK");
                    }
                }
                else if(req_type == web::json::value("command")){
                    auto data = surfapi["tool_req"]["data"];
                    if(data["cmd_type"] == web::json::value("play_list_clear")){
                        auto sender_tool_id = tool_req_list[n_tool_id]["tool_req"]["tool_id"].as_integer();
                        std::cout << "send_tool_id : "<< sender_tool_id << std::endl;
                        MediaServer::HmpParser::ClearPlayList(sender_tool_id);
        
                    }
                    if(data["cmd_type"] == web::json::value("play_list_append")){
                        auto sender_tool_id = tool_req_list[n_tool_id]["tool_req"]["tool_id"].as_integer();
                        std::vector<std::string> filelist;
                        auto files = data["files"].as_array();
                        for(auto & file : files){
                            auto name = file["name"].as_string();
                            filelist.push_back(name);
                        }
                        MediaServer::HmpParser::AppandPlayList(sender_tool_id, filelist);
                    }
                    if(data["cmd_type"] == web::json::value("play")){
                        auto sender_tool_id = tool_req_list[n_tool_id]["tool_req"]["tool_id"].as_integer();
                        MediaServer::HmpParser::PlayFile(sender_tool_id);
                    }

                }

                else if(req_type == web::json::value("remove")){
                    MediaServer::HmpParser::PlayRemove(n_tool_id);
                    tool_req_list[n_tool_id] = web::json::value::null();
                }
            }
            SendResponse:

            
            auto rsp = response.serialize();
            
            uint32_t len_rsp = static_cast<uint32_t>(rsp.length());
            pm.Logger().debug("[Send] Response {} : {}", len_rsp, rsp);
            

            std::vector<boost::asio::const_buffer> buffers;
            buffers.push_back(boost::asio::buffer(&len_rsp,sizeof(len_rsp)));
            buffers.push_back(boost::asio::buffer(rsp, len_rsp));
            boost::asio::async_write(sock, 
                                    buffers,
                                    [me=shared_from_this(), this](const boost::system::error_code & error, size_t len){
                                        if(error){
                                            throw std::runtime_error("TODO " + error.message());
                                            return;
                                        }
                                        pm.Logger().debug("[Send] Response Success");
                                    });
            
            return;
        }



    };
}