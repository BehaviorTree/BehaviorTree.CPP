// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/ascii.hpp>

#include "verify.hpp"
#include <cctype>

namespace
{
constexpr auto callback = token_callback;

template <typename Rule, typename Predicate>
void test(const char* name, Rule rule, Predicate pred)
{
    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, name).cancel());

    auto non_ascii = LEXY_VERIFY("\x80");
    CHECK(non_ascii.status == test_result::fatal_error);
    CHECK(non_ascii.trace == test_trace().expected_char_class(0, name).cancel());

    for (auto c = 0; c <= 127; ++c)
    {
        const char input[] = {char(c), char(c)};
        auto       cp      = lexy::code_point(static_cast<char32_t>(c));
        INFO(cp);

        auto result = verify(rule, lexy::string_input(input, 2), callback);
        if (pred(c))
        {
            CHECK(result.status == test_result::success);
            CHECK(result.trace == test_trace().token(doctest::toString(cp).c_str()));
        }
        else
        {
            CHECK(result.status == test_result::fatal_error);
            CHECK(result.trace == test_trace().expected_char_class(0, name).cancel());
        }
    }

    auto utf16 = LEXY_VERIFY(u"A");
    if (pred('A'))
    {
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().token("A"));
    }
    else
    {
        CHECK(utf16.status == test_result::fatal_error);
        CHECK(utf16.trace == test_trace().expected_char_class(0, name).cancel());
    }
}
} // namespace

TEST_CASE("dsl::ascii::control")
{
    constexpr auto rule = dsl::ascii::control;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.control", rule, [](int c) { return std::iscntrl(c); });
}

TEST_CASE("dsl::ascii::blank")
{
    constexpr auto rule = dsl::ascii::blank;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.blank", rule, [](int c) { return std::isblank(c); });
}

TEST_CASE("dsl::ascii::newline")
{
    constexpr auto rule = dsl::ascii::newline;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.newline", rule, [](int c) { return c == '\n' || c == '\r'; });
}

TEST_CASE("dsl::ascii::other_space")
{
    constexpr auto rule = dsl::ascii::other_space;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.other-space", rule, [](int c) { return c == '\v' || c == '\f'; });
}

TEST_CASE("dsl::ascii::space")
{
    constexpr auto rule = dsl::ascii::space;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.space", rule, [](int c) { return std::isspace(c); });
}

TEST_CASE("dsl::ascii::digit")
{
    constexpr auto rule = dsl::ascii::digit;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.digit", rule, [](int c) { return std::isdigit(c); });
}

TEST_CASE("dsl::ascii::lower")
{
    constexpr auto rule = dsl::ascii::lower;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.lower", rule, [](int c) { return std::islower(c); });
}

TEST_CASE("dsl::ascii::upper")
{
    constexpr auto rule = dsl::ascii::upper;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.upper", rule, [](int c) { return std::isupper(c); });
}

TEST_CASE("dsl::ascii::alpha")
{
    constexpr auto rule = dsl::ascii::alpha;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.alpha", rule, [](int c) { return std::isalpha(c); });
}

TEST_CASE("dsl::ascii::alpha_underscore")
{
    constexpr auto rule = dsl::ascii::alpha_underscore;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.alpha-underscore", rule, [](int c) { return (std::isalpha(c) != 0) || c == '_'; });
}

TEST_CASE("dsl::ascii::alpha_digit")
{
    constexpr auto rule = dsl::ascii::alpha_digit;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.alpha-digit", rule, [](int c) { return std::isalnum(c); });
}

TEST_CASE("dsl::ascii::alnum")
{
    constexpr auto rule = dsl::ascii::alnum;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::ascii::alpha_digit));
}

TEST_CASE("dsl::ascii::word")
{
    constexpr auto rule = dsl::ascii::word;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.word", rule, [](int c) { return (std::isalnum(c) != 0) || c == '_'; });
}

TEST_CASE("dsl::ascii::alpha_digit_underscore")
{
    constexpr auto rule = dsl::ascii::alpha_digit_underscore;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::ascii::word));
}

TEST_CASE("dsl::ascii::punct")
{
    constexpr auto rule = dsl::ascii::punct;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.punct", rule, [](int c) { return std::ispunct(c); });
}

TEST_CASE("dsl::ascii::graph")
{
    constexpr auto rule = dsl::ascii::graph;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.graph", rule, [](int c) { return std::isgraph(c); });
}

TEST_CASE("dsl::ascii::print")
{
    constexpr auto rule = dsl::ascii::print;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.print", rule, [](int c) { return std::isprint(c); });
}

TEST_CASE("dsl::ascii::character")
{
    constexpr auto rule = dsl::ascii::character;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII", rule, [](int c) { return 0x00 <= c && c <= 0x7F; });
}

TEST_CASE("dsl::ascii::one_of")
{
    constexpr auto rule = LEXY_ASCII_ONE_OF("abc");
    CHECK(lexy::is_token_rule<decltype(rule)>);

#if LEXY_HAS_NTTP
    CHECK(equivalent_rules(rule, dsl::ascii::one_of<"abc">));
#endif

    test("abc", rule, [](int c) { return c == 'a' || c == 'b' || c == 'c'; });
}

