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

#if __has_include(<charconv>)
#include <charconv>
#endif

#include <string>
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
  using EnableIntegral = typename std::enable_if<std::is_integral<T>::value ||
                                                 std::is_enum<T>::value>::type*;

  template <typename T>
  using EnableNonIntegral = typename std::enable_if<!std::is_integral<T>::value &&
                                                    !std::is_enum<T>::value>::type*;

  template <typename T>
  using EnableString =
      typename std::enable_if<std::is_same<T, std::string>::value>::type*;

  template <typename T>
  using EnableArithmetic = typename std::enable_if<std::is_arithmetic<T>::value>::type*;

  template <typename T>
  using EnableEnum = typename std::enable_if<std::is_enum<T>::value>::type*;

  template <typename T>
  using EnableUnknownType =
      typename std::enable_if<!std::is_arithmetic<T>::value && !std::is_enum<T>::value &&
                              !std::is_same<T, std::string>::value>::type*;

  template <typename T>
  nonstd::expected<T, std::string> stringToNumber() const;

public:
  Any() : _original_type(UndefinedAnyType)
  {}

  ~Any() = default;

  Any(const Any& other) : _any(other._any), _original_type(other._original_type)
  {}

  Any(Any&& other) : _any(std::move(other._any)), _original_type(other._original_type)
  {}

  explicit Any(const double& value) : _any(value), _original_type(typeid(double))
  {}

  explicit Any(const uint64_t& value) : _any(value), _original_type(typeid(uint64_t))
  {}

  explicit Any(const float& value) : _any(double(value)), _original_type(typeid(float))
  {}

  explicit Any(const std::string& str)
    : _any(SafeAny::SimpleString(str)), _original_type(typeid(std::string))
  {}

  explicit Any(const char* str)
    : _any(SafeAny::SimpleString(str)), _original_type(typeid(std::string))
  {}

  explicit Any(const SafeAny::SimpleString& str)
    : _any(str), _original_type(typeid(std::string))
  {}

  explicit Any(const std::string_view& str)
    : _any(SafeAny::SimpleString(str)), _original_type(typeid(std::string))
  {}

  // all the other integrals are casted to int64_t
  template <typename T>
  explicit Any(const T& value, EnableIntegral<T> = 0)
    : _any(int64_t(value)), _original_type(typeid(T))
  {}

  Any(const std::type_index& type) : _original_type(type)
  {}

  // default for other custom types
  template <typename T>
  explicit Any(const T& value, EnableNonIntegral<T> = 0)
    : _any(value), _original_type(typeid(T))
  {
    static_assert(!std::is_reference<T>::value, "Any can not contain references");
  }

  Any& operator=(const Any& other);

  [[nodiscard]] bool isNumber() const;

  [[nodiscard]] bool isIntegral() const;

  [[nodiscard]] bool isString() const
  {
    return _any.type() == typeid(SafeAny::SimpleString);
  }

  // check is the original type is equal to T
  template <typename T>
  [[nodiscard]] bool isType() const
  {
    return _original_type == typeid(T);
  }

  // copy the value (casting into dst). We preserve the destination type.
  void copyInto(Any& dst) const;

  // this is different from any_cast, because if allows safe
  // conversions between arithmetic values and from/to string.
  template <typename T>
  nonstd::expected<T, std::string> tryCast() const;

  // same as tryCast, but throws if fails
  template <typename T>
  [[nodiscard]] T cast() const
  {
    if(auto res = tryCast<T>())
    {
      return res.value();
    }
    else
    {
      throw std::runtime_error(res.error());
    }
  }

  // Method to access the value by pointer.
  // It will return nullptr, if the user try to cast it to a
  // wrong type or if Any was empty.
  template <typename T>
  [[nodiscard]] T* castPtr()
  {
    static_assert(!std::is_same_v<T, float>, "The value has been casted internally to "
                                             "[double]. Use that instead");

    return _any.empty() ? nullptr : linb::any_cast<T>(&_any);
  }

  // This is the original type
  [[nodiscard]] const std::type_index& type() const noexcept
  {
    return _original_type;
  }

  // This is the type we casted to, internally
  [[nodiscard]] const std::type_info& castedType() const noexcept
  {
    return _any.type();
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return _any.empty();
  }

