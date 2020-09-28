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


extern GstElement * filesrc_t;
extern GstElement * queue_t;
extern GstElement * wavparse_t;
extern GstElement * alawdec_t;
extern GstElement * amrnbenc_t;
extern GstElement * rtpamrpay_t;
extern GstElement * udpsink_t;


// void testf(const GstTagList * list, const gchar      * tag, gpointer           user_data)
// {

// }



static gboolean
cb_print_position (GstElement *pipeline)
{
  gint64 pos, len;

  if (gst_element_query_position (pipeline, GST_FORMAT_TIME, &pos)
    && gst_element_query_duration (pipeline, GST_FORMAT_TIME, &len)) {
    g_print ("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
         GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
  }

  /* call me again */
  return TRUE;
}



static 
gboolean
my_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
    
    
    g_print ("############# Got %s message.. src:%s\n", GST_MESSAGE_TYPE_NAME (message), gst_element_get_name (message->src));

    switch (GST_MESSAGE_TYPE (message)) {

        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old_state;
            GstState new_state;
            GstState pending_state;
            gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
            // g_print("####### GST_MESSAGE_STATE_CHANGED.. old:%s, new:%s, pending:%s\n", 
            //     gst_element_state_get_name (old_state), 
            //     gst_element_state_get_name (new_state), 
            //     gst_element_state_get_name (pending_state));

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
        case GST_MESSAGE_STREAM_STATUS:
        {
            GstStreamStatusType type;
            GstElement *owner;
            const GValue *val;
            gchar *path;
            GstTask *task = NULL;
            gst_message_parse_stream_status(message, &type, &owner);

            val = gst_message_get_stream_status_object (message);
   
            g_print ("type:   %d", type);
            path = gst_object_get_path_string (GST_MESSAGE_SRC (message));
            g_print ("source: %s", path);
            g_free (path);
            path = gst_object_get_path_string (GST_OBJECT (owner));
            g_print ("owner:  %s", path);
            g_free (path);
            g_print ("object: type %s, value %p", G_VALUE_TYPE_NAME (val), g_value_get_object (val));
   
            /* see if we know how to deal with this object */
            if (G_VALUE_TYPE (val) == GST_TYPE_TASK) {
              task = (GstTask *)g_value_get_object (val);
            }
   
            switch (type) {
              case GST_STREAM_STATUS_TYPE_CREATE:
                g_print ("GST_STREAM_STATUS_TYPE_CREATE ... created task %p", task);
                break;
              case GST_STREAM_STATUS_TYPE_ENTER:
                g_print ("GST_STREAM_STATUS_TYPE_ENTER .. created task %p", task);
                /* g_message ("raising task priority"); */
                /* setpriority (PRIO_PROCESS, 0, -10); */
                break;
              case GST_STREAM_STATUS_TYPE_LEAVE:
                g_print ("GST_STREAM_STATUS_TYPE_LEAVE .. created task %p", task);
                break;
              default:
                g_print ("default .. created task %p", task);
                break;
            }
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
            GstClockTime starttime = gst_element_get_start_time(filesrc_t);
            GstClockTime basetime = gst_element_get_base_time(filesrc_t);
            g_print("starttime : %"GST_TIME_FORMAT"\n", GST_TIME_ARGS(starttime));
            g_print("basetime : %"GST_TIME_FORMAT"\n", GST_TIME_ARGS(basetime));
            GST_ERROR("This is modify_element start\n");

            
            gst_element_set_state (pipeline, GST_STATE_PAUSED);
            
            // std::cout << "gst_element_seek_simple start\n";
            
            // gst_element_seek_simple (pipeline, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 0 );
            gst_element_seek_simple (pipeline, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_SEGMENT), 0 );


            // pipeline->start_time
            std::cout << "filesrc_t GST_STATE_READY\n";
            // gst_element_set_state (wavparse_t, GST_STATE_READY);
            gst_element_set_state (filesrc_t, GST_STATE_NULL);
            
            g_object_set(filesrc_t, field.c_str(), value.c_str(), NULL);
            
            
            gst_element_sync_state_with_parent(filesrc_t);

            std::cout << "filesrc_t GST_STATE_PLAYING\n";
            // std::cout << "pipeline GST_STATE_PLAYING\n";
            gst_element_set_state (pipeline, GST_STATE_PLAYING);
            GST_ERROR("This is modify_element end\n");
            
            GstClockTime starttime2 = gst_element_get_start_time(filesrc_t);
            GstClockTime basetime2 = gst_element_get_base_time(filesrc_t);
            g_print("starttime2 : %"GST_TIME_FORMAT"\n", GST_TIME_ARGS(starttime2));
            g_print("basetime2 : %"GST_TIME_FORMAT"\n", GST_TIME_ARGS(basetime2));


            return;
            std::cout << "######### start########## \n";

            std::cout << "######### GST_STATE_PAUSED ########## \n";
            gst_element_set_state (pipeline, GST_STATE_PAUSED);
            std::cout << "######### gst_bin_remove ########## \n";
            // gst_element_unlink(filesrc, wavparse);
            gst_bin_remove(GST_BIN(pipeline), filesrc_t);
            std::cout << "######### gst_bin_remove 2 ########## \n";
            gst_bin_remove(GST_BIN(pipeline), wavparse_t);
            
            GstElement * new_filesrc = gst_element_factory_make ("filesrc", "filesrc_1");
            g_assert (new_filesrc);
            g_object_set(new_filesrc, field.c_str(), value.c_str(), NULL);
            GstElement * new_wavparse = gst_element_factory_make ("wavparse", "wavparse_1");
            g_assert (new_wavparse);
            gst_bin_add_many(GST_BIN (pipeline), new_filesrc, new_wavparse, NULL);

            if (!gst_element_link_many (new_filesrc, new_wavparse, alawdec_t, NULL)) {
                g_error ("Failed to link audiosrc, audioconv, audioresample, "
                    "audio encoder and audio payloader");
            }

            std::cout << "######### pipeline GST_STATE_PLAYING########## \n";
            gst_element_set_state (pipeline, GST_STATE_PLAYING);

            std::cout << "######### end ########## \n";
            return;
            // std::cout << name << std::endl;
            // std::cout << field << std::endl;
            // std::cout << value << std::endl;
            // GstState state;
            // GstState pending;
            // GstStateChangeReturn ret;
            // ret = gst_element_get_state (pipeline, &state, &pending, -1);
            // std::cout << "current get state.. ret : " << ret << state << pending << std::endl;

            // GST_ERROR("This is modify_element start\n");
            // GstElement * filesrc
            //     = gst_bin_get_by_name(GST_BIN(pipeline), name.c_str());
            // g_assert (filesrc);

            // GstElement * wavparse
            //     = gst_bin_get_by_name(GST_BIN(pipeline), "wavparse");
            // g_assert (wavparse);


            // std::cout << "start GST_STATE_PAUSED??\n";
            // gst_element_set_state (pipeline, GST_STATE_PAUSED);
            // std::cout << "start GST_STATE_READY??\n";
            // // gst_element_set_state(wavparse, GST_STATE_READY);
            // // g_assert (nullptr);
            // std::cout << "start gst_element_unlink??\n";

            // gst_element_unlink(filesrc, wavparse);
            // gst_bin_remove(GST_BIN(pipeline), filesrc);

            // std::cout << "start gst_bin_remove3??\n";
            // GstState TestState;
            // gst_element_get_state(filesrc, &TestState, NULL, GST_CLOCK_TIME_NONE);
            // std::clog << "########## FIRST TestState: " << gst_element_state_get_name(TestState) << std::endl;


            // GstElement * new_filesrc = gst_element_factory_make ("filesrc", "filesrc_1");
            // g_assert (new_filesrc);

            // g_object_set(new_filesrc, field.c_str(), value.c_str(), NULL);

            // std::cout << "start gst_bin_add_many\n";
            // gst_bin_add(GST_BIN (pipeline), new_filesrc);



            // std::this_thread::sleep_for(std::chrono::seconds(1));


            // if (!gst_element_link_many (new_filesrc, wavparse, NULL)) {
            //     g_error ("Failed to link audiosrc, audioconv, audioresample, "
            //         "audio encoder and audio payloader");
            // }

            // gst_element_set_state ( wavparse, GST_STATE_PAUSED);
            // gst_element_set_state  (filesrc, GST_STATE_PLAYING);

            //  gst_element_set_state (pipeline, GST_STATE_PLAYING);

            //  GST_ERROR("This is modify_element end\n");


            // std::cout << "start sleep\n";
            // std::this_thread::sleep_for(std::chrono::seconds(2));
            // std::cout << "start cb_print_position..\n";
            // cb_print_position(pipeline);
            // std::cout << "end cb_print_position..\n";
            // gint64 duration;
            // GstQuery *query;
            // gboolean res;
            // query = gst_query_new_duration (GST_FORMAT_TIME);
            // res = gst_element_query (pipeline, query);
            // if (res) {
            //     gint64 duration;
            //     gst_query_parse_duration (query, NULL, &duration);
            //     g_print ("duration = %"GST_TIME_FORMAT, GST_TIME_ARGS (duration));
            // } else {
            //     g_print ("duration query failed...");
            // }
            // gst_query_unref (query);


#if 0
            GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
            GstMessage* eosmsg = gst_message_new_eos(GST_OBJECT(pipeline));
            gst_bus_post(bus, eosmsg);
            g_object_unref(bus);
#endif

        }
        void make_pipeline_by_command(std::string command)
        {
#if 0
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


                
            g_object_set (udpsink, "host", "192.168.0.4", NULL);
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
#endif
#if 1
//"rtpbin name=rtpbin filesrc name=filesrc_0 location=/home/ysh8361/BCN3009999990.wav !  
// wavparse name=wavparse_0 ! 
//alawdec name=alawdec_0 ! 
//amrnbenc ! 
//rtpamrpay pt=104 !  
//rtpbin.send_rtp_sink_0 ;rtpbin.send_rtp_src_0 ! udpsink async=false host=192.168.0.4 port=5000"
            // GstElement * filesrc;
            // GstElement * wavparse;
            // GstElement * alawdec;
            // GstElement * amrnbenc;
            // GstElement * rtpamrpay;
            // GstElement * udpsink;

            pipeline = gst_pipeline_new (NULL);
            g_assert (pipeline);

            filesrc_t = gst_element_factory_make ("filesrc", "filesrc");
            g_object_set (filesrc_t, "location", "/home/ysh8361/BCN3009999990.wav", NULL);
            g_assert (filesrc_t);

            queue_t = gst_element_factory_make("queue", "queue");
            g_assert (queue_t);

            wavparse_t = gst_element_factory_make ("wavparse", "wavparse");
            g_assert (wavparse_t);

            alawdec_t = gst_element_factory_make ("alawdec", "alawdec");
            g_assert (alawdec_t);

            amrnbenc_t = gst_element_factory_make ("amrnbenc", "amrnbenc");
            g_assert (amrnbenc_t);

            rtpamrpay_t = gst_element_factory_make ("rtpamrpay", "rtpamrpay");
            g_assert (rtpamrpay_t);

            g_object_set (rtpamrpay_t, "pt", 104, NULL);


            udpsink_t = gst_element_factory_make ("udpsink", "udpsink");
            g_assert (udpsink_t);

            g_object_set (udpsink_t, "host", "192.168.0.4", NULL);
            g_object_set (udpsink_t, "port", 5000, NULL);


            /* add capture and payloading to the pipeline and link */
            gst_bin_add_many (GST_BIN (pipeline), filesrc_t, queue_t, wavparse_t, alawdec_t, amrnbenc_t, rtpamrpay_t, udpsink_t, NULL);

            if (!gst_element_link_many (filesrc_t, queue_t, wavparse_t, alawdec_t, amrnbenc_t, rtpamrpay_t, udpsink_t, NULL)) {
                g_error ("Failed to link audiosrc, audioconv, audioresample, "
                    "audio encoder and audio payloader");
            }


            GstBus * bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
            g_assert(bus);
            gst_bus_add_watch (bus, my_bus_callback, NULL);
            gst_object_unref (bus);

            g_timeout_add (200, (GSourceFunc) cb_print_position, pipeline);

            /* set the pipeline to playing */
            g_print ("starting sender pipeline\n");
            gst_element_set_state (pipeline, GST_STATE_PLAYING);
#endif
#if 0
            std::cout << "make_pipeline_by_command" << command.c_str() << std::endl;
            pipeline = gst_parse_launch(command.c_str(), NULL);
            g_assert(pipeline);

            GstBus * bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
            g_assert(bus);
            gst_bus_add_watch (bus, my_bus_callback, NULL);
            gst_object_unref (bus);


            g_timeout_add (200, (GSourceFunc) cb_print_position, pipeline);

            
            /* Start playing */
            gst_element_set_state (pipeline, GST_STATE_PLAYING);
            std::cout << "make_pipeline_by_command end\n";
#endif
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
