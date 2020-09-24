#ifndef __MS_SINGLETON_HPP__
#define __MS_SINGLETON_HPP__

template<class _Target>
class Singleton {
private:
    static _Target * _ptr;
protected:
    Singleton() = default;
    ~Singleton()
    {
        delete _ptr;
        _ptr = nullptr;
    }
public:
    static _Target& Instance()
    {
        if(_ptr == nullptr ) 
            _ptr = new _Target();
        return *_ptr;
    }

};

template<class _Target>
_Target * Singleton<_Target>::_ptr = nullptr;

#endif