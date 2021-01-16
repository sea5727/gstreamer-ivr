#pragma once

#include "ms_include.hpp"

namespace MediaServer
{

class HmpManager{
    HmpManager() = default;
public:

    std::array<web::json::value, 10000> tool_req_list;

    static
    HmpManager &
    getInstance(){
        static HmpManager * instance = nullptr;
        if(instance == nullptr){
            instance = new HmpManager;
        }
        return * instance;
    }
};

} // namespace MediaServer