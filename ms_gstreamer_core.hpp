#pragma once

#include "ms_include.hpp"

namespace MediaServer
{
    class GstreamerCore{
    public:
        static std::array<std::shared_ptr<GstreamerCore>, 10000> CoreList;
        Manager & pm = Manager::getInstance();
        unsigned long ssrc;
        uint16_t payload;
        uint16_t bindport;
        std::string remoteip;
        uint16_t port;
        std::string codec;
        GstElement * pipeline;

        int tool_id;
        boost::asio::ip::tcp::socket & sock;
        std::vector<std::string> filelist;
        int current_file_index;

    public:
        GstreamerCore(int tool_id, boost::asio::ip::tcp::socket & sock)
            : tool_id{tool_id}
            , sock{sock}
            , filelist{}
            , current_file_index{-1}
            { }

        void
        GstParseLaunchSender(const std::string & command){

            pm.Logger().debug("[CORE] GstParseLaunch : {}", command);

            pipeline = gst_parse_launch(command.c_str(), NULL);
            g_assert(pipeline);

            GstBus * bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
            if(bus != nullptr){
                gst_bus_add_watch (bus, GstreamerCore::bus_callback, this);
                gst_object_unref (bus);
            }
        }
        void
        GstParseLaunchReceiver(GstreamerCore & sender_core, const std::string & command){

            pipeline = gst_parse_launch(command.c_str(), NULL);
            g_assert(pipeline);

            GstElement* udpsink = gst_bin_get_by_name(GST_BIN(sender_core.pipeline), "udpsink");
            GSocket *udp_sink_socket;
            g_object_get (udpsink, "used-socket", &udp_sink_socket, NULL);
            

            GstElement* udpsrc = gst_bin_get_by_name(GST_BIN(pipeline), "udpsrc");
            g_object_set (udpsrc, "socket", udp_sink_socket, NULL);

            GstElement* rtpbin = gst_bin_get_by_name(GST_BIN(pipeline), "rtpbin");
            


            // GstElement* rtpptdemux = gst_bin_get_by_name(GST_BIN(pipeline), "rtpptdemux");
            g_signal_connect (rtpbin, "pad-added", G_CALLBACK (pad_added), this);
            g_signal_connect (rtpbin, "request-pt-map", G_CALLBACK (request_pt_map), NULL);
            g_signal_connect (rtpbin, "on-new-ssrc", G_CALLBACK (on_new_ssrc), NULL);
            // g_signal_connect (rtpptdemux, "new-payload-type", G_CALLBACK (new_payload_type), nullptr);

            GstBus * bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
            if(bus != nullptr){
                gst_bus_add_watch (bus, GstreamerCore::bus_callback, this);
                gst_object_unref (bus);
            }
        }

