// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/production.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/recover.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};
} // namespace

TEST_CASE("dsl::inline_")
{
    struct production : production_for<decltype(LEXY_LIT("abc"))>
    {};

    constexpr auto rule = dsl::inline_<production>;
    CHECK(lexy::is_branch_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, LEXY_LIT("abc")));
}

TEST_CASE("dsl::p")
{
    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; }, [](const char*, auto) { return 1; });

    SUBCASE("as rule")
    {
        struct production : production_for<decltype(dsl::capture(LEXY_LIT("a")) + dsl::position
                                                    + dsl::try_(LEXY_LIT("bc")))>
        {
            static constexpr auto name()
            {
                return "production";
            }
        };

        constexpr auto rule = dsl::p<production>;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        auto empty_trace
            = test_trace().production("production").expected_literal(0, "a", 0).cancel().cancel();
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == empty_trace);

        auto a       = LEXY_VERIFY("a");
        auto a_trace = test_trace()
                           .production("production")
                           .literal("a")
                           .position()
                           .expected_literal(1, "bc", 0);
        CHECK(a.status == test_result::recovered_error);
        CHECK(a.value == 1);
        CHECK(a.trace == a_trace);
        auto ab       = LEXY_VERIFY("ab");
        auto ab_trace = test_trace()
                            .production("production")
                            .literal("a")
                            .position()
                            .error_token("b")
                            .expected_literal(1, "bc", 1);
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.value == 1);
        CHECK(ab.trace == ab_trace);

        auto abc = LEXY_VERIFY("abc");
        auto abc_trace
            = test_trace().production("production").literal("a").position().literal("bc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == abc_trace);
    }
    SUBCASE("as branch")
    {
        struct production : production_for<decltype(dsl::capture(LEXY_LIT("a"))
                                                    >> dsl::position + dsl::try_(LEXY_LIT("bc")))>
        {
            static constexpr auto name()
            {
                return "production";
            }
        };

        constexpr auto rule = dsl::if_(dsl::p<production>);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty       = LEXY_VERIFY("");
        auto empty_trace = test_trace().production("production").cancel();
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == empty_trace);

        auto a       = LEXY_VERIFY("a");
        auto a_trace = test_trace()
                           .production("production")
                           .literal("a")
                           .position()
                           .expected_literal(1, "bc", 0);
        CHECK(a.status == test_result::recovered_error);
        CHECK(a.value == 1);
        CHECK(a.trace == a_trace);
        auto ab       = LEXY_VERIFY("ab");
        auto ab_trace = test_trace()
                            .production("production")
                            .literal("a")
                            .position()
                            .error_token("b")
                            .expected_literal(1, "bc", 1);
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.value == 1);
        CHECK(ab.trace == ab_trace);

        auto abc = LEXY_VERIFY("abc");
        auto abc_trace
            = test_trace().production("production").literal("a").position().literal("bc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == abc_trace);
    }
    SUBCASE("as nested branch")
    {
        struct production : production_for<decltype(dsl::capture(LEXY_LIT("a"))
                                                    >> dsl::position + dsl::try_(LEXY_LIT("bc")))>
        {
            static constexpr auto name()
            {
                return "production";
            }
        };

        struct nested : production_for<decltype(dsl::p<production>)>
        {
            static constexpr auto name()
            {
                return "nested";
            }
        };

        constexpr auto rule = dsl::if_(dsl::p<nested>);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        auto empty_trace
            = test_trace().production("nested").production("production").cancel().cancel();
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == empty_trace);

        auto a       = LEXY_VERIFY("a");
        auto a_trace = test_trace()
                           .production("nested")
                           .production("production")
                           .literal("a")
                           .position()
                           .expected_literal(1, "bc", 0);
        CHECK(a.status == test_result::recovered_error);
        CHECK(a.value == 1);
        CHECK(a.trace == a_trace);
        auto ab       = LEXY_VERIFY("ab");
        auto ab_trace = test_trace()
                            .production("nested")
                            .production("production")
                            .literal("a")
                            .position()
                            .error_token("b")
                            .expected_literal(1, "bc", 1);
        CHECK(ab.status == test_result::recovered_error);
        CHECK(ab.value == 1);
        CHECK(ab.trace == ab_trace);

        auto abc       = LEXY_VERIFY("abc");
        auto abc_trace = test_trace()
                             .production("nested")
                             .production("production")
                             .literal("a")
                             .position()
                             .literal("bc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == abc_trace);
    }

    SUBCASE("token production")
    {
        struct inner : production_for<decltype(LEXY_LIT("ab") + LEXY_LIT("c"))>,
                       lexy::token_production
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };

        struct production : test_production_for<decltype(dsl::p<inner>)>, with_whitespace
        {};

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().production("inner").expected_literal(0, "ab", 0).cancel().cancel());

        auto abc = LEXY_VERIFY_P(production, "abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().production("inner").literal("ab").literal("c"));

        auto leading_ws = LEXY_VERIFY_P(production, "..abc");
        CHECK(leading_ws.status == test_result::success);
        CHECK(leading_ws.trace
              == test_trace().whitespace("..").production("inner").literal("ab").literal("c"));
        auto inner_ws = LEXY_VERIFY_P(production, "ab..c");
        CHECK(inner_ws.status == test_result::fatal_error);
        CHECK(inner_ws.trace
              == test_trace()
                     .production("inner")
                     .literal("ab")
                     .expected_literal(2, "c", 0)
                     .cancel()
                     .cancel());
        auto trailing_ws = LEXY_VERIFY_P(production, "abc..");
        CHECK(trailing_ws.status == test_result::success);
        CHECK(trailing_ws.trace
              == test_trace().production("inner").literal("ab").literal("c").finish().whitespace(
                  ".."));
    }
}

