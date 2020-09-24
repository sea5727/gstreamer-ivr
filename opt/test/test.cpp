#include <stdio.h>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <stdexcept>
#include "expected.hpp"

using namespace std;

void proc(std::vector<string> & v, int i)
{
    v.at(i) = to_string(i);
}
void test1()
{
    throw std::runtime_error("testerror");
}
void test2()
{
    test1();
}
void test3()
{
    test2();
}
void test4()
{
    test3();
}

void proc_with_throw1(std::vector<string> & v, int i)
{
    throw std::runtime_error("testerror");
}

void proc_with_throw2(std::vector<string> & v, int i)
{
    test4();
}


Expected<int> proc_with_expected(std::vector<string> & v, int i)
{
    v.at(i) = to_string(i);
    return 0;
}

Expected<int> proc_with_expected_throw(std::vector<string> & v, int i)
{
    return Expected<int>::fromException(std::invalid_argument("not a number"));
    v.at(i) = to_string(i);
}
int main(int argc, char *argv[])
{

    
    
    printf("hello world\n");

    auto start = std::chrono::steady_clock::now();

    std::vector<string> v(100000);

    for(int i = 0 ; i < 100000 ; i++)
    {
        try
        {
            if(i >= 50000)
                proc_with_throw2(v, i);
            // if(i >= 50000)
            //     goto TEST;
            // if(i >= 50000)
            //     proc_with_expected_throw(v, i);
            proc_with_expected(v, i);
        }
        catch(const std::exception& e)
        {
            // std::cerr << e.what() << '\n';
            
            proc_with_expected(v, i);
        }

        continue;

        TEST:
        proc_with_expected(v, i);
        
    }

    auto end = std::chrono::steady_clock::now();

    cout << std::chrono::duration_cast<chrono::microseconds>(end - start).count() << "ms\n";
}