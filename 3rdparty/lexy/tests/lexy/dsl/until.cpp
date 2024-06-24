// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/until.hpp>

#include "verify.hpp"

TEST_CASE("dsl::until()")
{
    constexpr auto rule = dsl::until(LEXY_LIT("!"));
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "!", 0).cancel());

    auto zero = LEXY_VERIFY("!");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("any", "!"));
    auto one = LEXY_VERIFY("a!");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().token("any", "a!"));
    auto two = LEXY_VERIFY("ab!");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().token("any", "ab!"));
    auto three = LEXY_VERIFY("abc!");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().token("any", "abc!"));

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated.status == test_result::fatal_error);
    CHECK(unterminated.trace
          == test_trace().error_token("abc").expected_literal(3, "!", 0).cancel());

    auto invalid_utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, 'a', 'b', 'c', 0x80, '!');
    CHECK(invalid_utf8.status == test_result::success);
    CHECK(invalid_utf8.trace == test_trace().token("any", "abc\\x80!"));
}

TEST_CASE("dsl::until().or_eof()")
{
    constexpr auto rule = dsl::until(LEXY_LIT("!")).or_eof();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace().token("any", ""));

    auto zero = LEXY_VERIFY("!");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("any", "!"));
    auto one = LEXY_VERIFY("a!");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().token("any", "a!"));
    auto two = LEXY_VERIFY("ab!");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().token("any", "ab!"));
    auto three = LEXY_VERIFY("abc!");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().token("any", "abc!"));

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated.status == test_result::success);
    CHECK(unterminated.trace == test_trace().token("any", "abc"));

    auto invalid_utf8 = LEXY_VERIFY(lexy::utf8_encoding{}, 'a', 'b', 'c', 0x80, '!');
    CHECK(invalid_utf8.status == test_result::success);
    CHECK(invalid_utf8.trace == test_trace().token("any", "abc\\x80!"));
}

