#include "liboops/configure.hpp"
#include "ms_memory.hpp"
// #include "ms_hmp_interface.hpp"
// #include "ms_hmp_server.hpp"
// #include "ms_core.hpp"
// #include "ms_message_route.hpp"
// #include "ms_core_gst_builder.hpp"
#include "ms_http_interface.hpp"
// #include "ms_thread_pool.hpp"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <gst/gst.h>
#include <iostream>
#include <functional>

#include <cpprest/http_listener.h>
#include <cpprest/json.h>



/* change this to send the RTP data and RTCP to another host */
#define DEST_HOST "127.0.0.1"

/* #define AUDIO_SRC  "alsasrc" */
#define AUDIO_SRC  "audiotestsrc"

/* the encoder and payloader elements */
#define AUDIO_ENC  "alawenc"
#define AUDIO_PAY  "rtppcmapay"

static void
print_source_stats (GObject * source);

static gboolean
print_stats (GstElement * rtpbin);

void 
proc_demo(const char *host);


// _TOOL	_HMP_TOOLS[MAX_TOOL];

GMainLoop *loop;
unsigned int _current_mem_index = 0;
_ST_PIPELINE    _PIPELINE_MEM[3000];
_ST_DATA_       _DATA_MEM[3000];

int main(int argc, char* argv[]) {

    gst_init (&argc, &argv);
    
    
    oop::config::Configure config = oop::config::Configure(argc, argv);
    config.get_commandline_parameter();
    if(config("CMD.HELP", 1))
    {
        config.print_help();
        return 0;
    }

    auto config_file_path = config("CMD.CONFIG_FILE", "~/config/media-server.ini");
    if(config_file_path.size() == 0)
    {
        std::cerr << "This program requires a config file..\n";
        std::cerr << "you can input --help \n";
        return 0;
    }

    config.load_configfile(config("CMD.CONFIG_FILE", "~/config.ini"));


    // boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::info );



    std::cout << "start http server\n";


    MsCore::MsCore::init();
    
    
    // TcpGenericServer<HmpInterface> hmp_server(1);
    // hmp_server.tcp_listen(9000);
    // hmp_server.run();
    
    
    // MsCore ms_core;
    // message_router.set_route_table(MessageRouter::ROUTING_TABLE::CORE, ms_core.get_route_container());
    // ms_core.run();



    std::cout << "start http server\n";
    std::string address = "http://0.0.0.0:8080/media";
    HttpInterface http_interface(address);
    http_interface.set_name("http");
    http_interface
        .open()
        .then([&](){
            std::cout << http_interface.get_addr().to_string() << " http_interface listen start!!!\n";
        })
        .wait();

    
    GMainLoop *loop = g_main_loop_new (NULL, FALSE);

    std::cout << "g_main_loop_run start\n" ;
    g_main_loop_run (loop);



    
}


/* print the stats of a source */
static void
print_source_stats (GObject * source)
{
    GstStructure *stats;
    gchar *str;

    /* get the source stats */
    g_object_get (source, "stats", &stats, NULL);

    /* simply dump the stats structure */
    str = gst_structure_to_string (stats);
    g_print ("source stats: %s\n", str);

    gst_structure_free (stats);
    g_free (str);
}

/* this function is called every second and dumps the RTP manager stats */
static gboolean
print_stats (GstElement * rtpbin)
{
    GObject *session;
    GValueArray *arr;
    GValue *val;
    guint i;

    g_print ("***********************************\n");

    /* get session 0 */
    g_signal_emit_by_name (rtpbin, "get-internal-session", 0, &session);

    /* print all the sources in the session, this includes the internal source */
    g_object_get (session, "sources", &arr, NULL);

    for (i = 0; i < arr->n_values; i++) {
        GObject *source;

        val = g_value_array_get_nth (arr, i);
        source = (GObject *)g_value_get_object (val);

        print_source_stats (source);
    }
    g_value_array_free (arr);

    g_object_unref (session);

    return TRUE;
}

void proc_demo(const char *host)
{
    GstElement* pipeline = gst_pipeline_new (NULL);
    g_assert (pipeline);
    /* the audio capture and format conversion */
    GstElement* audiosrc = gst_element_factory_make ("audiotestsrc", NULL);
    g_assert (audiosrc);
    GstElement* audioconv = gst_element_factory_make ("audioconvert", NULL);
    g_assert (audioconv);
    GstElement* audiores = gst_element_factory_make ("audioresample", NULL);
    g_assert (audiores);
    /* the encoding and payloading */
    GstElement* audioenc = gst_element_factory_make ("alawenc", NULL);
    g_assert (audioenc);
    GstElement* audiopay = gst_element_factory_make ("rtppcmapay", NULL);
    g_assert (audiopay);
    gst_bin_add_many (  GST_BIN (pipeline), 
                        audiosrc, 
                        audioconv, 
                        audiores,
                        audioenc, 
                        audiopay, 
                        NULL);

    if (!gst_element_link_many (audiosrc, 
                                audioconv, 
                                audiores, 
                                audioenc,
                                audiopay, 
                                NULL)) {
        g_error ("Failed to link audiosrc, audioconv, audioresample, "
            "audio encoder and audio payloader");
        return;
    }

    /* the rtpbin element */
    GstElement* rtpbin = gst_element_factory_make ("rtpbin", NULL);
    g_assert (rtpbin);

    gst_bin_add (GST_BIN (pipeline), rtpbin);


    /* the udp sinks and source we will use for RTP and RTCP */
    GstElement* rtpsink = gst_element_factory_make ("udpsink", NULL);
    g_assert (rtpsink);
    g_object_set (rtpsink, "port", 5002, "host", host, NULL);

    GstElement* rtcpsink = gst_element_factory_make ("udpsink", NULL);
    g_assert (rtcpsink);
    g_object_set (rtcpsink, "port", 5003, "host", host, NULL);
    /* no need for synchronisation or preroll on the RTCP sink */
    g_object_set (rtcpsink, "async", FALSE, "sync", FALSE, NULL);

    GstElement* rtcpsrc = gst_element_factory_make ("udpsrc", NULL);
    g_assert (rtcpsrc);
    g_object_set (rtcpsrc, "port", 5007, NULL);

    gst_bin_add_many (GST_BIN (pipeline), rtpsink, rtcpsink, rtcpsrc, NULL);

    /* now link all to the rtpbin, start by getting an RTP sinkpad for session 0 */
    GstPad* sinkpad = NULL;
    GstPad* srcpad = NULL;
    sinkpad = gst_element_get_request_pad (rtpbin, "send_rtp_sink_0");
    srcpad = gst_element_get_static_pad (audiopay, "src");
    GstPadLinkReturn ret = gst_pad_link (srcpad, sinkpad);

    if (ret != GST_PAD_LINK_OK)
        g_error ("Failed to link audio payloader to rtpbin, %d", ret);
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

    /* print stats every second */
    // g_timeout_add_seconds (1, (GSourceFunc) print_stats, rtpbin);

}


