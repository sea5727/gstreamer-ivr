#pragma once

#include <iostream>
#include <stdio.h>
#include <vector>
#include <thread>

#include <gst/gst.h>
#include <gst/rtp/gstrtpbasepayload.h>
#include <gio/gio.h>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cpprest/json.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "ms_manager.hpp"
#include "ms_hmp_manager.hpp"
#include "ms_gstreamer_core.hpp"
#include "ms_hmp_parser.hpp"

#include "ms_tcp_listener.hpp"
#include "ms_hmp_server.hpp"







namespace ms{

}