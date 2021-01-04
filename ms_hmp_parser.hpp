#pragma once

#include "ms_include.hpp"

namespace MediaServer
{
    class HmpParser{
    public:
        HmpParser() = default;
        
        static
        void
        ParseForSender(web::json::value & surfapi){
            auto tool_id = surfapi["tool_req"]["tool_id"].as_integer();

            auto & core = GstreamerCore::CoreList.at(tool_id);
            if(!core){
                GstreamerCore::CoreList[tool_id] = std::make_shared<GstreamerCore>();
            }

            auto data = surfapi["tool_req"]["data"];
            core->ssrc = g_random_int();
            core->payload = data["RTP"]["in_payload_type"].as_integer();
            core->bindport = data["RTP"]["local_udp_port"].as_integer();
            core->remoteip = data["RTP"]["remote_ip"].as_string();
            core->port = data["RTP"]["remote_udp_port"].as_integer();
            core->codec = data["decoder"]["type"].as_string();

        }
        static
        void
        ParseForReceiver(web::json::value & surfapi){
            auto tool_id = surfapi["tool_req"]["tool_id"].as_integer();
            auto data = surfapi["tool_req"]["data"];
            
            auto dst_tool_id = data["backend_tool_id"].as_integer();
            auto & core = GstreamerCore::CoreList.at(dst_tool_id);
            if(!core){
                GstreamerCore::CoreList[dst_tool_id] = std::make_shared<GstreamerCore>();
            }

            core->bindport = data["RTP"]["local_udp_port"].as_integer();
        }
        static
        void
        MakeRtpSender(web::json::value & surfapi){
            auto tool_id = surfapi["tool_req"]["tool_id"].as_integer();

            auto & core = GstreamerCore::CoreList.at(tool_id);
            if(!core){
                return;
            }
            auto command = GetCommandSender(*core);
            core->GstParseLaunchSender(command);

        }
        static
        void
        MakeRtpReceiver(web::json::value & surfapi){
            auto tool_id = surfapi["tool_req"]["tool_id"].as_integer();
            auto dst_tool_id = surfapi["tool_req"]["data"]["backend_tool_id"].as_integer();

            auto & sender_core = GstreamerCore::CoreList.at(tool_id);
            if(!sender_core){
                return;
            }

            auto & core = GstreamerCore::CoreList.at(dst_tool_id);
            if(!core){
                return;
            }
            auto command = GetCommandReceiver(*core);
            core->GstParseLaunchReceiver(*sender_core, command);

        }
        static
        void
        SetStatePaused(int index){

            auto & core = GstreamerCore::CoreList.at(index);
            if(!core){
                return;
            }
            core->GstPaused();
        }
        static
        void
        SetStateReady(int index){

            auto & core = GstreamerCore::CoreList.at(index);
            if(!core){
                return;
            }
            core->GstReady();
        }
        static
        void
        SetStatePlay(int index){

            auto & core = GstreamerCore::CoreList.at(index);
            if(!core){
                return;
            }
            core->GstPlay();
        }

        static
        std::string
        GetCommandSender(GstreamerCore & core){
            std::stringstream ss;
            if(core.codec == "AMR_NB"){
            ss 
                << "rtpbin name=rtpbin "
                << "audiotestsrc wave=0 ! "
                << "audioconvert ! "
                << "amrnbenc band-mode=7 ! "
                << "rtpamrpay pt=" << core.payload << " ssrc=" << core.ssrc << " ! "
                << "rtpbin.send_rtp_sink_0 rtpbin.send_rtp_src_0 ! "
                << "udpsink name=udpsink host=" << core.remoteip << " port=" << core.port << " bind-port=" << core.bindport << " ";
            }
            
            return ss.str();
        }


        // gst-launch-1.0 rtpbin name=rtpbin udpsrc caps="application/x-rtp,encoding-name=TELEPHONE-EVENT,payload=101,media=(string)audio, clock-rate=(int)0" port=5000 ! rtpbin.recv_rtp_sink_0 rtpbin. ! rtpdtmfdepay ! fakesink
        static
        std::string
        GetCommandReceiver(GstreamerCore & core){
            std::stringstream ss;
            ss 
                << "udpsrc name=udpsrc caps=application/x-rtp ! "
                << "rtpptdemux name=rtpptdemux ! "
                << " fakesink ";
            return ss.str();
        }

    };
}