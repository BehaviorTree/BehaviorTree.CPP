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

namespace SafeAny
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
    explicit Any(): _original_type(nullptr)
    {
    }

    ~Any() = default;

    explicit Any(const Any& other) : _any(other._any), _original_type( &other.type() )
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

    explicit Any(const std::string& str) : _any(SimpleString(str)), _original_type( &typeid(std::string) )
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
               _any.type() ==typeid(uint64_t) ||
               _any.type() == typeid(double);
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
            return convert<T>();
        }
    }

    const std::type_info& type() const noexcept
    {
        return *_original_type;
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
    DST convert(EnableString<DST> = 0) const
    {
        const auto& type = _any.type();

        if (type == typeid(SimpleString))
        {
            return linb::any_cast<SimpleString>(_any).toStdString();
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

        throw errorMsg<DST>();
    }

    template <typename DST>
    DST convert(EnableArithmetic<DST> = 0) const
    {
        using details::convertNumber;
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
        else
        {
            throw errorMsg<DST>();
        }
        return out;
    }

    template <typename DST>
    DST convert(EnableEnum<DST> = 0) const
    {
        using details::convertNumber;

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

        throw errorMsg<DST>();
    }

    template <typename DST>
    DST convert(EnableUnknownType<DST> = 0) const
    {
        throw errorMsg<DST>();
    }

    template <typename T>
    std::runtime_error errorMsg() const
    {
        char buffer[1024];
        sprintf(buffer, "[Any::convert]: no known safe conversion between %s and %s",
                BT::demangle( _any.type() ).c_str(),
                BT::demangle( typeid(T) ).c_str() );
        return std::runtime_error(buffer);
    }
};

}   // end namespace VarNumber

#endif   // VARNUMBER_H
