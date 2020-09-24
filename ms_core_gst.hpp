#ifndef __MS_GST_BUILDER_HPP__
#define __MS_GST_BUILDER_HPP__

#include <gst/gst.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <utility>

using std::string;
using std::vector;

extern GMainLoop *loop;

namespace MsCore
{
    class GstCore
    {
    public:
        GstElement * pipeline;

        vector<string> _file_locations;
        
        string _remote_rtp_ip;
        uint16_t _remote_rtp_port;

        string _local_rtp_ip;
        uint16_t _local_rtp_port;

        string _remote_rtcp_ip;
        uint16_t _remote_rtcp_port;

        string _local_rtcp_ip;
        uint16_t _local_rtcp_port;


        uint16_t _audio_payload;

    public:
        GstCore()
            : pipeline(nullptr)
            , _file_locations()
            , _remote_rtp_ip("")
            , _remote_rtp_port(0)
            , _local_rtp_ip("")
            , _local_rtp_port(0)
            , _remote_rtcp_ip("")
            , _remote_rtcp_port(0)
            , _local_rtcp_ip("")
            , _local_rtcp_port(0)
            , _audio_payload(0)
        {

        }

        void replace(int data)
        {
            std::cout << "replace : " << data << std::endl;
            if(pipeline == nullptr)
                return;

            GstElement * audiotestsrc = gst_bin_get_by_name(GST_BIN(pipeline), "audiotestsrc_0");
            g_assert (audiotestsrc);

            gst_element_set_state (pipeline, GST_STATE_PAUSED);

            gint prev_wave;
            g_object_get(audiotestsrc, "wave", &prev_wave, NULL);

            std::cout << "prev_wave : " << prev_wave << std::endl;

            g_object_set(audiotestsrc, "wave", data, NULL);

            gint new_wave;
            g_object_get(audiotestsrc, "wave", &new_wave, NULL);

            std::cout << "new_wave : " << new_wave << std::endl;

            gst_element_set_state (pipeline, GST_STATE_PLAYING);

            std::cout << "replace : end" << data << std::endl;
        }

        void make_tone_pcm()
        {
            if(_remote_rtp_ip.empty())
                g_error ("make_tone _remote_rtp_ip is empty");
            if(_remote_rtp_port == 0)
                g_error ("make_tone _remote_rtp_port is empty");
            if(_audio_payload == 0)
                g_error ("make_tone _audio_payload is empty");
            GstElement * audiotestsrc;
            GstElement * audioconvert;
            GstElement * amrnbenc;
            GstElement * rtpamrpay;
            GstElement * udpsink;

            pipeline = gst_pipeline_new (NULL);
            g_assert (pipeline);

            audiotestsrc = gst_element_factory_make ("audiotestsrc", "audiotestsrc_0");
            g_assert (audiotestsrc);

            audioconvert = gst_element_factory_make ("audioconvert", "audioconvert");
            g_assert (audioconvert);

            amrnbenc = gst_element_factory_make ("amrnbenc", "amrnbenc");
            g_assert (amrnbenc);

            rtpamrpay = gst_element_factory_make ("rtpamrpay", "rtpamrpay");
            g_assert (rtpamrpay);

            g_object_set (rtpamrpay, "pt", 104, NULL);


            udpsink = gst_element_factory_make ("udpsink", "udpsink");
            g_assert (udpsink);


                
            g_object_set (udpsink, "host", "192.168.0.219", NULL);
            g_object_set (udpsink, "port", 5000, NULL);


            /* add capture and payloading to the pipeline and link */
            gst_bin_add_many (GST_BIN (pipeline), audiotestsrc, audioconvert, amrnbenc, rtpamrpay, udpsink, NULL);

            if (!gst_element_link_many (audiotestsrc, audioconvert, amrnbenc, rtpamrpay, udpsink, NULL)) {
                g_error ("Failed to link audiosrc, audioconv, audioresample, "
                    "audio encoder and audio payloader");
            }

            /* set the pipeline to playing */
            g_print ("starting sender pipeline\n");
            gst_element_set_state (pipeline, GST_STATE_PLAYING);

        }

        void make_silence()
        {
            
            if(_remote_rtp_ip.empty())
                g_error ("make_tone _remote_rtp_ip is empty");
            if(_remote_rtp_port == 0)
                g_error ("make_tone _remote_rtp_port is empty");
            if(_audio_payload == 0)
                g_error ("make_tone _audio_payload is empty");
            
            std::cout << "_remote_rtp_ip:" << _remote_rtp_ip << std::endl;
            std::cout << "_remote_rtp_port:" << _remote_rtp_port << std::endl;
            std::cout << "_audio_payload:" << _audio_payload << std::endl;


            GstElement * audiotestsrc;
            GstElement * audioconvert;
            GstElement * amrnbenc;
            GstElement * rtpamrpay;
            GstElement * udpsink;

            pipeline = gst_pipeline_new (NULL);
            g_assert (pipeline);

            audiotestsrc = gst_element_factory_make ("audiotestsrc", "audiotestsrc_0");
            g_assert (audiotestsrc);
            g_object_set(audiotestsrc, "wave", 2, NULL);

            audioconvert = gst_element_factory_make ("audioconvert", "audioconvert");
            g_assert (audioconvert);

            amrnbenc = gst_element_factory_make ("amrnbenc", "amrnbenc");
            g_assert (amrnbenc);

            rtpamrpay = gst_element_factory_make ("rtpamrpay", "rtpamrpay");
            g_assert (rtpamrpay);

            g_object_set (rtpamrpay, "pt", _audio_payload, NULL);


            udpsink = gst_element_factory_make ("udpsink", "udpsink");
            g_assert (udpsink);
                
            g_object_set (udpsink, "host", _remote_rtp_ip.c_str(), NULL);
            g_object_set (udpsink, "port", _remote_rtp_port, NULL);


            /* add capture and payloading to the pipeline and link */
            gst_bin_add_many (GST_BIN (pipeline), audiotestsrc, audioconvert, amrnbenc, rtpamrpay, udpsink, NULL);

            if (!gst_element_link_many (audiotestsrc, audioconvert, amrnbenc, rtpamrpay, udpsink, NULL)) {
                g_error ("Failed to link audiotestsrc, audioconv, audioresample, "
                    "audio encoder and audio payloader");
            }

            /* set the pipeline to playing */
            g_print ("starting sender pipeline\n");
            gst_element_set_state (pipeline, GST_STATE_PLAYING);

        }

