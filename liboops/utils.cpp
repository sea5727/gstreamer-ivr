#include <string>
#include <sstream>
#include <wordexp.h>
#include <boost/filesystem.hpp>


// change environment_varibale to variiable has value
// 환경변수를 값을 가진 변수로 변경
std::string change_environment_variable(std::string input)
{
    wordexp_t exp;
    int offset = 0;
    u_int i;
    if (wordexp(input.c_str(), &exp, 0))
    {
        return input;
    }

    std::stringstream ss;

    for (i = 0; i < exp.we_wordc; i++)
    {
        if (i > 0)
            ss << " ";
        ss << exp.we_wordv[i];
    }
    return ss.str();
}


bool is_config_file_exist(const char *file_path)
{   
    if(boost::filesystem::is_regular_file(file_path))
    {
        return true;
    }
    else
    {
        return false;
    }
}