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

template <typename T>
struct any_cast_base
{
  using type = void;  // Default: no base known, fallback to default any storage
};

// C++17 backport of std::type_identity
template <typename T>
struct type_identity
{
  using type = T;
};

// Trait to check if a type has a valid cast base
template <typename T, typename = void>
struct has_valid_cast_base : std::false_type
{
};

template <typename T>
struct has_valid_cast_base<T, std::void_t<typename any_cast_base<T>::type>>
{
  static constexpr bool value =
      !std::is_same<typename any_cast_base<T>::type, void>::value;
};

// Recursive helper (non-self-recursive, SFINAE-safe)
template <typename T>
struct resolve_root_base_helper
{
  using Base = typename any_cast_base<T>::type;

  using type = typename std::conditional<std::is_same<T, Base>::value, type_identity<T>,
                                         resolve_root_base_helper<Base>>::type::type;
};

// Public interface with guard
template <typename T>
struct root_base_resolver
{
  using type = typename std::conditional<has_valid_cast_base<T>::value,
                                         resolve_root_base_helper<T>,
                                         type_identity<T>>::type::type;
};

template <typename T>
using root_base_t = typename root_base_resolver<T>::type;

// Trait to detect std::shared_ptr types.
template <typename T>
struct is_shared_ptr : std::false_type
{
};

template <typename U>
struct is_shared_ptr<std::shared_ptr<U>> : std::true_type
{
};

// Trait to detect if a type is complete
template <typename T, typename = void>
struct is_complete : std::false_type
{
};

template <typename T>
struct is_complete<T, decltype(void(sizeof(T)))> : std::true_type
{
};

// Trait to detect if a trait is complete and polymorphic
template <typename T, typename = void>
struct is_polymorphic_safe : std::false_type
{
};

// Specialization only enabled if T is complete
template <typename T>
struct is_polymorphic_safe<T, std::enable_if_t<is_complete<T>::value>>
  : std::integral_constant<bool, std::is_polymorphic<T>::value>
{
};

template <typename T>
inline constexpr bool is_polymorphic_safe_v = is_polymorphic_safe<T>::value;

// Compute and store the base class inheritance chain of a type T
// from derived to root base order, e.g. SphynxCat -> Cat -> Animal
template <typename... Ts>
struct type_list
{
  template <typename U>
  using prepend = type_list<U, Ts...>;
};

template <typename... Ts>
std::vector<std::type_index> to_type_index_vector(type_list<Ts...>)
{
  return { std::type_index(typeid(Ts))... };
}

template <typename T, typename = void>
struct compute_base_chain_impl
{
  using type = type_list<T>;
};

// Base case: recursion ends when base is void or equal to self
template <typename T>
struct compute_base_chain_impl<
    T, std::enable_if_t<std::is_same_v<typename any_cast_base<T>::type, void> ||
                        std::is_same_v<typename any_cast_base<T>::type, T>>>
{
  using type = type_list<T>;
};

// Recursive case
template <typename T>
struct compute_base_chain_impl<
    T, std::enable_if_t<has_valid_cast_base<T>::value &&
                        !std::is_same_v<typename any_cast_base<T>::type, void> &&
                        !std::is_same_v<typename any_cast_base<T>::type, T> &&
                        is_polymorphic_safe_v<T>>>
{
private:
  using Base = typename any_cast_base<T>::type;
  using BaseChain = typename compute_base_chain_impl<Base>::type;

public:
  using type = typename BaseChain::template prepend<T>;
};

template <typename T>
using compute_base_chain = typename compute_base_chain_impl<T>::type;