        void make_ivr(const char * dest_host, uint16_t dest_port)
        {
            std::cout << "start make_ivr!!! " << std::endl;
            GstElement *audiosrc, *audioconv, *audiores, *audioenc, *audiopay;
            GstElement *rtpbin, *rtpsink, *rtcpsink, *rtcpsrc;
            GMainLoop *loop;
            GstPad *srcpad, *sinkpad;
            /* the pipeline to hold everything */
            pipeline = gst_pipeline_new (NULL);
            g_assert (pipeline);

            /* the audio capture and format conversion */
            audiosrc = gst_element_factory_make ("audiotestsrc", "audiosrc");
            g_assert (audiosrc);
            audioconv = gst_element_factory_make ("audioconvert", "audioconv");
            g_assert (audioconv);
            audiores = gst_element_factory_make ("audioresample", "audiores");
            g_assert (audiores);
            /* the encoding and payloading */
            audioenc = gst_element_factory_make ("alawenc", "audioenc");
            g_assert (audioenc);
            audiopay = gst_element_factory_make ("rtppcmapay", "audiopay");
            g_assert (audiopay);

            /* add capture and payloading to the pipeline and link */
            gst_bin_add_many (GST_BIN (pipeline), audiosrc, audioconv, audiores,
                audioenc, audiopay, NULL);

            if (!gst_element_link_many (audiosrc, audioconv, audiores, audioenc,
                    audiopay, NULL)) {
                g_error ("Failed to link audiosrc, audioconv, audioresample, "
                    "audio encoder and audio payloader");
            }

            /* the rtpbin element */
            rtpbin = gst_element_factory_make ("rtpbin", "rtpbin");
            g_assert (rtpbin);

            gst_bin_add (GST_BIN (pipeline), rtpbin);

            /* the udp sinks and source we will use for RTP and RTCP */
            rtpsink = gst_element_factory_make ("udpsink", "rtpsink");
            g_assert (rtpsink);
            g_object_set (rtpsink, "port", dest_port, "host", dest_host, NULL);

            rtcpsink = gst_element_factory_make ("udpsink", "rtcpsink");
            g_assert (rtcpsink);
            g_object_set (rtcpsink, "port", 5003, "host", dest_host, NULL);
            /* no need for synchronisation or preroll on the RTCP sink */
            g_object_set (rtcpsink, "async", FALSE, "sync", FALSE, NULL);

            rtcpsrc = gst_element_factory_make ("udpsrc", "rtcpsrc");
            g_assert (rtcpsrc);
            g_object_set (rtcpsrc, "port", 5007, NULL);

            gst_bin_add_many (GST_BIN (pipeline), rtpsink, rtcpsink, rtcpsrc, NULL);

            /* now link all to the rtpbin, start by getting an RTP sinkpad for session 0 */
            sinkpad = gst_element_get_request_pad (rtpbin, "send_rtp_sink_0");
            srcpad = gst_element_get_static_pad (audiopay, "src");
            if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
                g_error ("Failed to link audio payloader to rtpbin");
            gst_object_unref (srcpad);

            /* get the RTP srcpad that was created when we requested the sinkpad above and
            * link it to the rtpsink sinkpad*/
            srcpad = gst_element_get_static_pad (rtpbin, "send_rtp_src_0");
            sinkpad = gst_element_get_static_pad (rtpsink, "sink");
            if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
                g_error ("Failed to link rtpbin to rtpsink");
            gst_object_unref (srcpad);
            gst_object_unref (sinkpad);

            /* get an RTCP srcpad for sending RTCP to the receiver */
            srcpad = gst_element_get_request_pad (rtpbin, "send_rtcp_src_0");
            sinkpad = gst_element_get_static_pad (rtcpsink, "sink");
            if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
                g_error ("Failed to link rtpbin to rtcpsink");
            gst_object_unref (sinkpad);

            /* we also want to receive RTCP, request an RTCP sinkpad for session 0 and
            * link it to the srcpad of the udpsrc for RTCP */
            srcpad = gst_element_get_static_pad (rtcpsrc, "src");
            sinkpad = gst_element_get_request_pad (rtpbin, "recv_rtcp_sink_0");
            if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
                g_error ("Failed to link rtcpsrc to rtpbin");
            gst_object_unref (srcpad);

            /* set the pipeline to playing */
            g_print ("starting sender pipeline\n");
            gst_element_set_state (pipeline, GST_STATE_PLAYING);

            std::cout << "end make_ivr!!! " << std::endl;

        }
    };
}


#endif