        void
        ClearPlayList(){
            filelist.clear();
            current_file_index = -1;

            GstElement * audiotestsrc = gst_bin_get_by_name(GST_BIN(pipeline), "audiotestsrc");
            pm.Logger().info("ClearPlayList.. audiotestsrc : {}", (void *)audiotestsrc);
            if(audiotestsrc == nullptr){ // filesrc should be cleared 
                gst_element_set_state (pipeline, GST_STATE_READY);

                GstElement * filesrc = gst_bin_get_by_name(GST_BIN(pipeline), "filesrc");
                GstElement * wavparse = gst_bin_get_by_name(GST_BIN(pipeline), "wavparse");
                GstElement * alawdec = gst_bin_get_by_name(GST_BIN(pipeline), "alawdec");
                GstElement * audioconvert = gst_bin_get_by_name(GST_BIN(pipeline), "audioconvert");

                gst_bin_remove_many(GST_BIN(pipeline), filesrc, wavparse, alawdec, NULL);

                gst_element_set_state(filesrc, GST_STATE_NULL);
                gst_element_set_state(wavparse, GST_STATE_NULL);
                gst_element_set_state(alawdec, GST_STATE_NULL);
                
                gst_object_unref(filesrc);
                gst_object_unref(wavparse);
                gst_object_unref(alawdec);

                audiotestsrc = gst_element_factory_make("audiotestsrc", "audiotestsrc");
                g_object_set (audiotestsrc, "wave", 4, NULL);//4


                gst_bin_add(GST_BIN (pipeline), audiotestsrc);
                if (!gst_element_link(audiotestsrc, audioconvert)){
                    g_error("fail link audiotestsrc audioconvert\n");
                }

                GstElement * rtpamrpay = gst_bin_get_by_name(GST_BIN(pipeline), "rtppay");

                GstRTPBasePayload *rtpbasepayload = GST_RTP_BASE_PAYLOAD (rtpamrpay);
                guint32 new_timestamp = rtpbasepayload->timestamp;
                guint16 new_seq = rtpbasepayload->seqnum;
                g_object_set(rtpamrpay, "timestamp-offset", new_timestamp, "seqnum-offset", new_seq, NULL);

                gst_element_set_state(pipeline, GST_STATE_PLAYING);

            }
        }
        void
        AppandPlayList(std::vector<std::string> & list){
            for(std::string & file : list){
                
                auto old = std::string{"/home/sta/"};
                auto old_pos = file.find(old);
                file.replace(old_pos, old.length(), "/home/jdin/");

                filelist.push_back(file);
            }   
        }
        void
        PlayFile(){
            std::cout << "PlayFile start.. current_file_index : " << current_file_index << ", filelist.size():" << filelist.size() << std::endl;
            if(current_file_index < 0 && filelist.size() > 0){
                current_file_index = 0;

                gst_element_set_state(pipeline, GST_STATE_READY);

                GstElement * audiotestsrc = nullptr;
                GstElement * filesrc = nullptr;
                GstElement * wavparse = nullptr;
                GstElement * alawdec = nullptr;

                audiotestsrc = gst_bin_get_by_name(GST_BIN(pipeline), "audiotestsrc");
                if(audiotestsrc != nullptr){
                    gst_bin_remove(GST_BIN(pipeline), audiotestsrc);

                    gst_element_set_state(audiotestsrc, GST_STATE_NULL);

                    gst_object_unref(audiotestsrc);

                    filesrc = gst_element_factory_make("filesrc", "filesrc");
                    g_object_set (filesrc, "location", filelist[current_file_index].c_str(), NULL);
                    wavparse = gst_element_factory_make("wavparse", "wavparse");
                    alawdec = gst_element_factory_make("alawdec", "alawdec");

                    GstElement * audioconvert = gst_bin_get_by_name(GST_BIN(pipeline), "audioconvert");

                    gst_bin_add_many(GST_BIN (pipeline), filesrc, wavparse, alawdec, NULL);
                    if (!gst_element_link_many (filesrc, wavparse, alawdec, audioconvert, NULL)) {
                        g_error ("Failed to link audiosrc, audioconv, audioresample, "
                            "audio encoder and audio payloader");
                    }

                    g_print("file:%s\n", filelist[current_file_index].c_str());

                    GstElement * rtpamrpay = gst_bin_get_by_name(GST_BIN(pipeline), "rtppay");

                    GstRTPBasePayload *rtpbasepayload = GST_RTP_BASE_PAYLOAD (rtpamrpay);
                    guint32 new_timestamp = rtpbasepayload->timestamp;
                    guint16 new_seq = rtpbasepayload->seqnum;
                    g_object_set(rtpamrpay, "timestamp-offset", new_timestamp, "seqnum-offset", new_seq, NULL);

                    gst_element_set_state(pipeline, GST_STATE_PLAYING);

                }
                else{
                }
            }
        }
        void
        Remove(){
            gst_element_set_state (pipeline, GST_STATE_NULL);
            gst_object_unref (pipeline);
        }
        void
        GstPlay(){
           gst_element_set_state(pipeline, GST_STATE_PLAYING) ;
        }
        void
        GstReady(){
           gst_element_set_state(pipeline, GST_STATE_READY) ;
        }
        void
        GstPaused(){
           gst_element_set_state(pipeline, GST_STATE_PAUSED) ;
        }

