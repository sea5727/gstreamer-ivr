#pragma once

#include "ms_include.hpp"

namespace MediaServer
{
    class HmpParser{
    public:
        HmpParser() = default;
        
        static
        void
        Parse(web::json::value & surfapi){
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
        MakeReceiver(web::json::value & surfapi){
            auto tool_id = surfapi["tool_req"]["tool_id"].as_integer();

            auto & core = GstreamerCore::CoreList.at(tool_id);
            if(!core){
                return;
            }
            core->GstParseLaunch(core->GetCommand());

        }

        static
        void
        SetStatePlay(web::json::value & surfapi){
            auto tool_id = surfapi["tool_req"]["tool_id"].as_integer();

            auto & core = GstreamerCore::CoreList.at(tool_id);
            if(!core){
                return;
            }
            core->GstPlay();
        }

    };
}