template <typename T>
std::vector<std::type_index> get_base_chain_type_index()
{
  return to_type_index_vector(compute_base_chain<T>{});
}

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

  // default for shared pointers
  template <typename T>
  explicit Any(const std::shared_ptr<T>& value)
    : _original_type(typeid(std::shared_ptr<T>))
  {
    using Base = typename any_cast_base<T>::type;

    // store as base class if specialized
    if constexpr(!std::is_same_v<Base, void>)
    {
      using RootBase = root_base_t<T>;

      static_assert(is_polymorphic_safe_v<RootBase>, "Any Base trait specialization must "
                                                     "be "
                                                     "polymorphic");
      _any = std::static_pointer_cast<RootBase>(value);
    }
    else
    {
      _any = value;
    }
  }

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
  void copyInto(Any& dst);

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
  //
  // WARNING: The returned pointer may alias internal cache and be invalidated by subsequent castPtr() calls.
  // Do not store it long-term. Applies only to shared_ptr<Derived> where Derived is polymorphic and base-registered.
  template <typename T>
  [[nodiscard]] T* castPtr()
  {
    static_assert(!std::is_same_v<T, float>, "The value has been casted internally to "
                                             "[double]. "
                                             "Use that instead");
    static_assert(!SafeAny::details::is_integer<T>() || std::is_same_v<T, uint64_t>, "The"
                                                                                     " va"
                                                                                     "lue"
                                                                                     " ha"
                                                                                     "s "
                                                                                     "bee"
                                                                                     "n "
                                                                                     "cas"
                                                                                     "ted"
                                                                                     " in"
                                                                                     "ter"
                                                                                     "nal"
                                                                                     "ly "
                                                                                     "to "
                                                                                     "[in"
                                                                                     "t64"
                                                                                     "_t]"
                                                                                     ". "
                                                                                     "Use"
                                                                                     " th"
                                                                                     "at "
                                                                                     "ins"
                                                                                     "tea"
                                                                                     "d");

    // Special case: applies only when requesting shared_ptr<Derived> and Derived is polymorphic
    // with a registered base via any_cast_base.
    if constexpr(is_shared_ptr<T>::value)
    {
      using Derived = typename T::element_type;
      using Base = typename any_cast_base<Derived>::type;

      if constexpr(is_polymorphic_safe_v<Derived> && !std::is_same_v<Base, void>)
      {
        using RootBase = root_base_t<Derived>;

        try
        {
          // Attempt to retrieve the stored shared_ptr<Base> from the Any container
          auto base_ptr = linb::any_cast<std::shared_ptr<RootBase>>(&_any);
          if(!base_ptr)
            return nullptr;

          // Case 1: If Base and Derived are the same, no casting is needed
          if constexpr(std::is_same_v<RootBase, Derived>)
          {
            return reinterpret_cast<T*>(base_ptr);
          }

          // Case 2: Originally stored as shared_ptr<Derived>
          if(_original_type == typeid(std::shared_ptr<Derived>))
          {
            _cached_derived_ptr = std::static_pointer_cast<Derived>(*base_ptr);
            return reinterpret_cast<T*>(&_cached_derived_ptr);
          }

          // Case 3: Fallback to dynamic cast
          auto derived_ptr = std::dynamic_pointer_cast<Derived>(*base_ptr);
          if(derived_ptr)
          {
            _cached_derived_ptr = derived_ptr;
            return reinterpret_cast<T*>(&_cached_derived_ptr);
          }
        }
        catch(...)
        {
          return nullptr;
        }

        return nullptr;
      }
    }

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
  mutable std::shared_ptr<void> _cached_derived_ptr = nullptr;

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

inline void Any::copyInto(Any& dst)
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

  // special case: T is a shared_ptr to a registered polymorphic type.
  // The stored value is a shared_ptr<Base>, but the user is requesting shared_ptr<Derived>.
  // Perform safe downcasting (static or dynamic) from Base to Derived if applicable.
  if constexpr(is_shared_ptr<T>::value)
  {
    using Derived = typename T::element_type;
    using Base = typename any_cast_base<Derived>::type;

    if constexpr(is_polymorphic_safe_v<Derived> && !std::is_same_v<Base, void>)
    {
      using RootBase = root_base_t<Derived>;

      // Attempt to retrieve the stored shared_ptr<Base> from the Any container
      auto base_ptr = linb::any_cast<std::shared_ptr<RootBase>>(_any);
      if(!base_ptr)
      {
        throw std::runtime_error("Any::cast cannot cast to shared_ptr<Base> class");
      }

      // Case 1: If Base and Derived are the same, no casting is needed
      if constexpr(std::is_same_v<T, std::shared_ptr<RootBase>>)
      {
        return base_ptr;
      }

      // Case 2: If the original stored type was shared_ptr<Derived>, we can safely static_cast
      if(_original_type == typeid(std::shared_ptr<Derived>))
      {
        return std::static_pointer_cast<Derived>(base_ptr);
      }

      // Case 3: Otherwise, attempt a dynamic cast from Base to Derived
      auto derived_ptr = std::dynamic_pointer_cast<Derived>(base_ptr);
      if(!derived_ptr)
        throw std::runtime_error("Any::cast Dynamic cast failed, types are not related");

      return derived_ptr;
    }
  }

  if(castedType() == typeid(T))
  {
    return linb::any_cast<T>(_any);
  }

  // special case when the output is an enum.
  // We will try first a int convertion
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