        void
        GstClear()
        {
            gst_element_set_state (pipeline, GST_STATE_READY);

            GstElement * audiotestsrc = gst_bin_get_by_name(GST_BIN(pipeline), "audiotestsrc");
            pm.Logger().info("GstClear.. audiotestsrc : {}", (void *)audiotestsrc);
            if(audiotestsrc == nullptr){
                GstElement * filesrc = gst_bin_get_by_name(GST_BIN(pipeline), "filesrc");
                GstElement * wavparse = gst_bin_get_by_name(GST_BIN(pipeline), "wavparse");
                GstElement * alawdec = gst_bin_get_by_name(GST_BIN(pipeline), "alawdec");
                GstElement * audioconvert = gst_bin_get_by_name(GST_BIN(pipeline), "audioconvert");

                gst_bin_remove_many(GST_BIN(pipeline), filesrc, wavparse, alawdec, NULL);

                gst_element_set_state(filesrc, GST_STATE_NULL);
                gst_element_set_state(wavparse, GST_STATE_NULL);
                gst_element_set_state(alawdec, GST_STATE_NULL);

                gst_object_unref(filesrc);
                gst_object_unref(wavparse);
                gst_object_unref(alawdec);

                audiotestsrc = gst_element_factory_make("audiotestsrc", "audiotestsrc");
                g_object_set (audiotestsrc, "wave", 4, NULL);//4

                gst_bin_add(GST_BIN (pipeline), audiotestsrc);
                if (!gst_element_link(audiotestsrc, audioconvert)){
                    g_error("fail link audiotestsrc audioconvert\n");
                }

                GstElement * rtpamrpay = gst_bin_get_by_name(GST_BIN(pipeline), "rtppay");

                GstRTPBasePayload *rtpbasepayload = GST_RTP_BASE_PAYLOAD (rtpamrpay);
                guint32 new_timestamp = rtpbasepayload->timestamp;
                guint16 new_seq = rtpbasepayload->seqnum;
                g_object_set(rtpamrpay, "timestamp-offset", new_timestamp, "seqnum-offset", new_seq, NULL);

                gst_element_set_state(pipeline, GST_STATE_PLAYING);

            }
        }
        static
        void 
        on_new_ssrc (GstElement* object, guint arg0, guint arg1, gpointer user_data){
            std::cout << "on_new_ssrc arg0:" << arg0 << ", arg1:" << arg1 << std::endl;
            
        }

        static
        void
        pad_added (GstElement* object, GstPad* new_pad, gpointer user_data){
            GstreamerCore * core = (GstreamerCore *)user_data;
            std::cout << "pad_added arg0 : "  << new_pad << std::endl;

            GstPad *sinkpad;
            GstPadLinkReturn lres;

            g_print ("new payload on pad: %s\n", GST_PAD_NAME (new_pad));
            GstCaps* caps = gst_pad_get_current_caps(new_pad);
            gchar* caps_str = gst_caps_to_string(caps);
            g_print("caps: %s\n", caps_str);
            GstStructure* stru = gst_caps_get_structure(caps, 0);
            gint payload = 0;
            gst_structure_get_int(stru, "payload", &payload);

            std::cout << "payload : " << payload << std::endl;

            if (payload == 8 /* PCMA */)  {
                // sinkpad = gst_element_get_static_pad(depay, "sink");
                // g_assert(sinkpad);
            } else if (payload == 101 /* TELEPHONE-EVENT */)  {
                GError *err = 0;

                GstElement* rtpdtmfdepay = gst_bin_get_by_name(GST_BIN(core->pipeline), "rtpdtmfdepay");
                GstPad* sinkpad = gst_element_get_static_pad (rtpdtmfdepay, "sink");

                lres = gst_pad_link(new_pad, sinkpad);
                g_assert(lres == GST_PAD_LINK_OK); // Crashed when payload == TELEPHONE-EVENT
                gst_object_unref(sinkpad);



                // /* I try to use rtpdtmfdepay:  */
                // GstElement* dtmf_sink = gst_parse_bin_from_description(
                //             "rtpdtmfdepay !  fakesink", 1, &err);
                // g_assert(sinkpad);
                // gst_bin_add(GST_BIN(core->pipeline), dtmf_sink);
                // sinkpad = gst_element_get_static_pad(dtmf_sink, "sink");
            }


        }
        static
        GstCaps * 
        request_pt_map (GstElement* object, guint arg0, guint payload, gpointer user_data){
            if(payload == 104){
                GstCaps * caps = gst_caps_from_string("application/x-rtp, media=(string)AMR, encoding-params=(string)1, octet-align=(string)1, payload=(int)104");
                return caps;
            }
            else if(payload == 101){
                GstCaps * caps = gst_caps_from_string("application/x-rtp,media=(string)audio,clock-rate=(int)8000,encoding-name=TELEPHONE-EVENT,payload=(int)101");
                return caps;
            }
            return NULL;
            
        }
        static
        GstCaps * 
        new_payload_type (GstElement* object, guint arg0, GstPad* pad, gpointer user_data){
            std::cout << "new_payload_type arg0:" << arg0 << std::endl;
            return NULL;
        }

