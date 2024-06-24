// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/action/trace.hpp>

#include "verify.hpp"

TEST_CASE("dsl::debug")
{
    constexpr auto rule = LEXY_DEBUG("hello");
    CHECK(lexy::is_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
    CHECK(equivalent_rules(rule, dsl::debug<"hello">));
#endif

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace().token("debug", "hello"));

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().token("debug", "hello"));
}

