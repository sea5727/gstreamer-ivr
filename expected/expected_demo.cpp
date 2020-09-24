#include "expected.hpp"
#include <iostream>
#include <string>

Expected<int> parseInt(const std::string &s)
{
    int result;
    if(s.compare("nodigit") == 0)
    {
        return Expected<int>::fromException(std::invalid_argument("not a number"));
    }

    if(s.compare("toomany") == 0)
    {
        return Expected<int>::fromException(std::out_of_range("overflow"));
    }
    return 100;
}

int main(int argc, char *argv[])
{
    std::string s1 = "1234";
    auto r1 = parseInt(s1);
    std::cout << "r1 : " << r1.get() << std::endl;

    std::string s2 = "nodigit";
    auto r2 = parseInt(s2);
    if(r2.has_error())
    {
        if(r2.hasException<std::invalid_argument>())
        {
            std::cout << "this is no digit" << std::endl;
        }
        else if(r2.hasException<std::out_of_range>())
        {
            std::cout << "this is out_of_range" << std::endl;
        }
        else
        {
            r2.get();
        }
        
    }
    // std::cout << "r1 : " << r2.get() << std::endl;

    std::string s3 = "toomany";
    auto r3 = parseInt(s3);
    if(r3.has_error())
    {
        
        if(r3.hasException<std::invalid_argument>())
        {
            std::cout << "this is no digit" << std::endl;
        }
        else if(r3.hasException<std::out_of_range>())
        {
            std::cout << "this is out_of_range" << std::endl;
        }
        else
        {
            r3.get();
        }
        
    }
    else
    {
        std::cout << "r3?" << std::endl;
    }
    
    // std::cout << "r1 : " << r3.get() << std::endl;

    return 0;

}
