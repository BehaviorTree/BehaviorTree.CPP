// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/peek.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/position.hpp>

TEST_CASE("dsl::peek()")
{
    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    constexpr auto condition = dsl::peek(LEXY_LIT("a") + dsl::position + LEXY_LIT("b"));
    CHECK(lexy::is_branch_rule<decltype(condition)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = condition;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.trace == test_trace().error(0, 0, "peek failure"));

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::recovered_error);
        CHECK(a.trace == test_trace().error(0, 1, "peek failure").backtracked("a"));

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.trace == test_trace().backtracked("ab"));
    }
    SUBCASE("as rule with .error")
    {
        constexpr auto rule = condition.error<my_error>;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error"));

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::recovered_error);
        CHECK(a.trace == test_trace().error(0, 1, "my error").backtracked("a"));

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.trace == test_trace().backtracked("ab"));
    }

    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(condition >> LEXY_LIT("a"));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().backtracked("a"));

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.trace == test_trace().backtracked("ab").literal("a"));
    }
}

TEST_CASE("dsl::peek_not()")
{
    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    constexpr auto condition = dsl::peek_not(LEXY_LIT("a") + dsl::position + LEXY_LIT("b"));
    CHECK(lexy::is_branch_rule<decltype(condition)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = condition;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().backtracked("a"));

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.trace == test_trace().error(0, 2, "unexpected").backtracked("ab"));
    }
    SUBCASE("as rule with .error")
    {
        constexpr auto rule = condition.error<my_error>;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().backtracked("a"));

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.trace == test_trace().error(0, 2, "my error").backtracked("ab"));
    }

    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(condition >> LEXY_LIT("a"));

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.trace == test_trace().backtracked("a").literal("a"));

        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.trace == test_trace().backtracked("ab"));
    }
}

