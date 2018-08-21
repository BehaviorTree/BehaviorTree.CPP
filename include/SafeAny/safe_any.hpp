#ifndef SAFE_ANY_VARNUMBER_H
#define SAFE_ANY_VARNUMBER_H

#include <exception>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <string>
#include <cstring>
#include "any.hpp"

namespace SafeAny{

// Version of string that uses only two words. Good for small object optimization in linb::any
class SimpleString
{
public:
    SimpleString(const std::string& str): SimpleString(str.data(), str.size())
    { }
    SimpleString(const char* data): SimpleString( data, strlen(data) )
    { }

    SimpleString(const char* data, std::size_t size): _size(size)
    {
        _data = new char[_size+1];
        strncpy(_data, data, _size);
        _data[_size] = '\0';
    }

    SimpleString(const SimpleString& other): SimpleString(other.data(), other.size())
    { }


    ~SimpleString() {
        if(_data){
            delete[] _data;
        }
    }

    std::string toStdString() const
    {
        return std::string(_data, _size);
    }

    const char* data() const { return _data;}

    std::size_t size() const { return _size;}

private:
    char* _data;
    std::size_t _size;
};



class Any
{

public:

    Any() {}

    ~Any() = default;

    template<typename T> Any(const T& value) : _any(value)
    { }

    template<typename T> T convert( ) const;

    template<typename T> T extract( ) const
    {
        return linb::any_cast<T>(_any);
    }

    const std::type_info& type() const { return _any.type(); }

private:

    linb::any _any;
};

//----------------------------------------------
//specialization for std::string
template <> inline Any::Any(const std::string& str)
{
    _any = linb::any(SimpleString(str));
}

template <> inline std::string Any::extract() const
{
    return linb::any_cast<SimpleString>(_any).toStdString();
}

namespace details{


template <typename BoolCondition>
using EnableIf = typename std::enable_if< BoolCondition::value, void>::type;

template <typename T>
struct is_integer : std::integral_constant<bool, std::is_integral<T>::value
        && !std::is_same<T, bool>::value
        && !std::is_same<T, char>::value>
{};

template <typename From, typename To>
struct is_same_real : std::integral_constant<bool,
        std::is_same<From, To>::value
        && std::is_floating_point<To>::value >
{};


template <typename From, typename To>
struct is_safe_integer_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && is_integer<To>::value
        && sizeof(From) < sizeof(To)
&& std::is_signed<From>::value == std::is_signed<To>::value>
{};

template <typename T>
struct is_convertible_type
        : std::integral_constant<bool,
           std::is_integral<T>::value
        || std::is_floating_point<T>::value
        || std::is_same<T, bool>::value
        || std::is_same<T, char>::value
        || std::is_same<T, std::string>::value
        || std::is_same<T, SimpleString>::value>
{};

template <typename From, typename To>
struct float_conversion
        : std::integral_constant<bool, std::is_floating_point<From>::value
        && std::is_floating_point<To>::value && !std::is_same<From, To>::value >
{};

template <typename From, typename To>
struct unsigned_to_smaller_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && is_integer<To>::value
        && (sizeof(From) > sizeof(To))
&& !std::is_signed<From>::value
&& !std::is_signed<To>::value >
{};

template <typename From, typename To>
struct signed_to_smaller_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && is_integer<To>::value
        && (sizeof(From) > sizeof(To))
&& std::is_signed<From>::value
&& std::is_signed<To>::value >
{};

//---------------------------
template <typename From, typename To>
struct signed_to_smaller_unsigned_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && is_integer<To>::value
        && sizeof(From) >= sizeof(To)
&& std::is_signed<From>::value
&& !std::is_signed<To>::value >
{};

template <typename From, typename To>
struct signed_to_larger_unsigned_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && is_integer<To>::value
        && sizeof(From) < sizeof(To)
&& std::is_signed<From>::value
&& !std::is_signed<To>::value >
{};

template <typename From, typename To>
struct unsigned_to_smaller_signed_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && is_integer<To>::value
        && (sizeof(From) >= sizeof(To))
&& !std::is_signed<From>::value
&& std::is_signed<To>::value >
{};

template <typename From, typename To>
struct unsigned_to_larger_signed_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && is_integer<To>::value
        && sizeof(From) < sizeof(To)
&& !std::is_signed<From>::value
&& std::is_signed<To>::value >
{};

template <typename From, typename To>
struct floating_to_signed_conversion
        : std::integral_constant<bool, std::is_floating_point<From>::value
        && is_integer<To>::value
        && std::is_signed<To>::value >
{};

template <typename From, typename To>
struct floating_to_unsigned_conversion
        : std::integral_constant<bool, std::is_floating_point<From>::value
        && is_integer<To>::value
        && !std::is_signed<To>::value >
{};

template <typename From, typename To>
struct integer_to_floating_conversion
        : std::integral_constant<bool, is_integer<From>::value
        && std::is_floating_point<To>::value >
{};

template <typename From, typename To>
inline void checkUpperLimit(const From& from)
{
    if ((sizeof(To) < sizeof(From)) &&
            (from > static_cast<From>(std::numeric_limits<To>::max()))) {
        throw std::runtime_error("Value too large.");
    }
    else if (static_cast<To>(from) > std::numeric_limits<To>::max()) {
        throw std::runtime_error("Value too large.");
    }
}

template <typename From, typename To>
inline void checkUpperLimitFloat(const From& from)
{
    if (from > std::numeric_limits<To>::max()){
        throw std::runtime_error("Value too large.");
    }
}

template <typename From, typename To>
inline void checkLowerLimitFloat(const From& from)
{
    if (from < -std::numeric_limits<To>::max()){
        throw std::runtime_error("Value too small.");
    }
}

template <typename From, typename To>
inline void checkLowerLimit(const From& from)
{
    if (from < std::numeric_limits<To>::min()){
        throw std::runtime_error("Value too small.");
    }
}

template <typename From, typename To>
inline void checkTruncation(const From& from)
{
    if( from != static_cast<From>(static_cast<To>( from))){
        throw std::runtime_error("Floating point truncated");
    }
}


//----------------------- Implementation ----------------------------------------------

template<typename SRC,typename DST> inline
typename std::enable_if< !is_convertible_type<DST>::value, void>::type
convert_impl( const SRC& , DST&  )
{
    throw std::runtime_error("Not convertible");
}


template<typename SRC,typename DST> inline
EnableIf< std::is_same<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    target = from;
}

template<typename SRC,typename DST> inline
EnableIf< is_safe_integer_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    target = static_cast<DST>( from);
}

