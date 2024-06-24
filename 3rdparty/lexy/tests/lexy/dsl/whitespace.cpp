// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/whitespace.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/recover.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};
} // namespace

TEST_CASE("automatic whitespace")
{
    constexpr auto rule = dsl::lit_c<'x'>;
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; }, [](const char*, auto) { return 0; });

    SUBCASE("no whitespace")
    {
        auto abc = LEXY_VERIFY("x");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("x"));
    }
    SUBCASE("direct parent has whitespace")
    {
        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto ws = LEXY_VERIFY_P(production, "..x..");
        CHECK(ws.status == test_result::success);
        CHECK(ws.trace == test_trace().whitespace("..").literal("x").whitespace(".."));
    }
    SUBCASE("indirect parent has whitespace")
    {
        struct inner : production_for<decltype(rule)>
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };

        struct production : test_production_for<decltype(dsl::p<inner>)>, with_whitespace
        {};

        auto ws = LEXY_VERIFY_P(production, "..x..");
        CHECK(ws.status == test_result::success);
        CHECK(ws.trace
              == test_trace().whitespace("..").production("inner").literal("x").whitespace(".."));
    }
    SUBCASE("indirect non-root parent has whitespace")
    {
        struct inner : production_for<decltype(rule)>
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };

        struct mid : production_for<decltype(dsl::p<inner>)>, with_whitespace
        {
            static constexpr auto name()
            {
                return "mid";
            }
        };

        struct production : test_production_for<decltype(dsl::p<mid>)>
        {};

        auto ws = LEXY_VERIFY_P(production, "..x..");
        CHECK(ws.status == test_result::success);
        CHECK(ws.trace
              == test_trace()
                     .production("mid")
                     .whitespace("..")
                     .production("inner")
                     .literal("x")
                     .whitespace(".."));
    }
    SUBCASE("token production disables whitespace")
    {
        struct inner : production_for<decltype(rule + rule)>, lexy::token_production
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };

        struct production : test_production_for<decltype(dsl::p<inner>)>, with_whitespace
        {};

        auto ws = LEXY_VERIFY_P(production, "..xx..");
        CHECK(ws.status == test_result::success);
        CHECK(ws.trace
              == test_trace()
                     .whitespace("..")
                     .production("inner")
                     .literal("x")
                     .literal("x")
                     .finish()
                     .whitespace(".."));

        auto inner = LEXY_VERIFY_P(production, "..x..x..");
        CHECK(inner.status == test_result::fatal_error);
        CHECK(inner.trace
              == test_trace()
                     .whitespace("..")
                     .production("inner")
                     .literal("x")
                     .expected_literal(3, "x", 0)
                     .cancel()
                     .cancel());
    }
    SUBCASE("token production has whitespace")
    {
        struct inner : production_for<decltype(rule)>
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };

        struct token : production_for<decltype(dsl::p<inner>)>,
                       lexy::token_production,
                       with_whitespace
        {
            static constexpr auto name()
            {
                return "token";
            }
        };

        struct production : test_production_for<decltype(dsl::p<token>)>
        {};

        auto ws = LEXY_VERIFY_P(production, "..x..");
        CHECK(ws.status == test_result::success);
        CHECK(ws.trace
              == test_trace()
                     .production("token")
                     .whitespace("..")
                     .production("inner")
                     .literal("x")
                     .whitespace(".."));
    }
}

