#ifndef SAFE_ANY_VARNUMBER_H
#define SAFE_ANY_VARNUMBER_H

#include <exception>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <string>
#include <cstring>
#include <type_traits>
#include "any.hpp"
#include "demangle_util.h"
#include "convert_impl.hpp"
#include "expected.hpp"
#include "strcat.hpp"
#include "strcat.hpp"

namespace BT
{
// Rational: since type erased numbers will always use at least 8 bytes
// it is faster to cast everything to either double, uint64_t or int64_t.
class Any
{
    template <typename T>
    using EnableIntegral =
        typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type*;

    template <typename T>
    using EnableNonIntegral =
        typename std::enable_if<!std::is_integral<T>::value && !std::is_enum<T>::value>::type*;

    template <typename T>
    using EnableString = typename std::enable_if<std::is_same<T, std::string>::value>::type*;

    template <typename T>
    using EnableArithmetic = typename std::enable_if<std::is_arithmetic<T>::value>::type*;

    template <typename T>
    using EnableEnum = typename std::enable_if<std::is_enum<T>::value>::type*;

    template <typename T>
    using EnableUnknownType =
        typename std::enable_if<!std::is_arithmetic<T>::value && !std::is_enum<T>::value &&
                                !std::is_same<T, std::string>::value>::type*;

  public:
    Any(): _original_type(nullptr)
    {
    }

    ~Any() = default;

    Any(const Any& other) : _any(other._any), _original_type( other._original_type )
    {
    }

    Any(Any&& other) : _any( std::move(other._any) ), _original_type( other._original_type )
    {
    }

    explicit Any(const double& value) : _any(value), _original_type( &typeid(double) )
    {
    }

    explicit Any(const uint64_t& value) : _any(value), _original_type( &typeid(uint64_t) )
    {
    }

    explicit Any(const float& value) : _any(double(value)), _original_type( &typeid(float) )
    {
    }

    explicit Any(const std::string& str) : _any(SafeAny::SimpleString(str)), _original_type( &typeid(std::string) )
    {
    }

    explicit Any(const char* str) : _any(SafeAny::SimpleString(str)), _original_type( &typeid(std::string) )
    {
    }

    explicit Any(const SafeAny::SimpleString& str) : _any(str), _original_type( &typeid(std::string) )
    {
    }

    // all the other integrals are casted to int64_t
    template <typename T>
    explicit Any(const T& value, EnableIntegral<T> = 0) : _any(int64_t(value)), _original_type( &typeid(T) )
    {
    }

    // default for other custom types
    template <typename T>
    explicit Any(const T& value, EnableNonIntegral<T> = 0) : _any(value), _original_type( &typeid(T) )
    {
    }

    Any& operator = (const Any& other)
    {
        this->_any = other._any;
        this->_original_type = other._original_type;
        return *this;
    }

    bool isNumber() const
    {
        return _any.type() == typeid(int64_t) ||
               _any.type() == typeid(uint64_t) ||
               _any.type() == typeid(double);
    }

    bool isString() const
    {
        return _any.type() == typeid(SafeAny::SimpleString);
    }

    // this is different from any_cast, because if allows safe
    // conversions between arithmetic values.
    template <typename T>
    T cast() const
    {
        if( _any.empty() )
        {
            throw std::runtime_error("Any::cast failed because it is empty");
        }
        if (_any.type() == typeid(T))
        {
            return linb::any_cast<T>(_any);
        }
        else
        {
            auto res = convert<T>();
            if( !res )
            {
                throw std::runtime_error( res.error() );
            }
            return res.value();
        }
    }

    const std::type_info& type() const noexcept
    {
        return *_original_type;
    }

    const std::type_info& castedType() const noexcept
    {
        return _any.type();
    }

    bool empty() const noexcept
    {
        return _any.empty();
    }

  private:
    linb::any _any;
    const std::type_info* _original_type;

    //----------------------------

    template <typename DST>
    nonstd::expected<DST,std::string> convert(EnableString<DST> = 0) const
    {
        const auto& type = _any.type();

        if (type == typeid(SafeAny::SimpleString))
        {
            return linb::any_cast<SafeAny::SimpleString>(_any).toStdString();
        }
        else if (type == typeid(int64_t))
        {
            return std::to_string(linb::any_cast<int64_t>(_any));
        }
        else if (type == typeid(uint64_t))
        {
            return std::to_string(linb::any_cast<uint64_t>(_any));
        }
        else if (type == typeid(double))
        {
            return std::to_string(linb::any_cast<double>(_any));
        }

        return nonstd::make_unexpected( errorMsg<DST>() );
    }

    template <typename DST>
    nonstd::expected<DST,std::string> convert(EnableArithmetic<DST> = 0) const
    {
        using SafeAny::details::convertNumber;
        DST out;

        const auto& type = _any.type();

        if (type == typeid(int64_t))
        {
            convertNumber<int64_t, DST>(linb::any_cast<int64_t>(_any), out);
        }
        else if (type == typeid(uint64_t))
        {
            convertNumber<uint64_t, DST>(linb::any_cast<uint64_t>(_any), out);
        }
        else if (type == typeid(double))
        {
            convertNumber<double, DST>(linb::any_cast<double>(_any), out);
        }
        else{
            return nonstd::make_unexpected( errorMsg<DST>() );
        }
        return out;
    }

    template <typename DST>
    nonstd::expected<DST,std::string> convert(EnableEnum<DST> = 0) const
    {
        using SafeAny::details::convertNumber;

        const auto& type = _any.type();

        if (type == typeid(int64_t))
        {
            uint64_t out = linb::any_cast<int64_t>(_any);
            return static_cast<DST>(out);
        }
        else if (type == typeid(uint64_t))
        {
            uint64_t out = linb::any_cast<uint64_t>(_any);
            return static_cast<DST>(out);
        }

        return nonstd::make_unexpected( errorMsg<DST>() );
    }

    template <typename DST>
    nonstd::expected<DST,std::string> convert(EnableUnknownType<DST> = 0) const
    {
        return nonstd::make_unexpected( errorMsg<DST>() );
    }

    template <typename T>
    std::string errorMsg() const
    {
        return StrCat("[Any::convert]: no known safe conversion between [",
                      demangle( _any.type() ), "] and [", demangle( typeid(T) ),"]");
    }
};

}   // end namespace BT

#endif   // VARNUMBER_H