private:
  linb::any _any;
  std::type_index _original_type;

  //----------------------------

  template <typename DST>
  nonstd::expected<DST, std::string> convert(EnableString<DST> = 0) const;

  template <typename DST>
  nonstd::expected<DST, std::string> convert(EnableArithmetic<DST> = nullptr) const;

  template <typename DST>
  nonstd::expected<DST, std::string> convert(EnableEnum<DST> = 0) const;

  template <typename DST>
  nonstd::expected<DST, std::string> convert(EnableUnknownType<DST> = 0) const
  {
    return nonstd::make_unexpected(errorMsg<DST>());
  }

  template <typename T>
  std::string errorMsg() const
  {
    return StrCat("[Any::convert]: no known safe conversion between [", demangle(type()),
                  "] and [", demangle(typeid(T)), "]");
  }
};

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------

template <typename SRC, typename TO>
inline bool ValidCast(const SRC& val)
{
  // First check numeric limits
  if constexpr(std::is_arithmetic_v<SRC> && std::is_arithmetic_v<TO>)
  {
    // Handle conversion to floating point
    if constexpr(std::is_floating_point_v<TO>)
    {
      if constexpr(std::is_integral_v<SRC>)
      {
        // For integral to float, check if we can represent the value exactly
        TO as_float = static_cast<TO>(val);
        SRC back_conv = static_cast<SRC>(as_float);
        return back_conv == val;
      }
    }
    // Handle conversion to integral
    else if constexpr(std::is_integral_v<TO>)
    {
      if(val > static_cast<SRC>(std::numeric_limits<TO>::max()) ||
         val < static_cast<SRC>(std::numeric_limits<TO>::lowest()))
      {
        return false;
      }
    }
  }

  TO as_target = static_cast<TO>(val);
  SRC back_to_source = static_cast<SRC>(as_target);
  return val == back_to_source;
}

template <typename T>
inline bool isCastingSafe(const std::type_index& type, const T& val)
{
  if(type == typeid(T))
  {
    return true;
  }

  if(std::type_index(typeid(uint8_t)) == type)
  {
    return ValidCast<T, uint8_t>(val);
  }
  if(std::type_index(typeid(uint16_t)) == type)
  {
    return ValidCast<T, uint16_t>(val);
  }
  if(std::type_index(typeid(uint32_t)) == type)
  {
    return ValidCast<T, uint32_t>(val);
  }
  if(std::type_index(typeid(uint64_t)) == type)
  {
    return ValidCast<T, uint64_t>(val);
  }
  //------------
  if(std::type_index(typeid(int8_t)) == type)
  {
    return ValidCast<T, int8_t>(val);
  }
  if(std::type_index(typeid(int16_t)) == type)
  {
    return ValidCast<T, int16_t>(val);
  }
  if(std::type_index(typeid(int32_t)) == type)
  {
    return ValidCast<T, int32_t>(val);
  }
  if(std::type_index(typeid(int64_t)) == type)
  {
    return ValidCast<T, int64_t>(val);
  }
  //------------
  if(std::type_index(typeid(float)) == type)
  {
    return ValidCast<T, float>(val);
  }
  if(std::type_index(typeid(double)) == type)
  {
    return ValidCast<T, double>(val);
  }
  return false;
}

inline Any& Any::operator=(const Any& other)
{
  this->_any = other._any;
  this->_original_type = other._original_type;
  return *this;
}

inline bool Any::isNumber() const
{
  return _any.type() == typeid(int64_t) || _any.type() == typeid(uint64_t) ||
         _any.type() == typeid(double);
}

inline bool Any::isIntegral() const
{
  return _any.type() == typeid(int64_t) || _any.type() == typeid(uint64_t);
}

inline void Any::copyInto(Any& dst) const
{
  if(dst.empty())
  {
    dst = *this;
    return;
  }

  const auto& dst_type = dst.castedType();

  if((castedType() == dst_type) || (isString() && dst.isString()))
  {
    dst._any = _any;
  }
  else if(isNumber() && dst.isNumber())
  {
    if(dst_type == typeid(int64_t))
    {
      dst._any = cast<int64_t>();
    }
    else if(dst_type == typeid(uint64_t))
    {
      dst._any = cast<uint64_t>();
    }
    else if(dst_type == typeid(double))
    {
      dst._any = cast<double>();
    }
    else
    {
      throw std::runtime_error("Any::copyInto fails");
    }
  }
  else
  {
    throw std::runtime_error("Any::copyInto fails");
  }
}

