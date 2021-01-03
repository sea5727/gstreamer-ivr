#pragma once

#include "ms_include.hpp"

namespace MediaServer
{
    class GstreamerCore{
    public:
        static std::array<std::shared_ptr<GstreamerCore>, 10000> CoreList;
        unsigned long ssrc;
        uint16_t payload;
        uint16_t bindport;
        std::string remoteip;
        uint16_t port;
        std::string codec;
        
        GstElement * pipeline;

    public:
        GstreamerCore() = default;

        void
        GstParseLaunch(const std::string & command){
            pipeline = gst_parse_launch(command.c_str(), NULL);
            g_assert(pipeline);

            GstBus * bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
            if(bus != nullptr){
                gst_bus_add_watch (bus, GstreamerCore::bus_callback, this);
                gst_object_unref (bus);
            }

        }

        std::string
        GetCommand(){
            std::stringstream ss;
            if(codec == "AMR_NB"){
            ss 
                << "rtpbin name=rtpbin "
                << "audiotestsrc wave=0 ! "
                << "audioconvert ! "
                << "amrnbenc band-mode=7 ! "
                << "rtpamrpay pt=" << payload << " ssrc=" << ssrc << " ! "
                << "rtpbin.send_rtp_sink_0 rtpbin.send_rtp_src_0 ! "
                << "udpsink host=" << remoteip << " port=" << port << " bind-port=" << bindport << " ";
            }
            
            return ss.str();
        }

        void
        GstPlay(){
           gst_element_set_state(pipeline, GST_STATE_PLAYING) ;
        }

