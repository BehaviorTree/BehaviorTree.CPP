#ifndef SIMPLE_STRING_HPP
#define SIMPLE_STRING_HPP

#include <string>
#include <cstring>

namespace SafeAny
{
// Version of string that uses only two words. Good for small object optimization in linb::any
class SimpleString
{
  public:
    SimpleString(const std::string& str) : SimpleString(str.data(), str.size())
    {
    }
    SimpleString(const char* input_data) : SimpleString(input_data, strlen(input_data))
    {
    }

    SimpleString(const char* input_data, std::size_t size) : _size(size)
    {
        if(size >= sizeof(void*) )
        {
            _data.ptr = new char[_size + 1];
        }
        std::memcpy(data(), input_data, _size);
        data()[_size] = '\0';
    }

    SimpleString(const SimpleString& other) : SimpleString(other.data(), other.size())
    {
    }

    ~SimpleString()
    {
        if ( _size >= sizeof(void*) && _data.ptr )
        {
            delete[] _data.ptr;
        }
    }

    std::string toStdString() const
    {
        return std::string(data(), _size);
    }

    const char* data() const
    {
        if( _size >= sizeof(void*))
        {
            return _data.ptr;
        }
        else{
            return _data.soo;
        }
    }

    char* data()
    {
        if( _size >= sizeof(void*))
        {
            return _data.ptr;
        }
        else{
            return _data.soo;
        }
    }

    std::size_t size() const
    {
        return _size;
    }

  private:
    union{
        char*  ptr;
        char   soo[sizeof(void*)] ;
    }_data;

    std::size_t _size;
};

}

#endif   // SIMPLE_STRING_HPP
