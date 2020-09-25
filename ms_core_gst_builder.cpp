#include "ms_core_gst_builder.hpp"

using namespace MsCore;

GstCoreBuilder GstCoreBuilder::create()
{
    GstCoreBuilder builder;
    return builder;
}

GstCoreBuilder & GstCoreBuilder::set_file_location(vector<string> file_locations)
{
    gst_core->_file_locations.clear();
    gst_core->_file_locations.assign(file_locations.begin(), file_locations.end());
    return *this;
}

GstCoreBuilder & GstCoreBuilder::set_remote_rtp_ip(string remote_rtp_ip)
{
    gst_core->_remote_rtp_ip = remote_rtp_ip;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_remote_rtp_port(uint16_t remote_rtp_port)
{
    gst_core->_remote_rtp_port = remote_rtp_port;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_local_rtp_ip(string local_rtp_ip)
{
    gst_core->_local_rtp_ip = local_rtp_ip;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_local_rtp_port(uint16_t local_rtp_port)
{
    gst_core->_local_rtp_port = local_rtp_port;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_remote_rtcp_ip(string remote_rtcp_ip)
{
    gst_core->_remote_rtcp_ip = remote_rtcp_ip;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_remote_rtcp_port(uint16_t remote_rtcp_port)
{
    gst_core->_remote_rtcp_port = remote_rtcp_port;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_local_rtcp_ip(string local_rtcp_ip)
{
    gst_core->_local_rtcp_ip = local_rtcp_ip;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_local_rtcp_port(uint16_t local_rtcp_port)
{
    gst_core->_local_rtcp_port = local_rtcp_port;
    return *this;
}
GstCoreBuilder & GstCoreBuilder::set_audio_payload(uint16_t audio_payload)
{
    gst_core->_audio_payload = audio_payload;
    return *this;
}

GstCoreBuilder & GstCoreBuilder::proc_crcx_pipeline()
{
    gst_core->make_silence();
    return *this;
}

GstCoreBuilder & GstCoreBuilder::proc_rqnt_tone(int wave)
{
    gst_core->replace(wave);
    return *this;
}

GstCoreBuilder & GstCoreBuilder::proc_by_command(const std::string command)
{
    std::cout << "proc_by_command" << command << std::endl;
    gst_core->make_pipeline_by_command(std::move(command));
    return *this;
}
GstCoreBuilder & GstCoreBuilder::proc_modify_pipeline(const string name, const string field, const string value)
{
    gst_core->modify_element(std::move(name), std::move(field), std::move(value));
    return *this;
}



GstCoreBuilder & GstCoreBuilder::proc_make_tone()
{
    gst_core->make_silence();
    return *this;
}
