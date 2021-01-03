#pragma once

#include "ms_include.hpp"

namespace MediaServer
{
    class Manager{
    private:
        std::shared_ptr<spdlog::logger> logger;
        std::string name = "MediaServer";
        Manager() {
            try{
                logger = spdlog::basic_logger_mt("MediaServer", "log/MediaServer.log");
                spdlog::flush_on(spdlog::level::debug);
                logger->set_level(spdlog::level::debug);
                logger->info("======== {} START ===========", name);
            }
            catch(const spdlog::spdlog_ex & ex){
                std::cout << "Exception : " << ex.what() << std::endl;
            }
        }
    public:
        static  
        Manager &
        getInstance(){
            static Manager * instance = nullptr;
            if(instance == nullptr)
                instance = new Manager;
            return *instance;
        }

        spdlog::logger &
        Logger(){
            return *logger;
        }


    };
}