namespace
{
template <std::size_t N>
struct with_max_depth
{
    static constexpr auto max_recursion_depth = N;
};
} // namespace

TEST_CASE("dsl::recurse")
{
    constexpr auto rec = dsl::recurse<struct test>;
    CHECK(lexy::is_rule<decltype(rec)>);

    SUBCASE("direct recursion")
    {
        struct production
        : test_production_for<decltype(dsl::if_(LEXY_LIT("a") >> dsl::recurse<production>))>
        {};

        constexpr auto callback
            = lexy::callback<int>([](const char*) { return 0; },
                                  [](const char*, int count) { return count + 1; });

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto one       = LEXY_VERIFY_P(production, "a");
        auto one_trace = test_trace().literal("a").production("test_production");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);

        auto two       = LEXY_VERIFY_P(production, "aa");
        auto two_trace = test_trace(one_trace).literal("a").production("test_production");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);

        auto three       = LEXY_VERIFY_P(production, "aaa");
        auto three_trace = test_trace(two_trace).literal("a").production("test_production");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);
    }
    SUBCASE("indirect recursion")
    {
        struct production;
        struct inner : production_for<decltype(dsl::recurse<production>)>
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };
        struct production : test_production_for<decltype(dsl::if_(LEXY_LIT("a") >> dsl::p<inner>))>
        {};

        constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                      [](const char*, inner) { return 1; });

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY_P(production, "a");
        auto one_trace
            = test_trace().literal("a").production("inner").production("test_production");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);

        auto two = LEXY_VERIFY_P(production, "aa");
        auto two_trace
            = test_trace(one_trace).literal("a").production("inner").production("test_production");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 1);
        CHECK(two.trace == two_trace);

        auto three = LEXY_VERIFY_P(production, "aaa");
        auto three_trace
            = test_trace(two_trace).literal("a").production("inner").production("test_production");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 1);
        CHECK(three.trace == three_trace);
    }

    SUBCASE("token production")
    {
        struct inner : production_for<decltype(LEXY_LIT("ab") + LEXY_LIT("c"))>,
                       lexy::token_production
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };

        // Not actually using recursive, but the whitespace behavior should be the same.
        struct production : test_production_for<decltype(dsl::recurse<inner>)>, with_whitespace
        {};

        constexpr auto callback = [](const char*, inner) { return 0; };

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().production("inner").expected_literal(0, "ab", 0).cancel().cancel());

        auto abc = LEXY_VERIFY_P(production, "abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().production("inner").literal("ab").literal("c"));

        auto leading_ws = LEXY_VERIFY_P(production, "..abc");
        CHECK(leading_ws.status == test_result::success);
        CHECK(leading_ws.trace
              == test_trace().whitespace("..").production("inner").literal("ab").literal("c"));
        auto inner_ws = LEXY_VERIFY_P(production, "ab..c");
        CHECK(inner_ws.status == test_result::fatal_error);
        CHECK(inner_ws.trace
              == test_trace()
                     .production("inner")
                     .literal("ab")
                     .expected_literal(2, "c", 0)
                     .cancel()
                     .cancel());
        auto trailing_ws = LEXY_VERIFY_P(production, "abc..");
        CHECK(trailing_ws.status == test_result::success);
        CHECK(trailing_ws.trace
              == test_trace().production("inner").literal("ab").literal("c").finish().whitespace(
                  ".."));
    }

    SUBCASE("max depth")
    {
        struct production;
        struct inner : production_for<decltype(dsl::recurse<production> + dsl::recurse<production>)>
        {
            static constexpr auto name()
            {
                return "inner";
            }
        };
        struct production : test_production_for<decltype(dsl::if_(LEXY_LIT("a") >> dsl::p<inner>))>,
                            with_max_depth<3>
        {};

        constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                      [](const char*, inner) { return 1; });

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY_P(production, "a");
        // clang-format off
        auto one_trace = test_trace()
                             .literal("a")
                             .production("inner")
                                 .production("test_production").finish()
                                 .production("test_production").finish()
                                 .finish();
        // clang-format on
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);

        auto two = LEXY_VERIFY_P(production, "aa");
        // clang-format off
        auto two_trace = test_trace()
                             .literal("a")
                             .production("inner")
                                 .production("test_production")
                                     .literal("a")
                                     .production("inner")
                                         .production("test_production").finish()
                                         .production("test_production").finish()
                                         .finish()
                                     .finish()
                                 .production("test_production").finish()
                                 .finish();
        // clang-format on
        CHECK(two.status == test_result::success);
        CHECK(two.value == 1);
        CHECK(two.trace == two_trace);

        auto three = LEXY_VERIFY_P(production, "aaa");
        // clang-format off
        auto three_trace = test_trace()
                             .literal("a")
                             .production("inner")
                                 .production("test_production")
                                     .literal("a")
                                     .production("inner")
                                         .production("test_production")
                                             .literal("a")
                                             .production("inner")
                                                 .production("test_production").finish()
                                                 .production("test_production").finish()
                                                 .finish()
                                             .finish()
                                         .production("test_production").finish()
                                         .finish()
                                     .finish()
                                 .production("test_production").finish()
                                 .finish();
        // clang-format on
        CHECK(three.status == test_result::success);
        CHECK(three.value == 1);
        CHECK(three.trace == three_trace);

        auto four = LEXY_VERIFY_P(production, "aaaa");
        // clang-format off
        auto four_trace
            = test_trace()
                 .literal("a")
                 .production("inner")
                     .production("test_production")
                         .literal("a")
                         .production("inner")
                             .production("test_production")
                                 .literal("a")
                                 .production("inner")
                                     .production("test_production")
                                         .literal("a")
                                         .production("inner")
                                         .error(4, 4, "maximum recursion depth exceeded")
                                         .cancel()
                                     .cancel()
                                 .cancel()
                             .cancel()
                         .cancel()
                     .cancel()
                 .cancel()
              .cancel();
        // clang-format on
        CHECK(four.status == test_result::fatal_error);
        CHECK(four.trace == four_trace);
    }
}

