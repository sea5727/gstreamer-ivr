#ifndef __OOP_CONFIGURE_HPP__
#define __OOP_CONFIGURE_HPP__


#include <boost/lexical_cast.hpp>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <wordexp.h>


#define BOOST_BIND_GLOBAL_PLACEHOLDERS

using std::string;
using std::stringstream;

namespace oop
{
    namespace config
    {
        class Configure
        {
        private:
            int                                 _argc;
            std::vector<std::string>            _argv;
            boost::property_tree::ptree         _config_map; 
        public:
            static std::shared_ptr<Configure> make(int argc, char *argv[])
            {
                return std::make_shared<Configure>(argc, argv);
            }
            Configure(int argc, char *argv[]);
            ~Configure();

            void get_commandline_parameter();
            void load_configfile(string file_path) throw();

            // *** Access *** //
            void print_help();
            boost::property_tree::ptree & get_config_map() { return _config_map; } 

            std::string operator()(std::string key, const char * default_value);
            
            //searching key:value
            //return value / or default value
            template<typename T>
            T operator()(std::string key, T default_value)
            {
                return _config_map.get<T>(key.c_str(), default_value);
            }

               
        };
    }
}


#endif