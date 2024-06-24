// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/eof.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::eof")
{
    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = dsl::eof;
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace().eof());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::recovered_error);
        CHECK(abc.trace == test_trace().error(0, 0, "expected EOF"));

        // 0xFF is the EOF marker for UTF-8 input.
        auto invalid_UTF8 = LEXY_VERIFY(lexy::utf8_encoding{}, 0xFF, 'a', 'b', 'c');
        CHECK(invalid_UTF8.status == test_result::success);
        CHECK(invalid_UTF8.trace == test_trace().eof());
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(dsl::eof);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace().eof());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace());

        // 0xFF is the EOF marker for UTF-8 input.
        auto invalid_UTF8 = LEXY_VERIFY(lexy::utf8_encoding{}, 0xFF, 'a', 'b', 'c');
        CHECK(invalid_UTF8.status == test_result::success);
        CHECK(invalid_UTF8.trace == test_trace().eof());
    }
}

