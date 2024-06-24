// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/unicode.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>

namespace
{
constexpr auto callback = token_callback;

template <typename Rule, typename AsciiRule>
void test(const char* name, Rule rule, AsciiRule arule)
{
    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, name).cancel());

    for (auto c = 0; c <= 127; ++c)
    {
        const char16_t input[] = {char16_t(c), char16_t(c)};
        auto           cp      = lexy::code_point(static_cast<char32_t>(c));

        auto result  = verify(rule, lexy::string_input(input, 2), callback);
        auto aresult = verify(arule, lexy::string_input(input, 2), callback);
        if (aresult.status == test_result::success)
        {
            CHECK(result.status == test_result::success);
            CHECK(result.trace == test_trace().token(doctest::toString(cp).c_str()));
        }
        else
        {
            auto spelling = doctest::toString(cp);

            CHECK(result.status == test_result::fatal_error);
            CHECK(result.trace == test_trace().expected_char_class(0, name).cancel());
        }
    }
}
} // namespace

TEST_CASE("dsl::unicode::control")
{
    constexpr auto rule = dsl::unicode::control;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.control", rule, dsl::ascii::control);
}

TEST_CASE("dsl::unicode::blank")
{
    constexpr auto rule = dsl::unicode::blank;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.blank", rule, dsl::ascii::blank);
}

TEST_CASE("dsl::unicode::newline")
{
    constexpr auto rule = dsl::unicode::newline;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.newline", rule, dsl::ascii::newline);

    auto nel = LEXY_VERIFY(u"\u0085");
    CHECK(nel.status == test_result::success);
    CHECK(nel.trace == test_trace().token("\\u0085"));
    auto lsep = LEXY_VERIFY(u"\u2028");
    CHECK(lsep.status == test_result::success);
    CHECK(lsep.trace == test_trace().token("\\u2028"));
    auto psep = LEXY_VERIFY(u"\u2029");
    CHECK(psep.status == test_result::success);
    CHECK(psep.trace == test_trace().token("\\u2029"));
}

TEST_CASE("dsl::unicode::other_space")
{
    constexpr auto rule = dsl::unicode::other_space;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.other-space", rule, dsl::ascii::other_space);
}

TEST_CASE("dsl::unicode::space")
{
    constexpr auto rule = dsl::unicode::space;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.whitespace", rule, dsl::ascii::space);
}

TEST_CASE("dsl::unicode::digit")
{
    constexpr auto rule = dsl::unicode::digit;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.decimal-number", rule, dsl::ascii::digit);
}

TEST_CASE("dsl::unicode::lower")
{
    constexpr auto rule = dsl::unicode::lower;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.lowercase", rule, dsl::ascii::lower);
}

TEST_CASE("dsl::unicode::upper")
{
    constexpr auto rule = dsl::unicode::upper;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.uppercase", rule, dsl::ascii::upper);
}

TEST_CASE("dsl::unicode::alpha")
{
    constexpr auto rule = dsl::unicode::alpha;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.alphabetic", rule, dsl::ascii::alpha);
}

TEST_CASE("dsl::unicode::alpha_digit")
{
    constexpr auto rule = dsl::unicode::alpha_digit;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.alphabetic-decimal", rule, dsl::ascii::alpha_digit);
}

TEST_CASE("dsl::unicode::alnum")
{
    constexpr auto rule = dsl::unicode::alnum;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::unicode::alpha_digit));
}

TEST_CASE("dsl::unicode::word")
{
    constexpr auto rule = dsl::unicode::word;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.word", rule, dsl::ascii::word);

    auto join = LEXY_VERIFY(u"\u200C");
    CHECK(join.status == test_result::success);
    CHECK(join.trace == test_trace().token("\\u200C"));
}

TEST_CASE("dsl::unicode::graph")
{
    constexpr auto rule = dsl::unicode::graph;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.graph", rule, dsl::ascii::graph);
}

TEST_CASE("dsl::unicode::print")
{
    constexpr auto rule = dsl::unicode::print;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.print", rule, dsl::ascii::print);
}

TEST_CASE("dsl::unicode::character")
{
    constexpr auto rule = dsl::unicode::character;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.character", rule, dsl::ascii::character);
}

TEST_CASE("dsl::unicode::xid_start")
{
    constexpr auto rule = dsl::unicode::xid_start;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.XID-start", rule, dsl::ascii::alpha);
}

TEST_CASE("dsl::unicode::xid_start_underscore")
{
    constexpr auto rule = dsl::unicode::xid_start_underscore;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.XID-start-underscore", rule, dsl::ascii::alpha_underscore);
}

TEST_CASE("dsl::unicode::xid_continue")
{
    constexpr auto rule = dsl::unicode::xid_continue;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("code-point.XID-continue", rule, dsl::ascii::alpha_digit_underscore);
}