template<typename SRC,typename DST> inline
EnableIf< float_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    checkTruncation<SRC,DST>(from);
    target = static_cast<DST>( from );
}


template<typename SRC,typename DST> inline
EnableIf< unsigned_to_smaller_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    checkUpperLimit<SRC,DST>(from);
    target = static_cast<DST>( from);
}

template<typename SRC,typename DST> inline
EnableIf< signed_to_smaller_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    checkLowerLimit<SRC,DST>(from);
    checkUpperLimit<SRC,DST>(from);
    target = static_cast<DST>( from);
}


template<typename SRC,typename DST> inline
EnableIf< signed_to_smaller_unsigned_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    if (from < 0 )
        throw std::runtime_error("Value is negative and can't be converted to signed");

    checkUpperLimit<SRC,DST>(from);
    target = static_cast<DST>( from );
}


template<typename SRC,typename DST> inline
EnableIf< signed_to_larger_unsigned_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    //std::cout << "signed_to_larger_unsigned_conversion" << std::endl;

    if ( from < 0 )
        throw std::runtime_error("Value is negative and can't be converted to signed");

    target = static_cast<DST>( from);
}

template<typename SRC,typename DST> inline
EnableIf< unsigned_to_larger_signed_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    //std::cout << "unsigned_to_larger_signed_conversion" << std::endl;
    target = static_cast<DST>( from);
}

template<typename SRC,typename DST> inline
EnableIf< unsigned_to_smaller_signed_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    checkUpperLimit<SRC,DST>(from);
    target = static_cast<DST>( from);
}

template<typename SRC,typename DST> inline
EnableIf< floating_to_signed_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    checkLowerLimitFloat<SRC,DST>(from);
    checkLowerLimitFloat<SRC,DST>(from);

    if( from != static_cast<SRC>(static_cast<DST>( from)))
        throw std::runtime_error("Floating point truncated");

    target = static_cast<DST>( from);
}

