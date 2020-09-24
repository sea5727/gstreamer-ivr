#ifndef __MS_HMP_SERVICE_HPP__
#define __MS_HMP_SERVICE_HPP__

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include <sstream>

using namespace rapidjson;

class HmpService
{
public:
    class ERROR_CODE
    {
    public:
        enum CODE : int
        {
            OK = 0,
            TOOL_DOES_NOT_EXIST = 1001,

        };
    
        ERROR_CODE() = default;
        constexpr ERROR_CODE(CODE code) : error_code(code) { }

        operator CODE() const { return error_code; }
        explicit operator bool() = delete;
        constexpr bool operator==(ERROR_CODE cmp) const { return error_code == cmp.error_code; }
        constexpr bool operator!=(ERROR_CODE cmp) const { return error_code != cmp.error_code; }

        std::string to_string()
        {
            switch(error_code)
            {
                case OK: return std::string("OK");
                case TOOL_DOES_NOT_EXIST :  return std::string("tool does not exist");
            }
            return std::string("undefined error code");
        }
        std::string to_desc()
        {
            if(error_code == OK)
                return to_string();
            return std::string("SurfHMP Error: ") + to_string();
        }

    private:
        CODE error_code;
    };

public:
    HmpService();
    ~HmpService();


    static bool get_connect_message(StringBuffer & s)
    {
        Writer<StringBuffer> w(s);
        auto result = 
            w.StartObject()     &&
            w.Key("connect")    &&
            w.StartObject()     &&
            w.Key("api_version")&&
            w.StartArray()      &&
            w.Uint(1)           &&
            w.Uint(2)           &&
            w.EndArray()        &&
            w.EndObject()       &&
            w.EndObject();

        return result;
    }

    static bool get_tool_ans_message(StringBuffer & s, int tool_id, int req_id, const char *req_type, int error_code, const char * description)
    {
        Writer<StringBuffer> w(s);
        auto result = 
            w.StartObject()                                 &&
            w.Key("tool_ans")                               &&
            w.StartObject()                                 &&
            w.Key("req_id")                                 &&
            w.Uint(req_id)                                 &&
            w.Key("tool_id")                                &&
            w.Uint(tool_id)                                     &&
            w.Key("req_type")                               &&
            w.String(req_type)                             &&
            w.Key("data")                                   &&
            w.StartObject()                                 &&
            w.Key("error_code")                             &&
            w.Uint(error_code)                                    &&
            w.Key("description")                            &&
            w.String(description)  &&
            w.EndObject()                                   &&
            w.EndObject()                                   &&
            w.EndObject();

        return result;
    }

};
#endif