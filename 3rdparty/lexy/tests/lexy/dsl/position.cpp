// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/position.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/loop.hpp>

TEST_CASE("dsl::position")
{
    constexpr auto pos = dsl::position;
    CHECK(lexy::is_rule<decltype(pos)>);
    constexpr auto rule = dsl::while_(dsl::lit_c<'a'>) + pos + dsl::while_(dsl::lit_c<'b'>);

    constexpr auto callback
        = [](const char* begin, const char* pos) { return static_cast<int>(pos - begin); };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace().position());

    auto one = LEXY_VERIFY("ab");
    CHECK(one.status == test_result::success);
    CHECK(one.value == 1);
    CHECK(one.trace == test_trace().literal("a").position().literal("b"));
    auto two = LEXY_VERIFY("aabb");
    CHECK(two.status == test_result::success);
    CHECK(two.value == 2);
    CHECK(two.trace == test_trace().literal("a").literal("a").position().literal("b").literal("b"));
    auto three = LEXY_VERIFY("aaabbb");
    CHECK(three.status == test_result::success);
    CHECK(three.value == 3);
    CHECK(three.trace
          == test_trace()
                 .literal("a")
                 .literal("a")
                 .literal("a")
                 .position()
                 .literal("b")
                 .literal("b")
                 .literal("b"));
}

TEST_CASE("dsl::position rule")
{
    constexpr auto pos = dsl::position(LEXY_LIT("abc"));

    constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                  [](const char* begin, const char* pos) {
                                                      CHECK(pos == begin);
                                                      return 1;
                                                  });

    SUBCASE("as rule")
    {
        constexpr auto rule = pos;
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().position().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().position().literal("abc"));
        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd.status == test_result::success);
        CHECK(abcd.value == 1);
        CHECK(abcd.trace == test_trace().position().literal("abc"));

        auto ad = LEXY_VERIFY("ad");
        CHECK(ad.status == test_result::fatal_error);
        CHECK(ad.trace
              == test_trace().position().error_token("a").expected_literal(0, "abc", 1).cancel());
    }
    SUBCASE("as branch rule")
    {
        constexpr auto rule = dsl::if_(pos);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().position().literal("abc"));
        auto abcd = LEXY_VERIFY("abcd");
        CHECK(abcd.status == test_result::success);
        CHECK(abcd.value == 1);
        CHECK(abcd.trace == test_trace().position().literal("abc"));

        auto ad = LEXY_VERIFY("ad");
        CHECK(ad.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(ad.trace == test_trace());
    }
}

