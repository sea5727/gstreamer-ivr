#include "configure.hpp"
#include <exception>
#include <type_traits>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <thread>
#include <getopt.h>
#include <wordexp.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "configure.hpp"
#include "utils.hpp"

using std::string;
using std::stringstream;
using oop::config::Configure;

Configure::Configure(int argc, char *argv[]) 
    : _argc(argc)
    , _argv(argv, argv + argc)
    ,_config_map()
{

}

Configure::~Configure() 
{

}

string 
Configure::operator()(
    string key, 
    const char * default_value)
{
    string value = _config_map.get<string>(key.c_str(), default_value);
    return change_environment_variable(value);
}

void
Configure::print_help()
{
    printf("This process : %s\n", this->operator()("CMD.NAME", "NONAME").c_str());
    printf("-h [ --help ]                           show help\n");
    printf("-c [--config-file]                      process configuration file path(.ini) \n");
    printf("-a [--active]                           only manually active \n");
    printf("-s [--standby]                          only manually standby.. unless --active \n");
}

void 
Configure::get_commandline_parameter()
{
    boost::filesystem::path p(_argv[0].c_str());
    string name = p.stem().string(); // get file name ( = process name )

    string config_file;
    static struct option long_option[] = //default option
    {
        // ? no_argument         : --active 처럼 value 를 가지지 않음..
        // ? required_argument   : --config-file value, --config-file=value 로 값을 가짐.. ???? 뭔소린지..
        // ? optional_argument   : --config-file=value 처럼만 값을 가짐.. ???? 뭔소린지
        {"config-file", required_argument, 0, 'c'},
        {"active", no_argument, 0, 'a'},
        {"standby", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int help_flag = 0;
    int active_flag = 0;
    int standby_flag = 0;

    int opt = 0;

    
    while((opt = getopt_long(_argc, reinterpret_cast<char**>(_argv.data()), "c:ash", long_option, &option_index)) != -1)
    // while((opt = getopt_long(argc, argv, "c:ash", long_option, &option_index)) != -1)
    {
        switch(opt)
        {
            case 0: break;
            case 'c' : //config-file
            {
                config_file = optarg;
            }
                break;
            case 'a' : 
                active_flag = 1;
                break;
            case 's' : 
                standby_flag = 1;
                break;
            case 'h' : 
                help_flag = 1;
                break;
            case 'n':
                break;
            default:
                break;
        }
    }
    // ! config-file 없이 default나 env variable 값으로 기동할수도 있지만, 명시적인 환경을 받아 수행하도록 함.
    if(config_file.empty())
    {
        // throw jdin::oop::exception::RuntimeException("--config-file is required");
    }

    _config_map.add("CMD.NAME" , change_environment_variable(name));
    _config_map.add("CMD.CONFIG_FILE", change_environment_variable(config_file));
    _config_map.add("CMD.ACTIVE", active_flag);
    _config_map.add("CMD.STANDBY", standby_flag);
    _config_map.add("CMD.HELP", help_flag);

}



void
Configure::load_configfile(std::string file_path) throw()
{
    if(!is_config_file_exist(file_path.c_str()))
    {
        throw std::runtime_error("no exist file : " + file_path);
    }

    boost::property_tree::ptree temp_ptree;
    boost::filesystem::path p(file_path);
    if(p.extension().string() == ".ini")
    {
        boost::property_tree::read_ini(file_path, temp_ptree);
    }
    else if(p.extension().string() == ".json")
    {
        boost::property_tree::read_json(file_path, temp_ptree);
    }
    else if(p.extension().string() == ".xml")
    {
        boost::property_tree::read_xml(file_path, temp_ptree);
    }
    else 
    {
        throw std::runtime_error("invalid file format : " + p.extension().string());
    }

    for(auto & conf : temp_ptree)
    {
        _config_map.put_child(conf.first, conf.second);
    }

}
