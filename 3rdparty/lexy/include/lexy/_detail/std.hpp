// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_STD_HPP_INCLUDED
#define LEXY_DETAIL_STD_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

//=== iterator tags ===//
#if defined(__GLIBCXX__)

namespace std
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION
struct forward_iterator_tag;
struct bidirectional_iterator_tag;
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

#elif defined(_LIBCPP_VERSION)

_LIBCPP_BEGIN_NAMESPACE_STD
struct forward_iterator_tag;
struct bidirectional_iterator_tag;
_LIBCPP_END_NAMESPACE_STD

#else

// Forward declaring things in std is not allowed, but I'm willing to take the risk.

namespace std
{
struct forward_iterator_tag;
struct bidirectional_iterator_tag;
} // namespace std

#endif

//=== (constexpr) construct_at ===//
#if !LEXY_HAS_CONSTEXPR_DTOR

namespace lexy::_detail
{
// We don't have constexpr dtor's, so this is just a regular function.
template <typename T, typename... Args>
T* construct_at(T* ptr, Args&&... args)
{
    return ::new ((void*)ptr) T(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#elif defined(_MSC_VER)

namespace lexy::_detail
{
// MSVC can make it constexpr if marked with an attribute given by a macro.
template <typename T, typename... Args>
constexpr T* construct_at(T* ptr, Args&&... args)
{
#    if defined(_MSVC_CONSTEXPR)
    _MSVC_CONSTEXPR
#    endif
    return ::new ((void*)ptr) T(LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#else

namespace lexy::_detail
{
struct _construct_at_tag
{};
} // namespace lexy::_detail

namespace std
{
// GCC only allows constexpr placement new inside a function called `std::construct_at`.
// So we write our own.
template <typename T, typename... Args>
constexpr T* construct_at(lexy::_detail::_construct_at_tag, T* ptr, Args&&... args)
{
    return ::new ((void*)ptr) T(LEXY_FWD(args)...);
}
} // namespace std

namespace lexy::_detail
{
template <typename T, typename... Args>
constexpr T* construct_at(T* ptr, Args&&... args)
{
    return std::construct_at(lexy::_detail::_construct_at_tag{}, ptr, LEXY_FWD(args)...);
}
} // namespace lexy::_detail

#endif

#endif // LEXY_DETAIL_STD_HPP_INCLUDED

