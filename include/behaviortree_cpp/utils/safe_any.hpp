/* Copyright (C) 2022 Davide Faconti -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <exception>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <string>
#include <cstring>
#include <type_traits>
#include <typeindex>

#include "behaviortree_cpp/contrib/any.hpp"
#include "behaviortree_cpp/contrib/expected.hpp"
#include "behaviortree_cpp/utils/demangle_util.h"
#include "behaviortree_cpp/utils/convert_impl.hpp"
#include "behaviortree_cpp/utils/strcat.hpp"

namespace BT
{

static std::type_index UndefinedAnyType = typeid(nullptr);

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

    Any(): _original_type(UndefinedAnyType)
    {
    }

    ~Any() = default;

    Any(const Any& other) : _any(other._any), _original_type( other._original_type )
    {
    }

    Any(Any&& other) : _any( std::move(other._any) ), _original_type( other._original_type )
    {
    }

    explicit Any(const double& value) : _any(value), _original_type( typeid(double) )
    {
    }

    explicit Any(const uint64_t& value) : _any(value), _original_type( typeid(uint64_t) )
    {
    }

    explicit Any(const float& value) : _any(double(value)), _original_type( typeid(float) )
    {
    }

    explicit Any(const std::string& str) : _any(SafeAny::SimpleString(str)), _original_type( typeid(std::string) )
    {
    }

    explicit Any(const char* str) : _any(SafeAny::SimpleString(str)), _original_type( typeid(std::string) )
    {
    }

    explicit Any(const SafeAny::SimpleString& str) : _any(str), _original_type( typeid(std::string) )
    {
    }

    explicit Any(const std::string_view& str) : _any(SafeAny::SimpleString(str)), _original_type( typeid(std::string) )
    {
    }

    // all the other integrals are casted to int64_t
    template <typename T>
    explicit Any(const T& value, EnableIntegral<T> = 0) : _any(int64_t(value)), _original_type( typeid(T) )
    {
    }

    Any(const std::type_index& type): _original_type(type)
    {
    }

    // default for other custom types
    template <typename T>
    explicit Any(const T& value, EnableNonIntegral<T> = 0) : _any(value), _original_type( typeid(T) )
    {
        static_assert(!std::is_reference<T>::value, "Any can not contain references");
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

    bool isIntegral() const
    {
        return _any.type() == typeid(int64_t) ||
               _any.type() == typeid(uint64_t);
    }

    bool isString() const
    {
        return _any.type() == typeid(SafeAny::SimpleString);
    }

    template <typename T>
    bool isType() const
    {
      return _any.type() == typeid(T);
    }

    // copy the value (casting into dst). We preserve the destination type.
    void copyInto(Any& dst)
    {
        if(dst.empty())
        {
            dst = *this;
            return;
        }

        const auto& dst_type = dst.castedType();

        if ((type() == dst_type) || (isString() && dst.isString()) )
        {
            dst._any = _any;
        }
        else if(isNumber() && dst.isNumber())
        {
            if (dst_type == typeid(int64_t))
            {
                dst._any = cast<int64_t>();
            }
            else if (dst_type == typeid(uint64_t))
            {
                dst._any = cast<uint64_t>();
            }
            else if (dst_type == typeid(double))
            {
                dst._any = cast<double>();
            }
            else{
                throw std::runtime_error("Any::copyInto fails");
            }
        }
        else{
            throw std::runtime_error("Any::copyInto fails");
        }
    }

    // this is different from any_cast, because if allows safe
    // conversions between arithmetic values.
    template <typename T>
    T cast() const
    {
        static_assert(!std::is_reference<T>::value, "Any::cast uses value semantic, "
                                                    "can not cast to reference");
        if constexpr(std::is_enum_v<T>)
        {
            if(!isNumber())
            {
                std::cout  <<  demangle( _any.type() ) << std::endl;
                throw std::runtime_error("Any::cast failed to cast to enum type");
            }
            return static_cast<T>( convert<int>().value() );
        }
        else
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
              if(auto res = convert<T>())
              {
                return res.value();
              }
              else {
                throw std::runtime_error( res.error() );
              }
          }
        }
    }

    const std::type_index& type() const noexcept
    {
        return _original_type;
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
    std::type_index _original_type;

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
    nonstd::expected<DST,std::string> convert(EnableArithmetic<DST> = nullptr) const
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
            auto out = linb::any_cast<int64_t>(_any);
            return static_cast<DST>(out);
        }
        else if (type == typeid(uint64_t))
        {
            auto out = linb::any_cast<uint64_t>(_any);
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

template <typename SRC, typename TO> inline
bool ValidCast(const SRC& val)
{
    return( val == static_cast<SRC>(static_cast<TO>(val)) );
}

template <typename T> inline
bool isCastingSafe(const std::type_index& type, const T& val)
{
    if(type == typeid(T)) {
        return true;
    }

    if(std::type_index(typeid(uint8_t)) == type) {
        return ValidCast<T, uint8_t>(val);
    }
    if(std::type_index(typeid(uint16_t)) == type) {
        return ValidCast<T, uint16_t>(val);
    }
    if(std::type_index(typeid(uint32_t)) == type) {
        return ValidCast<T, uint32_t>(val);
    }
    if(std::type_index(typeid(uint64_t)) == type) {
        return ValidCast<T, uint64_t>(val);
    }
    //------------
    if(std::type_index(typeid(int8_t)) == type) {
        return ValidCast<T, int8_t>(val);
    }
    if(std::type_index(typeid(int16_t)) == type) {
        return ValidCast<T, int16_t>(val);
    }
    if(std::type_index(typeid(int32_t)) == type) {
        return ValidCast<T, int32_t>(val);
    }
    if(std::type_index(typeid(int64_t)) == type) {
        return ValidCast<T, int64_t>(val);
    }
    //------------
    if(std::type_index(typeid(float)) == type) {
        return ValidCast<T, float>(val);
    }
    if(std::type_index(typeid(double)) == type) {
        return ValidCast<T, double>(val);
    }
    return false;
}

}   // end namespace BT

