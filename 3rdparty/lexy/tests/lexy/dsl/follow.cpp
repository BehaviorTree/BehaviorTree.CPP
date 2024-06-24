// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/follow.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/case_folding.hpp>

TEST_CASE("dsl::not_followed_by()")
{
    SUBCASE("basic")
    {
        constexpr auto rule = dsl::not_followed_by(LEXY_LIT("abc"), dsl::ascii::alpha);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());
        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == test_trace().expected_literal(0, "abc", 1).cancel());
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == test_trace().expected_literal(0, "abc", 2).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));

        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd.status == test_result::fatal_error);
        CHECK(abcd.trace
              == test_trace().error_token("abc").error(3, 3, "follow restriction").cancel());
    }
    SUBCASE("case folding")
    {
        constexpr auto rule
            = dsl::not_followed_by(dsl::ascii::case_folding(LEXY_LIT("abc")), dsl::lit_c<'d'>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());
        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == test_trace().expected_literal(0, "abc", 1).cancel());
        auto A = LEXY_VERIFY("A");
        CHECK(A.status == test_result::fatal_error);
        CHECK(A.trace == test_trace().expected_literal(0, "abc", 1).cancel());
        auto AB = LEXY_VERIFY("AB");
        CHECK(AB.status == test_result::fatal_error);
        CHECK(AB.trace == test_trace().expected_literal(0, "abc", 2).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
        auto ABC = LEXY_VERIFY("ABC");
        CHECK(ABC.status == test_result::success);
        CHECK(ABC.trace == test_trace().literal("ABC"));

        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd.status == test_result::fatal_error);
        CHECK(abcd.trace
              == test_trace().error_token("abc").error(3, 3, "follow restriction").cancel());
        auto ABCD = LEXY_VERIFY("ABCD");
        CHECK(ABCD.status == test_result::fatal_error);
        CHECK(ABCD.trace
              == test_trace().error_token("ABC").error(3, 3, "follow restriction").cancel());
    }
}

TEST_CASE("dsl::followed_by()")
{
    constexpr auto rule = dsl::followed_by(LEXY_LIT("abc"), dsl::ascii::alpha);
    CHECK(equivalent_rules(rule, dsl::not_followed_by(LEXY_LIT("abc"), -dsl::ascii::alpha)));
}

