// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/separator.hpp>

#include "verify.hpp"

TEST_CASE("dsl::sep()")
{
    struct tag
    {
        static constexpr auto name()
        {
            return "tag";
        }
    };

    constexpr auto basic = dsl::sep(LEXY_LIT("abc"));
    CHECK(equivalent_rules(decltype(basic)::rule{}, LEXY_LIT("abc")));
    constexpr auto specify_error = basic.trailing_error<tag>;
    CHECK(equivalent_rules(decltype(specify_error)::rule{}, LEXY_LIT("abc")));

    SUBCASE("trailing rule, default error")
    {
        constexpr auto rule = decltype(basic)::trailing_rule{};
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto sep = LEXY_VERIFY("abc");
        CHECK(sep.status == test_result::recovered_error);
        CHECK(sep.trace
              == test_trace().literal("abc").error(0, 3, "unexpected trailing separator"));

        auto partial_sep = LEXY_VERIFY("ab");
        CHECK(partial_sep.status == test_result::success);
        CHECK(partial_sep.trace == test_trace());
    }
    SUBCASE("trailing rule, custom error")
    {
        constexpr auto rule = decltype(specify_error)::trailing_rule{};
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto sep = LEXY_VERIFY("abc");
        CHECK(sep.status == test_result::recovered_error);
        CHECK(sep.trace == test_trace().literal("abc").error(0, 3, "tag"));

        auto partial_sep = LEXY_VERIFY("ab");
        CHECK(partial_sep.status == test_result::success);
        CHECK(partial_sep.trace == test_trace());
    }
}

TEST_CASE("dsl::trailing_sep()")
{
    constexpr auto sep = dsl::trailing_sep(LEXY_LIT("abc"));
    CHECK(equivalent_rules(decltype(sep)::rule{}, LEXY_LIT("abc")));
    CHECK(equivalent_rules(decltype(sep)::trailing_rule{}, dsl::if_(LEXY_LIT("abc"))));
}

