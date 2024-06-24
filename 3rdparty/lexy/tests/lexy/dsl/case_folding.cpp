// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/case_folding.hpp>

#include "verify.hpp"

TEST_CASE("dsl::ascii::case_folding")
{
    constexpr auto rule = dsl::ascii::case_folding(LEXY_LIT("abc"));
    CHECK(lexy::is_literal_rule<decltype(rule)>);

    auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

    auto a = LEXY_VERIFY("a");
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == test_trace().error_token("a").expected_literal(0, "abc", 1).cancel());
    auto ab = LEXY_VERIFY("ab");
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());
    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("abc"));
    auto abcd = LEXY_VERIFY("abcd");
    CHECK(abcd.status == test_result::success);
    CHECK(abcd.trace == test_trace().literal("abc"));

    auto AB = LEXY_VERIFY("AB");
    CHECK(AB.status == test_result::fatal_error);
    CHECK(AB.trace == test_trace().error_token("AB").expected_literal(0, "abc", 2).cancel());
    auto ABC = LEXY_VERIFY("ABC");
    CHECK(ABC.status == test_result::success);
    CHECK(ABC.trace == test_trace().literal("ABC"));

    auto aBc = LEXY_VERIFY("aBc");
    CHECK(aBc.status == test_result::success);
    CHECK(aBc.trace == test_trace().literal("aBc"));
}

TEST_CASE("dsl::unicode::simple_case_folding, UTF-32")
{
    constexpr auto rule = dsl::unicode::simple_case_folding(LEXY_LIT(U"abć"));
    CHECK(lexy::is_literal_rule<decltype(rule)>);

    auto callback = token_callback;

    auto empty = LEXY_VERIFY(U"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "ab\\u0107", 0).cancel());

    auto a = LEXY_VERIFY(U"a");
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == test_trace().error_token("a").expected_literal(0, "ab\\u0107", 1).cancel());
    auto ab = LEXY_VERIFY(U"ab");
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().error_token("ab").expected_literal(0, "ab\\u0107", 2).cancel());
    auto abc = LEXY_VERIFY(U"abć");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("ab\\u0107"));
    auto abcd = LEXY_VERIFY(U"abćd");
    CHECK(abcd.status == test_result::success);
    CHECK(abcd.trace == test_trace().literal("ab\\u0107"));

    auto AB = LEXY_VERIFY(U"AB");
    CHECK(AB.status == test_result::fatal_error);
    CHECK(AB.trace == test_trace().error_token("AB").expected_literal(0, "ab\\u0107", 2).cancel());
    auto ABC = LEXY_VERIFY(U"ABĆ");
    CHECK(ABC.status == test_result::success);
    CHECK(ABC.trace == test_trace().literal("AB\\u0106"));

    auto aBc = LEXY_VERIFY(U"aBć");
    CHECK(aBc.status == test_result::success);
    CHECK(aBc.trace == test_trace().literal("aB\\u0107"));
}

TEST_CASE("dsl::unicode::simple_case_folding, UTF-8 and UTF-16")
{
    constexpr auto rule
        = dsl::unicode::simple_case_folding(dsl::_lit<LEXY_CHAR8_T, 'a', 'b', 0xC4, 0x87>{});
    CHECK(lexy::is_literal_rule<decltype(rule)>);

    auto callback = token_callback;

    auto empty = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR(""));
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "ab\\u0107", 0).cancel());

    auto a = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("a"));
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == test_trace().error_token("a").expected_literal(0, "ab\\u0107", 1).cancel());
    auto ab = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("ab"));
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().error_token("ab").expected_literal(0, "ab\\u0107", 2).cancel());
    auto abc = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("abć"));
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("ab\\u0107"));
    auto abcd = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("abćd"));
    CHECK(abcd.status == test_result::success);
    CHECK(abcd.trace == test_trace().literal("ab\\u0107"));

    auto AB = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("AB"));
    CHECK(AB.status == test_result::fatal_error);
    CHECK(AB.trace == test_trace().error_token("AB").expected_literal(0, "ab\\u0107", 2).cancel());
    auto ABC = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("ABĆ"));
    CHECK(ABC.status == test_result::success);
    CHECK(ABC.trace == test_trace().literal("AB\\u0106"));

    auto aBc = LEXY_VERIFY(lexy::utf8_encoding{}, LEXY_CHAR8_STR("aBć"));
    CHECK(aBc.status == test_result::success);
    CHECK(aBc.trace == test_trace().literal("aB\\u0107"));

    auto partial = LEXY_VERIFY(lexy::utf8_encoding{}, 'a', 'b', 0xC4);
    CHECK(partial.status == test_result::fatal_error);
    CHECK(partial.trace
          == test_trace().error_token("ab\\xC4").expected_literal(0, "ab\\u0107", 3).cancel());

    auto different = LEXY_VERIFY(lexy::utf8_encoding{}, 'a', 'b', 0xC4, 0x88);
    CHECK(different.status == test_result::fatal_error);
    CHECK(different.trace
          == test_trace().error_token("ab").expected_literal(0, "ab\\u0107", 2).cancel());
}

