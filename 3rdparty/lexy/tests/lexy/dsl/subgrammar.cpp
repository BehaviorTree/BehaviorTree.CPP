// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/subgrammar.hpp>

#include "verify.hpp"
#include <lexy/action/match.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/callback/forward.hpp>

namespace lexy_subgrammar_test
{
struct production;
} // namespace lexy_subgrammar_test

LEXY_DECLARE_SUBGRAMMAR(lexy_subgrammar_test::production)

namespace
{
struct forward_value
{
    static constexpr auto value = lexy::forward<lexy_subgrammar_test::production*>;
};
} // namespace

TEST_CASE("dsl::subgrammar")
{
    constexpr auto rule
        = dsl::subgrammar<lexy_subgrammar_test::production, lexy_subgrammar_test::production*>;
    CHECK(lexy::is_rule<decltype(rule)>);

    SUBCASE("verify")
    {
        constexpr auto callback = +[](const char*, lexy_subgrammar_test::production*) { return 0; };

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace()
                     .production("production")
                     .expected_literal(0, "abc", 0)
                     .cancel()
                     .cancel());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().production("production").literal("abc"));
    }
    SUBCASE("match")
    {
        CHECK(!lexy::match<test_production_for<decltype(rule)>>(lexy::zstring_input("")));
        CHECK(lexy::match<test_production_for<decltype(rule)>>(lexy::zstring_input("abc")));
    }
    SUBCASE("parse")
    {
        struct production : test_production_for<decltype(rule)>, forward_value
        {};

        auto empty = lexy::parse<production>(lexy::zstring_input(""), lexy::noop);
        CHECK(!empty);
    }
}

