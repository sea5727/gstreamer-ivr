#ifndef __MS_GST_BUILDER_HPP__
#define __MS_GST_BUILDER_HPP__

#include <gst/gst.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>

using std::string;
using std::vector;

extern GMainLoop *loop;



// void testf(const GstTagList * list, const gchar      * tag, gpointer           user_data)
// {

// }


static 
gboolean
my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
    
    
    g_print ("Got %s message.. src:%s\n", GST_MESSAGE_TYPE_NAME (message), gst_element_get_name (message->src));

    switch (GST_MESSAGE_TYPE (message)) {

        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old_state;
            GstState new_state;
            GstState pending_state;
            gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
            g_print("####### GST_MESSAGE_STATE_CHANGED.. old:%s, new:%s, pending:%s\n", 
                gst_element_state_get_name (old_state), 
                gst_element_state_get_name (new_state), 
                gst_element_state_get_name (pending_state));
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            GError *err;
            gchar *debug;

            gst_message_parse_error (message, &err, &debug);
            g_print ("########  Error: %s\n", err->message);
            g_error_free (err);
            g_free (debug);
            break;
        }
        case GST_MESSAGE_EOS:
            g_print ("########  GST_MESSAGE_EOS ############ \n");
            break;
        case GST_MESSAGE_ASYNC_DONE:
        {
            // gst_message_parse_async_done()
            break;
        }
        case GST_MESSAGE_TAG:
        {
            GstTagList *tags = NULL;
            gst_message_parse_tag(message, &tags);

            auto lambdatest = [](const GstTagList * list, const gchar * tag, gpointer user_data){
                GValue val = { 0, };
                if(!gst_tag_list_copy_value(&val, list, tag))
                    return;
                gchar *str;
                if (G_VALUE_HOLDS_STRING (&val))
                    str = g_value_dup_string (&val);
                else
                    str = gst_value_serialize (&val);

                g_print ("%s:%s\n", gst_tag_get_nick (tag), str);
                g_free(str);

                g_value_unset(&val);

                // gint num = gst_tag_list_get_tag_size(list, tag);

                // for(gint i = 0 ; i < num ; ++i)
                // {
                //     const GValue * val;
                //     val = gst_tag_list_get_value_index(list, tag, i);
                //     if (G_VALUE_HOLDS_STRING (val)) {
                //         g_print ("    %s : %s \n", tag, g_value_get_string (val));
                //     } else if (G_VALUE_HOLDS_UINT (val)) {
                //         g_print ("    %s : %u \n", tag, g_value_get_uint (val));
                //     } else if (G_VALUE_HOLDS_DOUBLE (val)) {
                //         g_print ("    %s : %g \n", tag, g_value_get_double (val));
                //     } else if (G_VALUE_HOLDS_BOOLEAN (val)) {
                //         g_print ("    %s : %s \n", tag,
                //         g_value_get_boolean (val) ? "true" : "false");
                //     } else if (GST_VALUE_HOLDS_DATE_TIME (val)) {
                //         GstDateTime *dt = (GstDateTime *)g_value_get_boxed (val);
                //         gchar *dt_str = gst_date_time_to_iso8601_string (dt);

                //         g_print ("    %s : %s \n", tag, dt_str);
                //         g_free (dt_str);
                //     } else {
                //     g_print ("    %s : tag of type '%s' \n", tag, G_VALUE_TYPE_NAME (val));
                //     }
                // }


                g_print("GstTagList:%s.. , tag:%s..\n", gst_tag_list_to_string(list), tag);
            };
            gst_tag_list_foreach(tags, lambdatest, NULL);

            

            break;
        }
        
        default:
        /* unhandled message */
        break;
    }

    /* we want to be notified again the next time there is a message
    * on the bus, so returning TRUE (FALSE means we want to stop watching
    * for messages on the bus and our callback should not be called again)
    */
    return TRUE;
}


static void
cb_state (GstBus * bus, GstMessage * message, gpointer data)
{
    g_print("cb_state start\n");
  GstObject *pipe = GST_OBJECT (data);
  GstState old, new_, pending;
  gst_message_parse_state_changed (message, &old, &new_, &pending);
  if (message->src == pipe) {
    g_print ("Pipeline %s changed state from %s to %s\n",
        GST_OBJECT_NAME (message->src),
        gst_element_state_get_name (old), gst_element_state_get_name (new_));
  }
}


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
        void modify_element(std::string name, std::string field, string value)
        {

            std::cout << name << std::endl;
            std::cout << field << std::endl;
            std::cout << value << std::endl;
            GstState state;
            GstState pending;
            GstStateChangeReturn ret;
            ret = gst_element_get_state (pipeline, &state, &pending, -1);
            std::cout << "current get state.. ret : " << ret << state << pending << std::endl;

            GstElement * element
                = gst_bin_get_by_name(GST_BIN(pipeline), name.c_str());
            g_assert (element);

            // ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);


            ret = gst_element_set_state (element, GST_STATE_READY);
            // ret = gst_element_set_state (element, GST_STATE_READY);
            g_object_set(element, field.c_str(), value.c_str(), NULL);
            // ret = gst_element_set_state (element, GST_STATE_PAUSED);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            ret = gst_element_set_state (pipeline, GST_STATE_READY);
            ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);


            // ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
        }
        void make_pipeline_by_command(std::string command)
        {
            std::cout << "make_pipeline_by_command" << command.c_str() << std::endl;
            pipeline = gst_parse_launch(command.c_str(), NULL);
            g_assert(pipeline);

            GstBus * bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
            g_assert(bus);
            gst_bus_add_watch (bus, my_bus_callback, NULL);
            gst_object_unref (bus);

            /* Start playing */
            gst_element_set_state (pipeline, GST_STATE_PLAYING);
            std::cout << "make_pipeline_by_command end\n";
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