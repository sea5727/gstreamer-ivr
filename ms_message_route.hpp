#ifndef __MS_ROUTE_JOB_HPP__
#define __MS_ROUTE_JOB_HPP__

#define MAX_CORE_COUNT 16
#define MAX_HMP_COUNT 1



#include <vector>
#include <string>
#include <boost/asio.hpp>

using ptr_io_service = std::shared_ptr<boost::asio::io_service>;

class MessageRouter
{
public:
    class ROUTING_TABLE
    {
    public:
        enum TABLE : int
        {
            CORE = 0,
            HMP = 1,
            MAX_MODULES = 2,
        };
        ROUTING_TABLE() = default;
        constexpr ROUTING_TABLE(TABLE table) : _table(table) { }

        operator TABLE() const { return _table; }
        explicit operator bool() = delete;
        constexpr bool operator==(ROUTING_TABLE cmp) const { return _table == cmp._table; }
        constexpr bool operator!=(ROUTING_TABLE cmp) const { return _table != cmp._table; }


    private:
        TABLE _table;
    };

public:
    MessageRouter()
        : _count_of_modules(ROUTING_TABLE::MAX_MODULES)
        , _routing_tables(_count_of_modules)
        , _num_of_modules(_count_of_modules, 0)
    {
    }


    void set_route_table(ROUTING_TABLE table_num, std::vector<ptr_io_service> & route_container)
    {
        if(table_num >= _count_of_modules ) {
            std::cout << "invlaid table_num.." << std::endl;
            return; // TODO FAIL LOGIC
        }

        _routing_tables[table_num].clear();
        _num_of_modules[table_num] = route_container.size();
        for(auto route : route_container)
        {
            std::cout << "_routing_tables push_back route.. table_num : " << table_num<< std::endl;
            _routing_tables[table_num].push_back(route);
        }
    }

    ptr_io_service get_route_io(ROUTING_TABLE table_num, int index)
    {
        std::cout << "get_route_io.. _num_of_modules[table_num] : " << table_num << ".. " << _num_of_modules[table_num] << std::endl;
        if(_num_of_modules[table_num] == 0)
            return nullptr;

        std::cout << "size is " << _routing_tables[table_num].size() << std::endl;
        if(_num_of_modules[table_num] == 1)
            return _routing_tables[table_num][0];
            
        if(_routing_tables[table_num].size() >= index) 
            index = index % _routing_tables[table_num].size();

        return _routing_tables[table_num][index];
    }
private:
    int _count_of_modules;

    std::vector<int>                        _num_of_modules;
    std::vector<std::vector<ptr_io_service>>   _routing_tables;
    // std::vector<std::vector<std::string>>   _routing_tables;
    
};

#endif