        static
        gboolean
        bus_callback (GstBus * bus, GstMessage * message, gpointer data)
        {
            GstreamerCore * core = (GstreamerCore *)data;
            GstElement * pipeline = core->pipeline;
            auto bus_callback_id = std::this_thread::get_id();
            // std::cout << "]############# Got bust... " << bus_callback_id << std::endl;
            // g_print ("############# Got %s message.. src:%s\n", GST_MESSAGE_TYPE_NAME (message), gst_element_get_name (message->src));

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
                    if(core->current_file_index + 1 == core->filelist.size()){
                        core->GstClear();
                    }
                    else {
                        gst_element_set_state(pipeline, GST_STATE_READY);
                        (*core).current_file_index += 1;
                        GstElement * filesrc = gst_bin_get_by_name(GST_BIN(pipeline), "filesrc");
                        g_object_set (filesrc, "location", core->filelist[core->current_file_index].c_str(), NULL);

                        GstElement * rtpamrpay = gst_bin_get_by_name(GST_BIN(pipeline), "rtppay");

                        GstRTPBasePayload *rtpbasepayload = GST_RTP_BASE_PAYLOAD (rtpamrpay);
                        guint32 new_timestamp = rtpbasepayload->timestamp;
                        guint16 new_seq = rtpbasepayload->seqnum;
                        // TODO AMR은 160으로 gstreamer에 하드코딩 되어있음.  https://github.com/GStreamer/gst-plugins-good/blob/master/gst/rtp/gstrtpamrpay.c, line:348
                        // TODO 각 코덱 별로 값을 찾아 정의해줘야 할듯..
                        g_object_set(rtpamrpay, "timestamp-offset", new_timestamp + 160, "seqnum-offset", new_seq + 1, NULL);

                        gst_element_set_state(pipeline, GST_STATE_PLAYING);
                    }
                    break;
                }
                case GST_MESSAGE_ELEMENT: //gstdtmfdemay event..
                {
                    std::cout << "dtmf " << std::endl;
                    const GstStructure * structure = gst_message_get_structure(message);

                    auto cb = [](GQuark field, const GValue *value, gpointer user_data) -> gboolean {
                        GstreamerCore * core = (GstreamerCore *)user_data;
                        GstElement * pipeline = core->pipeline;


                        gchar *str = (char *)gst_value_serialize (value);
                        const char* fieldname = g_quark_to_string (field);
                        if(strcmp(fieldname, "number") == 0){
                            auto response = web::json::value::object();
                            response["tool_inf"]["inf_type"] = web::json::value::string("event");
                            response["tool_inf"]["tool_id"] = web::json::value::number(core->tool_id);
                            response["tool_inf"]["data"]["type"] = web::json::value::string("RTP_event_detected");
                            response["tool_inf"]["data"]["event"] = web::json::value::string(std::string{"DTMF"} + std::string{str});

                            auto rsp = response.serialize();
                            uint32_t rsp_len = rsp.length();
                            std::vector<boost::asio::const_buffer> buffers;
                            buffers.push_back(boost::asio::buffer(&rsp_len, sizeof(rsp_len)));
                            buffers.push_back(boost::asio::buffer(rsp, rsp_len));

                            boost::asio::async_write(core->sock, buffers, 
                                [](const boost::system::error_code & error, size_t len){
                                    if(error){
                                        return;
                                    }
                                });
                            
                        }
                        g_print ("%15s: %s\n", fieldname, str);
                        g_free (str);
                        return (gboolean)TRUE;
                    };
                    gst_structure_foreach(structure, cb, core);

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