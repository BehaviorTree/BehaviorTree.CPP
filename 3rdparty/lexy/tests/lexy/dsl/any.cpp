// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/any.hpp>

#include "verify.hpp"

TEST_CASE("dsl::any")
{
    constexpr auto rule = lexy::dsl::any;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace().token("any", ""));

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().token("any", "abc"));

    auto invalid_utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, 'a', 'b', 'c', 0x80, '1', '2', '3');
    CHECK(invalid_utf8.status == test_result::success);
    CHECK(invalid_utf8.trace == test_trace().token("any", "abc\\x80123"));
}

