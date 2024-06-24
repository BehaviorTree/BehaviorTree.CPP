// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/repeat.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/integer.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/separator.hpp>

namespace
{
constexpr auto integer = dsl::integer<int>;

struct count_production
{
    static constexpr auto name  = "count";
    static constexpr auto rule  = integer;
    static constexpr auto value = lexy::forward<int>;
};
} // namespace

TEST_CASE("dsl::repeat()")
{
    SUBCASE("basic")
    {
        constexpr auto rule = dsl::repeat(integer)(dsl::lit_c<'a'>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

        auto zero = LEXY_VERIFY("0");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("digits", "0"));
        auto one = LEXY_VERIFY("1a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().token("digits", "1").literal("a"));
        auto two = LEXY_VERIFY("2aa");
        CHECK(two.status == test_result::success);
        CHECK(two.trace == test_trace().token("digits", "2").literal("a").literal("a"));
        auto three = LEXY_VERIFY("3aaa");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace().token("digits", "3").literal("a").literal("a").literal("a"));

        auto more = LEXY_VERIFY("2aaaaa");
        CHECK(more.status == test_result::success);
        CHECK(more.trace == test_trace().token("digits", "2").literal("a").literal("a"));
        auto fewer = LEXY_VERIFY("2a");
        CHECK(fewer.status == test_result::fatal_error);
        CHECK(
            fewer.trace
            == test_trace().token("digits", "2").literal("a").expected_literal(2, "a", 0).cancel());
    }

    SUBCASE("with separator")
    {
        constexpr auto rule = dsl::repeat(integer)(dsl::lit_c<'a'>, dsl::sep(dsl::lit_c<','>));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

        auto zero = LEXY_VERIFY("0");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("digits", "0"));
        auto one = LEXY_VERIFY("1a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().token("digits", "1").literal("a"));
        auto two = LEXY_VERIFY("2a,a");
        CHECK(two.status == test_result::success);
        CHECK(two.trace
              == test_trace().token("digits", "2").literal("a").literal(",").literal("a"));
        auto three = LEXY_VERIFY("3a,a,a");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace()
                     .token("digits", "3")
                     .literal("a")
                     .literal(",")
                     .literal("a")
                     .literal(",")
                     .literal("a"));

        auto more = LEXY_VERIFY("2a,a,a,a,a");
        CHECK(more.status == test_result::recovered_error);
        CHECK(more.trace
              == test_trace()
                     .token("digits", "2")
                     .literal("a")
                     .literal(",")
                     .literal("a")
                     .literal(",")
                     .error(4, 5, "unexpected trailing separator"));
        auto fewer = LEXY_VERIFY("2a,b");
        CHECK(fewer.status == test_result::fatal_error);
        CHECK(fewer.trace
              == test_trace()
                     .token("digits", "2")
                     .literal("a")
                     .literal(",")
                     .expected_literal(3, "a", 0)
                     .cancel());

        auto no_sep = LEXY_VERIFY("2aa");
        CHECK(no_sep.status == test_result::fatal_error);
        CHECK(
            no_sep.trace
            == test_trace().token("digits", "2").literal("a").expected_literal(2, ",", 0).cancel());
    }
    SUBCASE("with trailing separator")
    {
        constexpr auto rule
            = dsl::repeat(integer)(dsl::lit_c<'a'>, dsl::trailing_sep(dsl::lit_c<','>));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

        auto zero = LEXY_VERIFY("0");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("digits", "0"));
        auto one = LEXY_VERIFY("1a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().token("digits", "1").literal("a"));
        auto two = LEXY_VERIFY("2a,a");
        CHECK(two.status == test_result::success);
        CHECK(two.trace
              == test_trace().token("digits", "2").literal("a").literal(",").literal("a"));
        auto three = LEXY_VERIFY("3a,a,a");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace()
                     .token("digits", "3")
                     .literal("a")
                     .literal(",")
                     .literal("a")
                     .literal(",")
                     .literal("a"));

        auto more = LEXY_VERIFY("2a,a,a,a,a");
        CHECK(more.status == test_result::success);
        CHECK(more.trace
              == test_trace()
                     .token("digits", "2")
                     .literal("a")
                     .literal(",")
                     .literal("a")
                     .literal(","));
        auto fewer = LEXY_VERIFY("2a,b");
        CHECK(fewer.status == test_result::fatal_error);
        CHECK(fewer.trace
              == test_trace()
                     .token("digits", "2")
                     .literal("a")
                     .literal(",")
                     .expected_literal(3, "a", 0)
                     .cancel());

        auto no_sep = LEXY_VERIFY("2aa");
        CHECK(no_sep.status == test_result::fatal_error);
        CHECK(
            no_sep.trace
            == test_trace().token("digits", "2").literal("a").expected_literal(2, ",", 0).cancel());
    }

    SUBCASE(".list()")
    {
        constexpr auto rule = dsl::repeat(integer).list(dsl::position + dsl::lit_c<'a'>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = [](const char*, std::size_t n) { return static_cast<int>(n); };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

        auto zero = LEXY_VERIFY("0");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().token("digits", "0"));
        auto one = LEXY_VERIFY("1a");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().token("digits", "1").position().literal("a"));
        auto two = LEXY_VERIFY("2aa");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace
              == test_trace().token("digits", "2").position().literal("a").position().literal("a"));
        auto three = LEXY_VERIFY("3aaa");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace()
                     .token("digits", "3")
                     .position()
                     .literal("a")
                     .position()
                     .literal("a")
                     .position()
                     .literal("a"));

        auto more = LEXY_VERIFY("2aaaaa");
        CHECK(more.status == test_result::success);
        CHECK(more.value == 2);
        CHECK(more.trace
              == test_trace().token("digits", "2").position().literal("a").position().literal("a"));
        auto fewer = LEXY_VERIFY("2a");
        CHECK(fewer.status == test_result::fatal_error);
        CHECK(fewer.trace
              == test_trace()
                     .token("digits", "2")
                     .position()
                     .literal("a")
                     .position()
                     .expected_literal(2, "a", 0)
                     .cancel());
    }
    SUBCASE(".capture()")
    {
        constexpr auto rule = dsl::repeat(integer).capture(dsl::lit_c<'a'>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = [](const char* start, lexy::string_lexeme<> lex) {
            CHECK(start < lex.begin());
            return static_cast<int>(lex.size());
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

        auto zero = LEXY_VERIFY("0");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().token("digits", "0"));
        auto one = LEXY_VERIFY("1a");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == test_trace().token("digits", "1").literal("a"));
        auto two = LEXY_VERIFY("2aa");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == test_trace().token("digits", "2").literal("a").literal("a"));
        auto three = LEXY_VERIFY("3aaa");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace
              == test_trace().token("digits", "3").literal("a").literal("a").literal("a"));

        auto more = LEXY_VERIFY("2aaaaa");
        CHECK(more.status == test_result::success);
        CHECK(more.value == 2);
        CHECK(more.trace == test_trace().token("digits", "2").literal("a").literal("a"));
        auto fewer = LEXY_VERIFY("2a");
        CHECK(fewer.status == test_result::fatal_error);
        CHECK(
            fewer.trace
            == test_trace().token("digits", "2").literal("a").expected_literal(2, "a", 0).cancel());
    }

    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(dsl::repeat(integer)(dsl::lit_c<'a'>));
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto zero = LEXY_VERIFY("0");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().token("digits", "0"));
        auto one = LEXY_VERIFY("1a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace().token("digits", "1").literal("a"));
        auto two = LEXY_VERIFY("2aa");
        CHECK(two.status == test_result::success);
        CHECK(two.trace == test_trace().token("digits", "2").literal("a").literal("a"));
        auto three = LEXY_VERIFY("3aaa");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace().token("digits", "3").literal("a").literal("a").literal("a"));

        auto more = LEXY_VERIFY("2aaaaa");
        CHECK(more.status == test_result::success);
        CHECK(more.trace == test_trace().token("digits", "2").literal("a").literal("a"));
        auto fewer = LEXY_VERIFY("2a");
        CHECK(fewer.status == test_result::fatal_error);
        CHECK(
            fewer.trace
            == test_trace().token("digits", "2").literal("a").expected_literal(2, "a", 0).cancel());
    }
    SUBCASE("production is count")
    {
        constexpr auto rule = dsl::repeat(dsl::p<count_production>)(dsl::lit_c<'a'>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace()
                     .production("count")
                     .expected_char_class(0, "digit.decimal")
                     .cancel()
                     .cancel());

        auto zero = LEXY_VERIFY("0");
        CHECK(zero.status == test_result::success);
        CHECK(zero.trace == test_trace().production("count").token("digits", "0"));
        auto one = LEXY_VERIFY("1a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace
              == test_trace().production("count").token("digits", "1").finish().literal("a"));
        auto two = LEXY_VERIFY("2aa");
        CHECK(two.status == test_result::success);
        CHECK(two.trace
              == test_trace()
                     .production("count")
                     .token("digits", "2")
                     .finish()
                     .literal("a")
                     .literal("a"));
        auto three = LEXY_VERIFY("3aaa");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace()
                     .production("count")
                     .token("digits", "3")
                     .finish()
                     .literal("a")
                     .literal("a")
                     .literal("a"));

        auto more = LEXY_VERIFY("2aaaaa");
        CHECK(more.status == test_result::success);
        CHECK(more.trace
              == test_trace()
                     .production("count")
                     .token("digits", "2")
                     .finish()
                     .literal("a")
                     .literal("a"));
        auto fewer = LEXY_VERIFY("2a");
        CHECK(fewer.status == test_result::fatal_error);
        CHECK(fewer.trace
              == test_trace()
                     .production("count")
                     .token("digits", "2")
                     .finish()
                     .literal("a")
                     .expected_literal(2, "a", 0)
                     .cancel());
    }
}

