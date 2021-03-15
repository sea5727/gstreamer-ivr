#include "ms_include.hpp"

int main(int argc, char* argv[]) {
    gst_init (&argc, &argv);

    auto & manager = MediaServer::Manager::getInstance();

    std::vector<std::thread> workers;
    
    auto tcpio = boost::asio::io_service{1};
    auto tcpip_work = boost::asio::io_service::work{tcpio};

    auto tcpserver = MediaServer::TcpListener<MediaServer::HmpServer>{tcpio, 9000};

    workers.emplace_back([&tcpio]{
        tcpio.run();
    });

    tcpserver.run();


    workers.emplace_back([]{
        GMainLoop *loop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (loop);
    });

    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
