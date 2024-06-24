// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/subgrammar.hpp>

#include "verify.hpp"

#include <lexy/action/match.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/callback/constant.hpp>

namespace lexy_subgrammar_test
{
struct production
{
    static constexpr auto name  = "production";
    static constexpr auto rule  = LEXY_LIT("abc");
    static constexpr auto value = lexy::constant(static_cast<production*>(nullptr));
};
} // namespace lexy_subgrammar_test

LEXY_DECLARE_SUBGRAMMAR(lexy_subgrammar_test::production) // Because we don't have a shared header
LEXY_DEFINE_SUBGRAMMAR(lexy_subgrammar_test::production)

LEXY_INSTANTIATE_SUBGRAMMAR(lexy_subgrammar_test::production,
                            lexy::match_action<void, lexy::string_input<>>)
LEXY_INSTANTIATE_SUBGRAMMAR(
    lexy_subgrammar_test::production,
    test_action<lexy::string_input<>, int (*)(const char*, lexy_subgrammar_test::production*)>)
LEXY_INSTANTIATE_SUBGRAMMAR(
    lexy_subgrammar_test::production,
    test_action<lexy::buffer<>, int (*)(const char*, lexy_subgrammar_test::production*)>)
LEXY_INSTANTIATE_SUBGRAMMAR(
    lexy_subgrammar_test::production,
    lexy::parse_action<void, lexy::string_input<>, LEXY_DECAY_DECLTYPE(lexy::noop)>)

