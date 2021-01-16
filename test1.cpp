#include <iostream>
#include <thread>
#include <memory>

class TestClass{
    std::string name;
public:
    TestClass() = default;
    TestClass(const std::string & name)
        : name{name} {
        std::cout << "TestClass Constructor" << std::endl;
    }
    TestClass(const TestClass & copy)
        : name{copy.name} {
        std::cout << "TestClass Copy Constructor" << std::endl;
    }
    TestClass(TestClass && move)
        : name{std::move(move.name)} {
        std::cout << "TestClass Move Constructor" << std::endl;
    }
    ~TestClass() {
        std::cout << "TestClass Destructor" << std::endl;
    }

    TestClass& operator=(const TestClass& other){
        this->name = other.name;
    }
};



int main(int argc, char * argv[]){
    std::cout << "start" << std::endl;

    std::array<TestClass, 100> myarray;

    TestClass a("a");

    myarray[0] = a;

    std::cout << "end" << std::endl;

    // myarray[0] = nullptr;

    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
