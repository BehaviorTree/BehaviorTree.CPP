// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_BIT_CAST_HPP_INCLUDED
#define LEXY_CALLBACK_BIT_CAST_HPP_INCLUDED

#include <lexy/callback/base.hpp>

#ifndef LEXY_HAS_BITCAST
#    if defined(__has_builtin)
#        if __has_builtin(__builtin_bit_cast)
#            define LEXY_HAS_BITCAST 2
#        endif
#    endif
#endif

#ifndef LEXY_HAS_BITCAST
#    if defined(__has_include)
#        if __has_include(<bit>) && __cplusplus >= 202002L
#            include <bit>
#            ifdef __cpp_lib_bit_cast
#                define LEXY_HAS_BITCAST 1
#            endif
#        endif
#    endif
#endif

#ifndef LEXY_HAS_BITCAST
#    define LEXY_HAS_BITCAST 0
#endif

#if LEXY_HAS_BITCAST == 2
#    define LEXY_BITCAST_CONSTEXPR constexpr
#elif LEXY_HAS_BITCAST == 1
#    define LEXY_BITCAST_CONSTEXPR constexpr
#else
#    include <cstring>
#    define LEXY_BITCAST_CONSTEXPR
#endif

namespace lexy
{
template <typename T>
struct _bit_cast
{
    static_assert(std::is_trivially_copyable_v<T>);

    using return_type = T;

    template <typename Arg, typename = std::enable_if_t<sizeof(T) == sizeof(Arg)
                                                        && std::is_trivially_copyable_v<Arg>>>
    LEXY_BITCAST_CONSTEXPR T operator()(const Arg& arg) const
    {
#if LEXY_HAS_BITCAST == 2

        return __builtin_bit_cast(T, arg);

#elif LEXY_HAS_BITCAST == 1

        return std::bit_cast<T>(arg);

#else

        static_assert(std::is_default_constructible_v<T>, "sorry, get a better standard library");

        T to;
        std::memcpy(&to, &arg, sizeof(T));
        return to;

#endif
    }
};

/// std::bit_cast as a callback.
template <typename T>
constexpr auto bit_cast = _bit_cast<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_BIT_CAST_HPP_INCLUDED

