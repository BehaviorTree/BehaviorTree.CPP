// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/lookahead.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/until.hpp>

TEST_CASE("dsl::lookahead()")
{
    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    constexpr auto callback = token_callback;

    SUBCASE("literal rules")
    {
        constexpr auto condition = dsl::lookahead(LEXY_LIT("."), LEXY_LIT("!"));
        CHECK(lexy::is_branch_rule<decltype(condition)>);

        SUBCASE("as rule")
        {
            constexpr auto rule = condition;

            auto empty = LEXY_VERIFY("");
            CHECK(empty.status == test_result::recovered_error);
            CHECK(empty.trace == test_trace().error(0, 0, "lookahead failure"));

            auto nothing = LEXY_VERIFY("abc");
            CHECK(nothing.status == test_result::recovered_error);
            CHECK(nothing.trace
                  == test_trace().error(0, 3, "lookahead failure").backtracked("abc"));
            auto nothing_limit = LEXY_VERIFY("abc!def");
            CHECK(nothing_limit.status == test_result::recovered_error);
            CHECK(nothing_limit.trace
                  == test_trace().error(0, 4, "lookahead failure").backtracked("abc!"));

            auto something = LEXY_VERIFY("abc.");
            CHECK(something.status == test_result::success);
            CHECK(something.trace == test_trace().backtracked("abc."));
            auto something_limit = LEXY_VERIFY("abc.def!ghi");
            CHECK(something_limit.status == test_result::success);
            CHECK(something_limit.trace == test_trace().backtracked("abc."));

            auto limit_something = LEXY_VERIFY("abc!def.");
            CHECK(limit_something.status == test_result::recovered_error);
            CHECK(limit_something.trace
                  == test_trace().error(0, 4, "lookahead failure").backtracked("abc!"));
        }
        SUBCASE("as rule with .error")
        {
            constexpr auto rule = condition.error<my_error>;

            auto empty = LEXY_VERIFY("");
            CHECK(empty.status == test_result::recovered_error);
            CHECK(empty.trace == test_trace().error(0, 0, "my error"));

            auto nothing = LEXY_VERIFY("abc");
            CHECK(nothing.status == test_result::recovered_error);
            CHECK(nothing.trace == test_trace().error(0, 3, "my error").backtracked("abc"));
            auto nothing_limit = LEXY_VERIFY("abc!def");
            CHECK(nothing_limit.status == test_result::recovered_error);
            CHECK(nothing_limit.trace == test_trace().error(0, 4, "my error").backtracked("abc!"));

            auto something = LEXY_VERIFY("abc.");
            CHECK(something.status == test_result::success);
            CHECK(something.trace == test_trace().backtracked("abc."));
            auto something_limit = LEXY_VERIFY("abc.def!ghi");
            CHECK(something_limit.status == test_result::success);
            CHECK(something_limit.trace == test_trace().backtracked("abc."));

            auto limit_something = LEXY_VERIFY("abc!def.");
            CHECK(limit_something.status == test_result::recovered_error);
            CHECK(limit_something.trace
                  == test_trace().error(0, 4, "my error").backtracked("abc!"));
        }
        SUBCASE("as branch")
        {
            constexpr auto rule = dsl::if_(condition >> dsl::until(LEXY_LIT(".")));

            auto empty = LEXY_VERIFY("");
            CHECK(empty.status == test_result::success);
            CHECK(empty.trace == test_trace());

            auto nothing = LEXY_VERIFY("abc");
            CHECK(nothing.status == test_result::success);
            CHECK(nothing.trace == test_trace().backtracked("abc"));
            auto nothing_limit = LEXY_VERIFY("abc!def");
            CHECK(nothing_limit.status == test_result::success);
            CHECK(nothing_limit.trace == test_trace().backtracked("abc!"));

            auto something = LEXY_VERIFY("abc.");
            CHECK(something.status == test_result::success);
            CHECK(something.trace == test_trace().backtracked("abc.").token("any", "abc."));
            auto something_limit = LEXY_VERIFY("abc.def!ghi");
            CHECK(something_limit.status == test_result::success);
            CHECK(something_limit.trace == test_trace().backtracked("abc.").token("any", "abc."));

            auto limit_something = LEXY_VERIFY("abc!def.");
            CHECK(limit_something.status == test_result::success);
            CHECK(limit_something.trace == test_trace().backtracked("abc!"));
        }
    }
    SUBCASE("literal set")
    {
        constexpr auto rule = dsl::lookahead(dsl::literal_set(LEXY_LIT("."), LEXY_LIT(",")),
                                             LEXY_LITERAL_SET(LEXY_LIT("!"), LEXY_LIT("?")));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.trace == test_trace().error(0, 0, "lookahead failure"));

        auto nothing = LEXY_VERIFY("abc");
        CHECK(nothing.status == test_result::recovered_error);
        CHECK(nothing.trace == test_trace().error(0, 3, "lookahead failure").backtracked("abc"));
        auto nothing_limit1 = LEXY_VERIFY("abc!def");
        CHECK(nothing_limit1.status == test_result::recovered_error);
        CHECK(nothing_limit1.trace
              == test_trace().error(0, 4, "lookahead failure").backtracked("abc!"));
        auto nothing_limit2 = LEXY_VERIFY("abc?def");
        CHECK(nothing_limit2.status == test_result::recovered_error);
        CHECK(nothing_limit2.trace
              == test_trace().error(0, 4, "lookahead failure").backtracked("abc?"));

        auto something1 = LEXY_VERIFY("abc.");
        CHECK(something1.status == test_result::success);
        CHECK(something1.trace == test_trace().backtracked("abc."));
        auto something2 = LEXY_VERIFY("abc,");
        CHECK(something2.status == test_result::success);
        CHECK(something2.trace == test_trace().backtracked("abc,"));
        auto something1_limit = LEXY_VERIFY("abc.def!ghi");
        CHECK(something1_limit.status == test_result::success);
        CHECK(something1_limit.trace == test_trace().backtracked("abc."));
        auto something2_limit = LEXY_VERIFY("abc,def!ghi");
        CHECK(something2_limit.status == test_result::success);
        CHECK(something2_limit.trace == test_trace().backtracked("abc,"));

        auto limit1_something = LEXY_VERIFY("abc!def.");
        CHECK(limit1_something.status == test_result::recovered_error);
        CHECK(limit1_something.trace
              == test_trace().error(0, 4, "lookahead failure").backtracked("abc!"));
        auto limit2_something = LEXY_VERIFY("abc?def.");
        CHECK(limit2_something.status == test_result::recovered_error);
        CHECK(limit2_something.trace
              == test_trace().error(0, 4, "lookahead failure").backtracked("abc?"));
    }
}

