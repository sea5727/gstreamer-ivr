#ifndef __MS_CORE_GST_BUILDER_HPP__
#define __MS_CORE_GST_BUILDER_HPP__

#include "ms_core_gst.hpp"
#include <utility>
#include <memory>

namespace MsCore
{
    class GstCoreBuilder
    {
        // GstCore gst_core;
        std::shared_ptr<GstCore> gst_core;
    public:
        GstCoreBuilder() 
            : gst_core(std::make_shared<GstCore>())
        {
        }
        GstCoreBuilder(std::shared_ptr<GstCore> copy) 
            : gst_core(copy)
        {

        }
        ~GstCoreBuilder() {}

        static GstCoreBuilder create();

        std::shared_ptr<GstCore> get() { return gst_core; }
        // operator GstCore() const { return std::move(gst_core); }

        /* service process */
        GstCoreBuilder & proc_make_tone();
        /* service process */
        GstCoreBuilder & proc_crcx_pipeline();
        /* service process */
        GstCoreBuilder & proc_rqnt_tone(int wave);

        /* setter */
        GstCoreBuilder & 
        set_file_location(vector<string> file_locations);

        /* setter */
        GstCoreBuilder & 
        set_remote_rtp_ip(string remote_rtp_ip);

        /* setter */
        GstCoreBuilder & 
        set_remote_rtp_port(uint16_t remote_rtp_port);

        /* setter */
        GstCoreBuilder & 
        set_local_rtp_ip(string local_rtp_ip);

        /* setter */
        GstCoreBuilder & 
        set_local_rtp_port(uint16_t local_rtp_port);

        /* setter */
        GstCoreBuilder & 
        set_remote_rtcp_ip(string remote_rtcp_ip);

        /* setter */
        GstCoreBuilder & 
        set_remote_rtcp_port(uint16_t remote_rtcp_port);

        /* setter */
        GstCoreBuilder & 
        set_local_rtcp_ip(string local_rtcp_ip);

        /* setter */
        GstCoreBuilder & 
        set_local_rtcp_port(uint16_t local_rtcp_port);

        /* setter */
        GstCoreBuilder & 
        set_audio_payload(uint16_t audio_payload);
    };
}


#endif