TEST_CASE("dsl::whitespace")
{
    constexpr auto callback = token_callback;

    SUBCASE("token")
    {
        constexpr auto rule = dsl::whitespace(LEXY_LIT("-"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY("-");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().whitespace("-"));
        auto two = LEXY_VERIFY("--");
        CHECK(two.status == test_result::success);
        CHECK(two.trace == test_trace().whitespace("--"));
        auto three = LEXY_VERIFY("---");
        CHECK(three.status == test_result::success);
        CHECK(three.trace == test_trace().whitespace("---"));

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto leading_whitespace = LEXY_VERIFY_P(production, "..--");
        CHECK(leading_whitespace.status == test_result::success);
        CHECK(leading_whitespace.trace == test_trace().whitespace("..").whitespace("--"));
        auto inner_whitespace = LEXY_VERIFY_P(production, "-..-");
        CHECK(inner_whitespace.status == test_result::success);
        CHECK(inner_whitespace.trace == test_trace().whitespace("-"));
        auto trailing_whitespace = LEXY_VERIFY_P(production, "--..");
        CHECK(trailing_whitespace.status == test_result::success);
        CHECK(trailing_whitespace.trace == test_trace().whitespace("--"));
    }
    SUBCASE("branch")
    {
        constexpr auto rule = dsl::whitespace(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY("abc");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().whitespace("abc"));
        auto two = LEXY_VERIFY("abcabc");
        CHECK(two.status == test_result::success);
        CHECK(two.trace == test_trace().whitespace("abcabc"));
        auto three = LEXY_VERIFY("abcabcabc");
        CHECK(three.status == test_result::success);
        CHECK(three.trace == test_trace().whitespace("abcabcabc"));

        auto ws_failure = LEXY_VERIFY("abd");
        CHECK(ws_failure.status == test_result::fatal_error);
        CHECK(ws_failure.trace
              == test_trace().expected_literal(1, "bc", 1).error_token("ab").cancel());

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto leading_whitespace = LEXY_VERIFY_P(production, "..abc");
        CHECK(leading_whitespace.status == test_result::success);
        CHECK(leading_whitespace.trace == test_trace().whitespace("..").whitespace("abc"));
        auto inner_whitespace = LEXY_VERIFY_P(production, "ab..c");
        CHECK(inner_whitespace.status == test_result::fatal_error);
        CHECK(inner_whitespace.trace
              == test_trace().expected_literal(1, "bc", 1).error_token("ab").cancel());
        auto trailing_whitespace = LEXY_VERIFY_P(production, "abc..");
        CHECK(trailing_whitespace.status == test_result::success);
        CHECK(trailing_whitespace.trace == test_trace().whitespace("abc"));
    }
    SUBCASE("choice")
    {
        constexpr auto rule
            = dsl::whitespace(LEXY_LIT("a") >> LEXY_LIT("bc") | LEXY_LIT("b") >> LEXY_LIT("cd"));
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY("abc");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().whitespace("abc"));
        auto two = LEXY_VERIFY("abcbcd");
        CHECK(two.status == test_result::success);
        CHECK(two.trace == test_trace().whitespace("abcbcd"));
        auto three = LEXY_VERIFY("bcdabcbcd");
        CHECK(three.status == test_result::success);
        CHECK(three.trace == test_trace().whitespace("bcdabcbcd"));

        auto ws_failure = LEXY_VERIFY("abd");
        CHECK(ws_failure.status == test_result::fatal_error);
        CHECK(ws_failure.trace
              == test_trace().expected_literal(1, "bc", 1).error_token("ab").cancel());

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto leading_whitespace = LEXY_VERIFY_P(production, "..abc");
        CHECK(leading_whitespace.status == test_result::success);
        CHECK(leading_whitespace.trace == test_trace().whitespace("..").whitespace("abc"));
        auto inner_whitespace = LEXY_VERIFY_P(production, "ab..c");
        CHECK(inner_whitespace.status == test_result::fatal_error);
        CHECK(inner_whitespace.trace
              == test_trace().expected_literal(1, "bc", 1).error_token("ab").cancel());
        auto trailing_whitespace = LEXY_VERIFY_P(production, "abc..");
        CHECK(trailing_whitespace.status == test_result::success);
        CHECK(trailing_whitespace.trace == test_trace().whitespace("abc"));
    }

    SUBCASE("operator|")
    {
        CHECK(equivalent_rules(dsl::whitespace(dsl::lit_c<'a'>) | dsl::lit_c<'b'>,
                               dsl::whitespace(dsl::lit_c<'a'> | dsl::lit_c<'b'>)));
        CHECK(equivalent_rules(dsl::lit_c<'a'> | dsl::whitespace(dsl::lit_c<'b'>),
                               dsl::whitespace(dsl::lit_c<'a'> | dsl::lit_c<'b'>)));
    }
}

TEST_CASE("dsl::no_whitespace")
{
    constexpr auto no_ws = dsl::no_whitespace(LEXY_LIT("ab") >> dsl::try_(LEXY_LIT("c")));
    CHECK(lexy::is_branch_rule<decltype(no_ws)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        struct production : test_production_for<decltype(no_ws)>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "ab", 0).cancel());

        auto ab = LEXY_VERIFY_P(production, "ab");
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.trace == test_trace().literal("ab").expected_literal(2, "c", 0));
        auto abc = LEXY_VERIFY_P(production, "abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("ab").literal("c"));

        auto leading_whitespace = LEXY_VERIFY_P(production, "..abc");
        CHECK(leading_whitespace.status == test_result::success);
        CHECK(leading_whitespace.trace == test_trace().whitespace("..").literal("ab").literal("c"));
        auto inner_whitespace = LEXY_VERIFY_P(production, "ab..c");
        CHECK(inner_whitespace.status == test_result::recovered_error);
        CHECK(inner_whitespace.trace
              == test_trace().literal("ab").expected_literal(2, "c", 0).whitespace(".."));
        auto trailing_whitespace = LEXY_VERIFY_P(production, "abc..");
        CHECK(trailing_whitespace.status == test_result::success);
        CHECK(trailing_whitespace.trace
              == test_trace().literal("ab").literal("c").whitespace(".."));
    }
    SUBCASE("as branch")
    {
        struct production : test_production_for<decltype(dsl::if_(no_ws))>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto ab = LEXY_VERIFY_P(production, "ab");
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.trace == test_trace().literal("ab").expected_literal(2, "c", 0));
        auto abc = LEXY_VERIFY_P(production, "abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("ab").literal("c"));

        auto leading_whitespace = LEXY_VERIFY_P(production, "..abc");
        CHECK(leading_whitespace.status == test_result::success);
        CHECK(leading_whitespace.trace == test_trace().whitespace("..").literal("ab").literal("c"));
        auto inner_whitespace = LEXY_VERIFY_P(production, "ab..c");
        CHECK(inner_whitespace.status == test_result::recovered_error);
        CHECK(inner_whitespace.trace
              == test_trace().literal("ab").expected_literal(2, "c", 0).whitespace(".."));
        auto trailing_whitespace = LEXY_VERIFY_P(production, "abc..");
        CHECK(trailing_whitespace.status == test_result::success);
        CHECK(trailing_whitespace.trace
              == test_trace().literal("ab").literal("c").whitespace(".."));
    }
}

