// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/position.hpp>

#include "verify.hpp"
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

