// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_FORWARD_HPP_INCLUDED
#define LEXY_CALLBACK_FORWARD_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
struct nullopt;

template <typename T>
struct _fwd
{
    using return_type = T;

    constexpr T operator()(T&& t) const
    {
        return LEXY_MOV(t);
    }
    constexpr T operator()(const T& t) const
    {
        return t;
    }
};
template <>
struct _fwd<void>
{
    using return_type = void;

    template <typename... Args>
    constexpr auto sink(const Args&...) const
    {
        // We don't need a separate type, forward itself can have the required functions.
        return *this;
    }

    constexpr void operator()() const {}
    constexpr void operator()(const lexy::nullopt&) const {}

    constexpr void finish() && {}
};

/// A callback that just forwards an existing object.
template <typename T>
constexpr auto forward = _fwd<T>{};
} // namespace lexy

#endif // LEXY_CALLBACK_FORWARD_HPP_INCLUDED