template<typename SRC,typename DST> inline
EnableIf< floating_to_unsigned_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    if ( from < 0 )
        throw std::runtime_error("Value is negative and can't be converted to signed");

    checkLowerLimitFloat<SRC,DST>(from);

    if( from != static_cast<SRC>(static_cast<DST>( from)))
        throw std::runtime_error("Floating point truncated");

    target = static_cast<DST>( from);
}

template<typename SRC,typename DST> inline
EnableIf< integer_to_floating_conversion<SRC, DST>>
convert_impl( const SRC& from, DST& target )
{
    checkTruncation<SRC,DST>(from);
    target = static_cast<DST>( from);
}

} //end namespace details


template<typename DST> inline
DST Any::convert() const
{
    using details::convert_impl;
    DST out;

    const auto& type = _any.type();

    if( ! details::is_convertible_type<DST>::value )
    {
        return linb::any_cast<DST>(_any);
    }

    if( type == typeid(bool)) {
        return DST( extract<bool>() );
    }
    else if( type == typeid(char)) {
        convert_impl<int8_t,  DST>( int8_t(extract<char>()), out  );
    }
    else if( type == typeid(int8_t)) {
        convert_impl<int8_t,  DST>(extract<int8_t>(), out  );
    }
    else if( type == typeid(int16_t)) {
        convert_impl<int16_t,  DST>(extract<int16_t>(), out  );
    }
    else if( type == typeid(int32_t)) {
        convert_impl<int32_t,  DST>(extract<int32_t>(), out  );
    }
    else if( type == typeid(int64_t)) {
        convert_impl<int64_t,  DST>(extract<int64_t>(), out  );
    }
    else if( type == typeid(uint8_t)) {
        convert_impl<uint8_t,  DST>(extract<uint8_t>(), out  );
    }
    else if( type == typeid(uint16_t)) {
        convert_impl<uint16_t,  DST>(extract<uint16_t>(), out  );
    }
    else if( type == typeid(uint32_t)) {
        convert_impl<uint32_t,  DST>(extract<uint32_t>(), out  );
    }
    else if( type == typeid(uint64_t)) {
        convert_impl<uint64_t,  DST>(extract<uint64_t>(), out  );
    }
    else if( type == typeid(float)) {
        convert_impl<float,  DST>(extract<float>(), out  );
    }
    else if( type == typeid(double)) {
        convert_impl<double,  DST>(extract<double>(), out  );
    }
    else if( type == typeid(SimpleString) )
    {
        throw std::runtime_error("String can not be converted to another type implicitly");
    }
    else{
        return linb::any_cast<DST>(_any);
    }

    return out;
}

template<> inline std::string Any::convert() const
{
    const auto& type = _any.type();

    if( type == typeid(SimpleString) )
    {
        return extract<SimpleString>().toStdString();
    }
    else if( type == typeid(bool)) {
        return std::to_string( extract<bool>() );
    }
    else if( type == typeid(char)) {
        return std::to_string( extract<char>() );
    }
    else if( type == typeid(int8_t)) {
        return std::to_string( extract<int8_t>() );
    }
    else if( type == typeid(int16_t)) {
        return std::to_string( extract<int16_t>() );
    }
    else if( type == typeid(int32_t)) {
        return std::to_string( extract<int32_t>() );
    }
    else if( type == typeid(int64_t)) {
        return std::to_string( extract<int64_t>() );
    }
    else if( type == typeid(uint8_t)) {
        return std::to_string( extract<uint8_t>() );
    }
    else if( type == typeid(uint16_t)) {
        return std::to_string( extract<uint16_t>() );
    }
    else if( type == typeid(uint32_t)) {
        return std::to_string( extract<uint32_t>() );
    }
    else if( type == typeid(uint64_t)) {
        return std::to_string( extract<uint64_t>() );
    }
    else if( type == typeid(float)) {
        return std::to_string( extract<float>() );
    }
    else if( type == typeid(double)) {
        return std::to_string( extract<double>() );
    }

    throw std::runtime_error("Conversion to std::string failed");
}


} // end namespace VarNumber


#endif // VARNUMBER_H
