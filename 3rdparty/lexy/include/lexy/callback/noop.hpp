// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_NOOP_HPP_INCLUDED
#define LEXY_CALLBACK_NOOP_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
struct _noop
{
    using return_type = void;

    template <typename... Args>
    constexpr auto sink(const Args&...) const
    {
        // We don't need a separate type, noop itself can have the required functions.
        return *this;
    }

    template <typename... Args>
    constexpr void operator()(const Args&...) const
    {}

    constexpr void finish() && {}
};

/// A callback with sink that does nothing.
inline constexpr auto noop = _noop{};
} // namespace lexy

#endif // LEXY_CALLBACK_NOOP_HPP_INCLUDED

