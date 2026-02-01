// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_INTEGER_HPP_INCLUDED
#define LEXY_CALLBACK_INTEGER_HPP_INCLUDED

#include <lexy/callback/base.hpp>
#include <lexy/dsl/sign.hpp>

namespace lexy
{
template <typename T>
struct _int
{
    using return_type = T;

    // You don't actually produce an integer value.
    constexpr T operator()(lexy::plus_sign) const  = delete;
    constexpr T operator()(lexy::minus_sign) const = delete;

    template <typename Integer>
    constexpr T operator()(const Integer& value) const
    {
        return T(value);
    }
    template <typename Integer>
    constexpr T operator()(lexy::plus_sign, const Integer& value) const
    {
        return T(value);
    }
    template <typename Integer>
    constexpr T operator()(lexy::minus_sign, const Integer& value) const
    {
        return T(-value);
    }
};

// A callback that takes an optional sign and an integer and produces the signed integer.
template <typename T>
constexpr auto as_integer = _int<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_INTEGER_HPP_INCLUDED