TEST_CASE("dsl::recurse_branch")
{
    constexpr auto rec = dsl::recurse_branch<struct test>;
    CHECK(lexy::is_branch_rule<decltype(rec)>);

    SUBCASE("direct recursion")
    {
        struct production
        : test_production_for<decltype(LEXY_LIT("b") >> dsl::if_(dsl::recurse_branch<production>)
                                       | LEXY_LIT("a"))>
        {};

        constexpr auto callback
            = lexy::callback<int>([](const char*) { return 0; },
                                  [](const char*, int count) { return count + 1; });

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted choice").cancel());

        auto a = LEXY_VERIFY_P(production, "a");
        CHECK(a.status == test_result::success);
        CHECK(a.value == 0);
        CHECK(a.trace == test_trace().literal("a"));
        auto b = LEXY_VERIFY_P(production, "b");
        CHECK(b.status == test_result::success);
        CHECK(b.value == 0);
        CHECK(b.trace == test_trace().literal("b").production("test_production").cancel());

        auto ba = LEXY_VERIFY_P(production, "ba");
        CHECK(ba.status == test_result::success);
        CHECK(ba.value == 1);
        CHECK(ba.trace == test_trace().literal("b").production("test_production").literal("a"));
        auto bb = LEXY_VERIFY_P(production, "bb");
        CHECK(bb.status == test_result::success);
        CHECK(bb.value == 1);
        CHECK(bb.trace
              == test_trace()
                     .literal("b")
                     .production("test_production")
                     .literal("b")
                     .production("test_production")
                     .cancel());

        auto bba = LEXY_VERIFY_P(production, "bba");
        CHECK(bba.status == test_result::success);
        CHECK(bba.value == 2);
        CHECK(bba.trace
              == test_trace()
                     .literal("b")
                     .production("test_production")
                     .literal("b")
                     .production("test_production")
                     .literal("a"));
    }

    // No need to test other cases, code is shared with `dsl::p`.
}

