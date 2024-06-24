// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/until.hpp>

#include "verify.hpp"
#include <lexy/dsl/newline.hpp>

TEST_CASE("dsl::until()")
{
    constexpr auto callback = token_callback;

    SUBCASE("basic")
    {
        constexpr auto rule = dsl::until(LEXY_LIT("!"));
        CHECK(lexy::is_token_rule<decltype(rule)>);

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
    SUBCASE("swar")
    {
        constexpr auto rule = dsl::until(dsl::newline);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::utf8_char_encoding{}, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "expected newline").cancel());

        auto zero = LEXY_VERIFY(lexy::utf8_char_encoding{}, "\n");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("any", "\\n"));

        auto few = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abc\r\n");
        CHECK(few.status == test_result::success);
        CHECK(few.trace == test_trace().token("any", "abc\\r\\n"));

        auto many = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abcdefghijklmnopqrstuvwxyz\n");
        CHECK(many.status == test_result::success);
        CHECK(many.trace == test_trace().token("any", "abcdefghijklmnopqrstuvwxyz\\n"));

        auto partial_before
            = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abcdefghijklmno\rpqrstuvwxyz\n");
        CHECK(partial_before.status == test_result::success);
        CHECK(partial_before.trace
              == test_trace().token("any", "abcdefghijklmno\\rpqrstuvwxyz\\n"));

        auto unterminated = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abcdefghijklmnopqrstuvwxyz");
        CHECK(unterminated.status == test_result::fatal_error);
        CHECK(unterminated.trace
              == test_trace()
                     .error_token("abcdefghijklmnopqrstuvwxyz")
                     .error(26, 26, "expected newline")
                     .cancel());
    }
}

TEST_CASE("dsl::until().or_eof()")
{
    constexpr auto callback = token_callback;

    SUBCASE("basic")
    {
        constexpr auto rule = dsl::until(LEXY_LIT("!")).or_eof();
        CHECK(lexy::is_token_rule<decltype(rule)>);

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
    SUBCASE("swar")
    {
        constexpr auto rule = dsl::until(dsl::newline).or_eof();
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::utf8_char_encoding{}, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace().token("any", ""));

        auto zero = LEXY_VERIFY(lexy::utf8_char_encoding{}, "\n");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("any", "\\n"));

        auto few = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abc\r\n");
        CHECK(few.status == test_result::success);
        CHECK(few.trace == test_trace().token("any", "abc\\r\\n"));

        auto many = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abcdefghijklmnopqrstuvwxyz\n");
        CHECK(many.status == test_result::success);
        CHECK(many.trace == test_trace().token("any", "abcdefghijklmnopqrstuvwxyz\\n"));

        auto partial_before
            = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abcdefghijklmno\rpqrstuvwxyz\n");
        CHECK(partial_before.status == test_result::success);
        CHECK(partial_before.trace
              == test_trace().token("any", "abcdefghijklmno\\rpqrstuvwxyz\\n"));

        auto unterminated = LEXY_VERIFY(lexy::utf8_char_encoding{}, "abcdefghijklmnopqrstuvwxyz");
        CHECK(unterminated.status == test_result::success);
        CHECK(unterminated.trace == test_trace().token("any", "abcdefghijklmnopqrstuvwxyz"));
    }
}

