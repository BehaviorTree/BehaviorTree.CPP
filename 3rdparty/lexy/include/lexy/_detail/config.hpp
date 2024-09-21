// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_CONFIG_HPP_INCLUDED
#define LEXY_DETAIL_CONFIG_HPP_INCLUDED

#include <cstddef>
#include <type_traits>

#if defined(LEXY_USER_CONFIG_HEADER)
#    include LEXY_USER_CONFIG_HEADER
#elif defined(__has_include)
#    if __has_include(<lexy_user_config.hpp>)
#        include <lexy_user_config.hpp>
#    elif __has_include("lexy_user_config.hpp")
#        include "lexy_user_config.hpp"
#    endif
#endif

#ifndef LEXY_HAS_UNICODE_DATABASE
#    define LEXY_HAS_UNICODE_DATABASE 0
#endif

#ifndef LEXY_EXPERIMENTAL
#    define LEXY_EXPERIMENTAL 0
#endif

//=== utility traits===//
#define LEXY_MOV(...) static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define LEXY_FWD(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define LEXY_DECLVAL(...) lexy::_detail::declval<__VA_ARGS__>()

#define LEXY_DECAY_DECLTYPE(...) std::decay_t<decltype(__VA_ARGS__)>

/// Creates a new type from the instantiation of a template.
/// This is used to shorten type names.
#define LEXY_INSTANTIATION_NEWTYPE(Name, Templ, ...)                                               \
    struct Name : Templ<__VA_ARGS__>                                                               \
    {                                                                                              \
        using Templ<__VA_ARGS__>::Templ;                                                           \
    }

namespace lexy::_detail
{
template <typename... T>
constexpr bool error = false;

template <typename T>
std::add_rvalue_reference_t<T> declval();

template <typename T>
constexpr void swap(T& lhs, T& rhs)
{
    T tmp = LEXY_MOV(lhs);
    lhs   = LEXY_MOV(rhs);
    rhs   = LEXY_MOV(tmp);
}

template <typename T, typename U>
constexpr bool is_decayed_same = std::is_same_v<std::decay_t<T>, std::decay_t<U>>;

template <typename T, typename Fallback>
using type_or = std::conditional_t<std::is_void_v<T>, Fallback, T>;
} // namespace lexy::_detail

//=== NTTP ===//
#ifndef LEXY_HAS_NTTP
//   See https://github.com/foonathan/lexy/issues/15.
#    if __cpp_nontype_template_parameter_class >= 201806 || __cpp_nontype_template_args >= 201911
#        define LEXY_HAS_NTTP 1
#    else
#        define LEXY_HAS_NTTP 0
#    endif
#endif

#if LEXY_HAS_NTTP
#    define LEXY_NTTP_PARAM auto
#else
#    define LEXY_NTTP_PARAM const auto&
#endif

//=== consteval ===//
#ifndef LEXY_HAS_CONSTEVAL
#    if defined(_MSC_VER) && !defined(__clang__)
//       Currently can't handle returning strings from consteval, check back later.
#        define LEXY_HAS_CONSTEVAL 0
#    elif __cpp_consteval
#        define LEXY_HAS_CONSTEVAL 1
#    else
#        define LEXY_HAS_CONSTEVAL 0
#    endif
#endif

#if LEXY_HAS_CONSTEVAL
#    define LEXY_CONSTEVAL consteval
#else
#    define LEXY_CONSTEVAL constexpr
#endif

//=== constexpr ===//
#ifndef LEXY_HAS_CONSTEXPR_DTOR
#    if __cpp_constexpr_dynamic_alloc
#        define LEXY_HAS_CONSTEXPR_DTOR 1
#    else
#        define LEXY_HAS_CONSTEXPR_DTOR 0
#    endif
#endif

#if LEXY_HAS_CONSTEXPR_DTOR
#    define LEXY_CONSTEXPR_DTOR constexpr
#else
#    define LEXY_CONSTEXPR_DTOR
#endif

//=== char8_t ===//
#ifndef LEXY_HAS_CHAR8_T
#    if __cpp_char8_t
#        define LEXY_HAS_CHAR8_T 1
#    else
#        define LEXY_HAS_CHAR8_T 0
#    endif
#endif

#if LEXY_HAS_CHAR8_T

#    define LEXY_CHAR_OF_u8 char8_t
#    define LEXY_CHAR8_T char8_t
#    define LEXY_CHAR8_STR(Str) u8##Str

#else

namespace lexy
{
using _char8_t = unsigned char;
} // namespace lexy

#    define LEXY_CHAR_OF_u8 char
#    define LEXY_CHAR8_T ::lexy::_char8_t
#    define LEXY_CHAR8_STR(Str)                                                                    \
        LEXY_NTTP_STRING(::lexy::_detail::type_string, u8##Str)::c_str<LEXY_CHAR8_T>

#endif

//=== endianness ===//
#ifndef LEXY_IS_LITTLE_ENDIAN
#    if defined(__BYTE_ORDER__)
#        if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#            define LEXY_IS_LITTLE_ENDIAN 1
#        elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#            define LEXY_IS_LITTLE_ENDIAN 0
#        else
#            error "unsupported byte order"
#        endif
#    elif defined(_MSC_VER)
#        define LEXY_IS_LITTLE_ENDIAN 1
#    else
#        error "unknown endianness"
#    endif
#endif

//=== force inline ===//
#ifndef LEXY_FORCE_INLINE
#    if defined(__has_cpp_attribute)
#        if __has_cpp_attribute(gnu::always_inline)
#            define LEXY_FORCE_INLINE [[gnu::always_inline]]
#        endif
#    endif
#
#    ifndef LEXY_FORCE_INLINE
#        define LEXY_FORCE_INLINE inline
#    endif
#endif

//=== empty_member ===//
#ifndef LEXY_EMPTY_MEMBER

#    if defined(__has_cpp_attribute)
#        if defined(__GNUC__) && !defined(__clang__) && __GNUC__ <= 11
//           GCC <= 11 has buggy support, see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=101040
#            define LEXY_HAS_EMPTY_MEMBER 0
#        elif __has_cpp_attribute(no_unique_address)
#            define LEXY_HAS_EMPTY_MEMBER 1
#        endif
#    endif
#    ifndef LEXY_HAS_EMPTY_MEMBER
#        define LEXY_HAS_EMPTY_MEMBER 0
#    endif

#    if LEXY_HAS_EMPTY_MEMBER
#        define LEXY_EMPTY_MEMBER [[no_unique_address]]
#    else
#        define LEXY_EMPTY_MEMBER
#    endif

#endif

#endif // LEXY_DETAIL_CONFIG_HPP_INCLUDED

