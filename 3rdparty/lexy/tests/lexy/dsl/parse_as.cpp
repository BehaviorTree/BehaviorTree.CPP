// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/parse_as.hpp>

#include "verify.hpp"
#include <lexy/callback/constant.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/production.hpp>

namespace
{
struct my_pos
{
    explicit constexpr my_pos(const char*) {}
};

struct inner_prod
{
    static constexpr auto name  = "inner_prod";
    static constexpr auto rule  = LEXY_LIT("abc");
    static constexpr auto value = lexy::constant(42);
};
} // namespace

TEST_CASE("dsl::parse_as")
{
    SUBCASE("rule")
    {
        static constexpr auto rule = dsl::parse_as<my_pos>(LEXY_LIT("abc") + dsl::position);
        CHECK(lexy::is_rule<decltype(rule)>);

        static constexpr auto callback = [](const char*, my_pos) { return 0; };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc").position());
    }
    SUBCASE("production")
    {
        static constexpr auto rule = dsl::parse_as<int>(dsl::p<inner_prod>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        static constexpr auto callback = [](const char*, int i) { return i; };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace()
                     .production("inner_prod")
                     .expected_literal(0, "abc", 0)
                     .cancel()
                     .cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().production("inner_prod").literal("abc"));
    }
}