template <typename DST>
inline nonstd::expected<DST, std::string> Any::convert(EnableString<DST>) const
{
  const auto& type = _any.type();

  if(type == typeid(SafeAny::SimpleString))
  {
    return linb::any_cast<SafeAny::SimpleString>(_any).toStdString();
  }
  else if(type == typeid(int64_t))
  {
    return std::to_string(linb::any_cast<int64_t>(_any));
  }
  else if(type == typeid(uint64_t))
  {
    return std::to_string(linb::any_cast<uint64_t>(_any));
  }
  else if(type == typeid(double))
  {
    return std::to_string(linb::any_cast<double>(_any));
  }

  return nonstd::make_unexpected(errorMsg<DST>());
}

template <typename T>
inline nonstd::expected<T, std::string> Any::stringToNumber() const
{
  static_assert(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, "Expecting a "
                                                                     "numeric type");

  const auto str = linb::any_cast<SafeAny::SimpleString>(_any);
#if __cpp_lib_to_chars >= 201611L
  T out;
  auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), out);
  if(err == std::errc())
  {
    return out;
  }
  else
  {
    return nonstd::make_unexpected("Any failed string to number conversion");
  }
#else
  try
  {
    if constexpr(std::is_same_v<T, uint16_t>)
    {
      return std::stoul(str.toStdString());
    }
    if constexpr(std::is_integral_v<T>)
    {
      const int64_t val = std::stol(str.toStdString());
      Any temp_any(val);
      return temp_any.convert<T>();
    }
    if constexpr(std::is_floating_point_v<T>)
    {
      return std::stod(str.toStdString());
    }
  }
  catch(...)
  {
    return nonstd::make_unexpected("Any failed string to number conversion");
  }
#endif
  return nonstd::make_unexpected("Any conversion from string failed");
}

template <typename DST>
inline nonstd::expected<DST, std::string> Any::convert(EnableEnum<DST>) const
{
  using SafeAny::details::convertNumber;

  const auto& type = _any.type();

  if(type == typeid(int64_t))
  {
    auto out = linb::any_cast<int64_t>(_any);
    return static_cast<DST>(out);
  }
  else if(type == typeid(uint64_t))
  {
    auto out = linb::any_cast<uint64_t>(_any);
    return static_cast<DST>(out);
  }

  return nonstd::make_unexpected(errorMsg<DST>());
}

template <typename DST>
inline nonstd::expected<DST, std::string> Any::convert(EnableArithmetic<DST>) const
{
  using SafeAny::details::convertNumber;
  DST out;

  const auto& type = _any.type();

  if(type == typeid(int64_t))
  {
    convertNumber<int64_t, DST>(linb::any_cast<int64_t>(_any), out);
  }
  else if(type == typeid(uint64_t))
  {
    convertNumber<uint64_t, DST>(linb::any_cast<uint64_t>(_any), out);
  }
  else if(type == typeid(double))
  {
    convertNumber<double, DST>(linb::any_cast<double>(_any), out);
  }
  else
  {
    return nonstd::make_unexpected(errorMsg<DST>());
  }
  return out;
}

template <typename T>
inline nonstd::expected<T, std::string> Any::tryCast() const
{
  static_assert(!std::is_reference<T>::value, "Any::cast uses value semantic, "
                                              "can not cast to reference");

  if(_any.empty())
  {
    throw std::runtime_error("Any::cast failed because it is empty");
  }

  if(castedType() == typeid(T))
  {
    return linb::any_cast<T>(_any);
  }

  // special case when the output is an enum.
  // We will try first a int conversion
  if constexpr(std::is_enum_v<T>)
  {
    if(isNumber())
    {
      return static_cast<T>(convert<int>().value());
    }
    if(isString())
    {
      if(auto out = stringToNumber<int64_t>())
      {
        return static_cast<T>(out.value());
      }
    }
    return nonstd::make_unexpected("Any::cast failed to cast to enum type");
  }

  if(isString())
  {
    if constexpr(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
    {
      if(auto out = stringToNumber<T>())
      {
        return out.value();
      }
      else
      {
        return out;
      }
    }
  }

  if(auto res = convert<T>())
  {
    return res.value();
  }
  else
  {
    return res;
  }
}

}  // end namespace BT
