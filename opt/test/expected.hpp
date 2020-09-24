#ifndef __EXPECTED_HPP__
#define __EXPECTED_HPP__

#include <exception>
#include <stdexcept>
#include <utility>
#include <typeinfo>

template <class T>
class Expected {
    union {
        T value;
        std::exception_ptr ex_ptr;
    };
    bool got_value;
    Expected() {}
public:
    Expected(const T& rhs) : value(rhs), got_value(true) {}
    Expected(T&& rhs) 
        : value(std::move(rhs))
        , got_value(true) {}
    Expected(const Expected& rhs) : got_value(rhs.got_value) {
        if(got_value) new (&value) T(rhs.value);
        else new(&ex_ptr) std::exception_ptr(rhs.ex_ptr);
    }
    Expected(Expected&& rhs) : got_value(rhs.got_value) {
        if(got_value) new (&value) T(std::move(rhs.value));
        else new(&ex_ptr) 
            std::exception_ptr(std::move(rhs.ex_ptr));
    }
    ~Expected()
    {
        using std::exception_ptr;
        if (got_value) value.~T();
        else ex_ptr.~exception_ptr();
    }

    void swap(Expected & rhs)
    {
        if(got_value)
        {
            if(rhs.got_value)
            {
                using std::swap;
                swap(value, rhs.value);
            }
            else
            {
                auto t = std::move(rhs.ex_ptr);
                new(&rhs.value) T(std::move(value));
                new(&ex_ptr) std::exception_ptr(t);
                std::swap(got_value, rhs.got_value);
            }
            
        }
        else
        {
            if(rhs.got_value)
                rhs.swapt(*this);
            else
            {
                ex_ptr.swap(rhs.ex_ptr);
                std::swap(got_value, rhs.got_value);
            }
        }
        
    }
    template<class E>
    static Expected<T> fromException(const E& exception)
    {
        if(typeid(exception) != typeid(E))
        {
            throw std::invalid_argument("slicing detected");
        }
        return fromException(std::make_exception_ptr(exception));
    }
    static Expected<T> fromException(std::exception_ptr p)
    {
        Expected<T> result;
        result.got_value = false;
        new(&result.ex_ptr) std::exception_ptr(std::move(p));
        return result;
    }
    static Expected<T> fromException() {
        return fromException(std::current_exception());
    }

    bool has_error() const {
        return !got_value;
    }

    bool is_valid() const {
        return got_value;
    }
    
    T& get() {
        if(!got_value) std::rethrow_exception(ex_ptr);
        return value;
    }
    const T& get() const {
        if(!got_value) std::rethrow_exception(ex_ptr);
        return value;
    }
    template<class E>
    bool hasException() const {
        try {
            if(!got_value) std::rethrow_exception(ex_ptr);
        }
        catch(const E& object)
        {
            return true;
        }
        catch(...)
        {

        }
        return false;
    }

    
};

#endif