        static
        gboolean
        bus_callback (GstBus * bus, GstMessage * message, gpointer data)
        {
            // Core * core = (Core *)data;
            // GstElement * pipeline = core->pipeline;
            auto bus_callback_id = std::this_thread::get_id();
            std::cout << "]############# Got bust... " << bus_callback_id << std::endl;
            g_print ("############# Got %s message.. src:%s\n", GST_MESSAGE_TYPE_NAME (message), gst_element_get_name (message->src));

            switch (GST_MESSAGE_TYPE (message)) {

                // case GST_MESSAGE_STATE_CHANGED:
                // {
                //     break;
                //     GstState old_state;
                //     GstState new_state;
                //     GstState pending_state;
                //     gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
                //     break;
                // }
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
                {
                    g_print("#### Got GST_MESSAGE_EOS Event.. ####\n");
                    // if(core->current_file_index + 1 == core->filesrc_locations.size()){
                    //     (*core).GstClear();
                    // }
                    // else {
                    //     gst_element_set_state(pipeline, GST_STATE_READY);
                    //     (*core).current_file_index += 1;
                    //     GstElement * filesrc = gst_bin_get_by_name(GST_BIN(pipeline), "filesrc");
                    //     g_object_set (filesrc, "location", core->filesrc_locations[core->current_file_index].c_str(), NULL);

                    //     GstElement * rtpamrpay = gst_bin_get_by_name(GST_BIN(pipeline), "rtppay");

                    //     GstRTPBasePayload *rtpbasepayload = GST_RTP_BASE_PAYLOAD (rtpamrpay);
                    //     guint32 new_timestamp = rtpbasepayload->timestamp;
                    //     guint16 new_seq = rtpbasepayload->seqnum;
                    //     g_object_set(rtpamrpay, "timestamp-offset", new_timestamp, "seqnum-offset", new_seq, NULL);

                    //     gst_element_set_state(pipeline, GST_STATE_PLAYING);
                    // }
                    break;
                }
                case GST_MESSAGE_ELEMENT: //gstdtmfdemay event..
                {
                    // const GstStructure * structure = gst_message_get_structure(message);

                    // auto cb = [](GQuark field, const GValue *value, gpointer user_data) -> gboolean {
                    //     MsCore * core = (MsCore *)user_data;
                    //     GstElement * pipeline = core->pipeline;


                    //     gchar *str = gst_value_serialize (value);
                    //     const char* fieldname = g_quark_to_string (field);
                    //     if(strcmp(fieldname, "number") == 0){
                    //         // int number = atoi(str);
                    //         // g_print("number:%d\n", number);
                    //         // HmpTcpParser parser;
                    //         // parser.tool_id = (*core).tool_id;
                    //         // std::stringstream ss;
                    //         // ss << "DTMF" << str;
                    //         // for(auto &event : (*core).event_list){
                    //         //     auto response1 = parser.MakeRtpEventDetected(ss.str());
                    //         //     g_print("response1:%s\n", response1.c_str());
                    //         //     event(response1);
                    //         //     auto response2 = parser.MakeRtpEventEnded();
                    //         //     g_print("response2:%s\n", response2.c_str());
                    //         //     event(response2);

                    //         // }
                    //     }
                    //     g_print ("%15s: %s\n", fieldname, str);
                    //     g_free (str);
                    //     return (gboolean)TRUE;
                    // };
                    // gst_structure_foreach(structure, cb, core);

                    break;
                }
                case GST_MESSAGE_ASYNC_DONE:
                {
                    // gst_message_parse_async_done()
                    break;
                }
                // case GST_MESSAGE_TAG:
                // {
                //     break;
                //     GstTagList *tags = NULL;
                //     gst_message_parse_tag(message, &tags);

                //     auto lambdatest = [](const GstTagList * list, const gchar * tag, gpointer user_data){
                //         GValue val = { 0, };
                //         if(!gst_tag_list_copy_value(&val, list, tag))
                //             return;
                //         gchar *str;
                //         if (G_VALUE_HOLDS_STRING (&val))
                //             str = g_value_dup_string (&val);
                //         else
                //             str = gst_value_serialize (&val);

                //         g_print ("%s:%s\n", gst_tag_get_nick (tag), str);
                //         g_free(str);

                //         g_value_unset(&val);

                //         // gint num = gst_tag_list_get_tag_size(list, tag);

                //         // for(gint i = 0 ; i < num ; ++i)
                //         // {
                //         //     const GValue * val;
                //         //     val = gst_tag_list_get_value_index(list, tag, i);
                //         //     if (G_VALUE_HOLDS_STRING (val)) {
                //         //         g_print ("    %s : %s \n", tag, g_value_get_string (val));
                //         //     } else if (G_VALUE_HOLDS_UINT (val)) {
                //         //         g_print ("    %s : %u \n", tag, g_value_get_uint (val));
                //         //     } else if (G_VALUE_HOLDS_DOUBLE (val)) {
                //         //         g_print ("    %s : %g \n", tag, g_value_get_double (val));
                //         //     } else if (G_VALUE_HOLDS_BOOLEAN (val)) {
                //         //         g_print ("    %s : %s \n", tag,
                //         //         g_value_get_boolean (val) ? "true" : "false");
                //         //     } else if (GST_VALUE_HOLDS_DATE_TIME (val)) {
                //         //         GstDateTime *dt = (GstDateTime *)g_value_get_boxed (val);
                //         //         gchar *dt_str = gst_date_time_to_iso8601_string (dt);

                //         //         g_print ("    %s : %s \n", tag, dt_str);
                //         //         g_free (dt_str);
                //         //     } else {
                //         //     g_print ("    %s : tag of type '%s' \n", tag, G_VALUE_TYPE_NAME (val));
                //         //     }
                //         // }
                //         g_print("GstTagList:%s.. , tag:%s..\n", gst_tag_list_to_string(list), tag);
                //     };
                //     gst_tag_list_foreach(tags, lambdatest, NULL);



                //     break;
                // }
                // case GST_MESSAGE_RESET_TIME:
                // {
                //     GstClockTime clocktime;
                //     gst_message_parse_reset_time(message, &clocktime);
                //     g_print("GST_MESSAGE_RESET_TIME : %" GST_TIME_FORMAT "\n", GST_TIME_ARGS(clocktime));
                //     break;
                // }
                // case GST_MESSAGE_SEGMENT_DONE:
                // {
                //     GstFormat format;
                //     gint64 position;
                //     gst_message_parse_segment_done(message, &format, &position);
                //     g_print("GST_MESSAGE_SEGMENT_DONE : format:%d, position:%d\n", format, position);
                //     gst_element_seek(pipeline, 1, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_SEGMENT), GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_END, 0);
                //     break;
                // }
                // case GST_MESSAGE_STREAM_STATUS:
                // {
                //     break;
                //     GstStreamStatusType type;
                //     GstElement *owner;
                //     const GValue *val;
                //     gchar *path;
                //     GstTask *task = NULL;
                //     gst_message_parse_stream_status(message, &type, &owner);

                //     val = gst_message_get_stream_status_object (message);

                //     g_print ("type:   %d", type);
                //     path = gst_object_get_path_string (GST_MESSAGE_SRC (message));
                //     g_print ("source: %s", path);
                //     g_free (path);
                //     path = gst_object_get_path_string (GST_OBJECT (owner));
                //     g_print ("owner:  %s", path);
                //     g_free (path);
                //     g_print ("object: type %s, value %p", G_VALUE_TYPE_NAME (val), g_value_get_object (val));

                //     /* see if we know how to deal with this object */
                //     if (G_VALUE_TYPE (val) == GST_TYPE_TASK) {
                //     task = (GstTask *)g_value_get_object (val);
                //     }

                //     switch (type) {
                //     case GST_STREAM_STATUS_TYPE_CREATE:
                //         g_print ("GST_STREAM_STATUS_TYPE_CREATE ... created task %p\n", task);
                //         break;
                //     case GST_STREAM_STATUS_TYPE_ENTER:
                //         g_print ("GST_STREAM_STATUS_TYPE_ENTER .. created task %p\n", task);
                //         /* g_message ("raising task priority"); */
                //         /* setpriority (PRIO_PROCESS, 0, -10); */
                //         break;
                //     case GST_STREAM_STATUS_TYPE_LEAVE:
                //         g_print ("GST_STREAM_STATUS_TYPE_LEAVE .. created task %p\n", task);
                //         break;
                //     default:
                //         g_print ("default .. created task %p\n", task);
                //         break;
                //     }
                //     break;
                // }

                case GST_MESSAGE_WARNING:
                {
                    GError *gerror;
                    gchar *debug;

                    if(message->type == GST_MESSAGE_ERROR)
                        gst_message_parse_error(message, &gerror, &debug);
                    else
                        gst_message_parse_warning(message, &gerror, &debug);
                    gst_object_default_error(GST_MESSAGE_SRC(message), gerror, debug);
                    g_error_free(gerror);
                    g_free(debug);

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

    };
}

std::array<std::shared_ptr<MediaServer::GstreamerCore>, 10000> MediaServer::GstreamerCore::CoreList;