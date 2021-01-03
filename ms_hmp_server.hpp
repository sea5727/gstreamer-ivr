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

        std::vector<web::json::value> tool_req_list;
        
    public:
        HmpServer(boost::asio::io_service & io_service)
            : sock{io_service}
            , need{0}
            , HANDSHAKE{"surfapi"}
            , LEN_HANDSHAKE{HANDSHAKE.length()}
            // , tool_req_list{10000}
            {}

        void
        start(){
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
            std::cout << "async_read_handshake end" << std::endl;
        }
        void 
        read_handshake_done(
            std::size_t read_size) {

            auto data = std::string(buffer, read_size);

            if(data == HANDSHAKE) {
                boost::asio::async_write(sock, 
                                        boost::asio::buffer(HANDSHAKE, LEN_HANDSHAKE),
                                        [me=shared_from_this()](const boost::system::error_code & error, size_t len){
                                            if(error){
                                                throw std::runtime_error("TODO " + error.message());
                                                return;
                                            }
                                            // std::cout << "write success len : " << len << std::endl;
                                        });
            }
        }
        void
        async_read_length(){
            boost::asio::async_read(
                sock, boost::asio::buffer(&need, sizeof(need)), boost::asio::transfer_exactly(sizeof(need)),
                [me=shared_from_this(), this]
                (const boost::system::error_code & error, std::size_t read_size){
                    if(error){
                        proc_error(error);
                        return;
                    }
                    std::cout << "read need : " << need << std::endl;
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
                    
                    return;
                });
        }
        void
        proc(char * buffer, size_t len){
            auto surfapi = web::json::value::parse(buffer);

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

                response["tool_ans"]["req_type"] = req_type;
                response["tool_ans"]["req_id"] = req_id;
                response["tool_ans"]["tool_id"] = tool_id;

                if(req_type == web::json::value("set_config")){
                    auto data = surfapi["tool_req"]["data"];
                    auto tool_type = data["tool_type"];
                    if(tool_type == web::json::value("voice_fe_ip")){
                        auto dst_tool_ids = data["dst_tool_ids"][0];
                        if(dst_tool_ids.is_null()){ // first
                            
                            print_data(data);

                            MediaServer::HmpParser::Parse(surfapi);
                            MediaServer::HmpParser::MakeReceiver(surfapi);
                            MediaServer::HmpParser::SetStatePlay(surfapi);

                            tool_req_list[n_tool_id] = surfapi;

                            response["tool_ans"]["data"]["error_code"] = web::json::value::number(0);
                            response["tool_ans"]["data"]["description"] = web::json::value::string("OK");
                        }
                        else{ // second
                            
                            std::cout << data["dst_tool_ids"][0] << std::endl;
                            auto dst_tool_ids = data["dst_tool_ids"][0].as_integer() ;


                            //     MediaProcess::Commander::create_core(n_tool_id);
                            // MediaProcess::Commander::set_hmp_background_source(n_tool_id, data);
                            // MediaProcess::Commander::make_hmp_background_source(n_tool_id);
                            // MediaProcess::Commander::link_hmp_sink_to_source(n_tool_id, dst_tool_ids);
                            // MediaProcess::Commander::play(n_tool_id);
                            tool_req_list[n_tool_id] = surfapi;
                            response["tool_ans"]["data"]["error_code"] = web::json::value::number(0);
                            response["tool_ans"]["data"]["description"] = web::json::value::string("OK");
                        }
                    }
                   else if(tool_type == web::json::value("file_reader")){ // third
                        
                        data["events"][0]["type"];
                        data["events"][0]["enabled"];
                        data["audio_enabled"];
                        auto audio_dst_tool_ids = data["audio_dst_tool_ids"][0].as_integer();
                        data["video_enabled"];
                        data["video_dst_tool_ids"][0];
                        auto dst_set_config = tool_req_list[audio_dst_tool_ids];
                        auto dst_tool_ids = dst_set_config["tool_req"]["data"]["dst_tool_ids"][0].as_integer();


            //             // MediaProcess::Commander::create_core(n_tool_id);
            //             // MediaProcess::Commander::set_hmp_file_source(n_tool_id, data);
            //             // MediaProcess::Commander::make_hmp_file_source(n_tool_id);
            //             // MediaProcess::Commander::link_hmp_sink_to_source(n_tool_id, dst_tool_ids);

            //             // std::cout << "tool_req_list: " << tool_req_list[n_tool_id].is_null() << std::endl;
            //             // tool_req_list[n_tool_id] = body;
            //             // std::cout << "tool_req_list: " << tool_req_list[n_tool_id].is_null() << std::endl;
                        response["tool_ans"]["data"]["error_code"] = web::json::value::number(0);
                        response["tool_ans"]["data"]["description"] = web::json::value::string("OK");
                    }
                }
                else if(req_type == web::json::value("command")){
                    auto data = surfapi["tool_req"]["data"];
                    if(data["cmd_type"] == web::json::value("play_list_clear")){
            //             // MediaProcess::Commander::clear_file_list(n_tool_id);
            //             // // MsCommander::clear_filesrc_pipeline(n_tool_id);
                    }
                    if(data["cmd_type"] == web::json::value("play_list_append")){
                        std::vector<std::string> filelist;
                        auto files = data["files"].as_array();
                        for(auto & file : files){
                            auto name = file["name"].as_string();
                            filelist.push_back(name);
                        }
                        // MediaProcess::Commander::append_file_list(n_tool_id, filelist);
                        // // MsCommander::append_filesrc_pipeline(n_tool_id, filelist);

                    }
                    if(data["cmd_type"] == web::json::value("play")){
                        // auto set_config = tool_req_list[n_tool_id];
                        // auto audio_dst_tool_ids = set_config["tool_req"]["data"]["audio_dst_tool_ids"][0].as_integer();

                        // MediaProcess::Commander::ready(audio_dst_tool_ids);
                        // MediaProcess::Commander::play_file_list(n_tool_id);
                        // // MsCommander::ready_media_core(audio_dst_tool_ids);
                        // // MsCommander::play_filesrc_pipeline(n_tool_id);
                    }

                }

                else if(req_type == web::json::value("remove")){
                    std::cout << "this is remove!! core_index:" << n_tool_id << "\n";
                    // MediaProcess::Commander::remove(n_tool_id);
                    std::cout << "remove end...\n";
                    // std::cout << "tool_req_list: " << tool_req_list[n_tool_id].is_null() << std::endl;
                    // tool_req_list[n_tool_id] = web::json::value::null();
                    // std::cout << "tool_req_list: " << tool_req_list[n_tool_id].is_null() << std::endl;
                }
            }
            SendResponse:
            return;
        }
        void
        print_data(web::json::value & data){
            std::cout << data["backend_tool_id"] << std::endl;
            std::cout << data["input_from_RTP"] << std::endl;
            std::cout << data["output_to_RTP"] << std::endl;
            std::cout << data["decoder"]["type"] << std::endl;
            std::cout << data["encoder"]["type"] << std::endl;
            std::cout << data["RTP"]["local_udp_port"] << std::endl;
            std::cout << data["RTP"]["remote_udp_port"] << std::endl;
            std::cout << data["RTP"]["remote_ip"] << std::endl;
            std::cout << data["RTP"]["in_payload_type"] << std::endl;
            std::cout << data["RTP"]["out_payload_type"] << std::endl;
            std::cout << data["RTP"]["dtmf_out_payload_type"] << std::endl;
            std::cout << data["RTP"]["dtmf_in_payload_type"] << std::endl;
            std::cout << data["events"][0]["type"] << std::endl;
            std::cout << data["events"][0]["enabled"] << std::endl;
            std::cout << data["EVD"]["decoder_side"]["enabled"] << std::endl;
            std::cout << data["EVD"]["decoder_side"]["events"][0] << std::endl;
            std::cout << data["EVD"]["decoder_side"]["tone_suppression"] << std::endl;
            std::cout << data["EVG"]["enabled"] << std::endl;
        }


    };
}