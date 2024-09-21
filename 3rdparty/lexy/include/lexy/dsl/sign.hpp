// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SIGN_HPP_INCLUDED
#define LEXY_DSL_SIGN_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/operator.hpp>

namespace lexy
{
template <int I>
struct _sign
{
    constexpr operator int() const
    {
        return I;
    }
};

struct plus_sign : _sign<+1>
{};
struct minus_sign : _sign<-1>
{};
} // namespace lexy

namespace lexyd
{
struct _plus : decltype(op<lexy::plus_sign>(LEXY_LIT("+")))
{};
struct _minus : decltype(op<lexy::minus_sign>(LEXY_LIT("-")))
{};

/// Matches a plus sign or nothing, producing +1.
constexpr auto plus_sign = if_(_plus{});
/// Matches a minus sign or nothing, producing +1 or -1.
constexpr auto minus_sign = if_(_minus{});

/// Matches a plus or minus sign or nothing, producing +1 or -1.
constexpr auto sign = if_(_plus{} | _minus{});
} // namespace lexyd

#endif // LEXY_DSL_SIGN_HPP_INCLUDED

