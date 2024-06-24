// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/ascii.hpp>

#include "verify.hpp"
#include <cctype>
#include <lexy/dsl/identifier.hpp>

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

    for (auto c = 0; c <= 255; ++c)
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

        auto swar_utf8 = rule.template char_class_match_swar<lexy::utf8_char_encoding>(
            lexy::_detail::swar_fill(char(c)));
        if (swar_utf8)
            CHECK(pred(c));
        if (!pred(c))
            CHECK(!swar_utf8);

        auto swar_utf32 = rule.template char_class_match_swar<lexy::utf32_encoding>(
            lexy::_detail::swar_fill(char32_t(c)));
        if (swar_utf32)
            CHECK(pred(c));
        if (!pred(c))
            CHECK(!swar_utf32);

        auto swar_utf32_wrong = rule.template char_class_match_swar<lexy::utf32_encoding>(
            lexy::_detail::swar_fill(char32_t(0xFF00 | c)));
        CHECK(!swar_utf32_wrong);
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

template <typename Rule>
bool test_swar(Rule, const char* str)
{
    constexpr auto rule = lexy::dsl::identifier(Rule()).pattern();

    auto input  = lexy::buffer<lexy::utf8_char_encoding>(str, std::strlen(str));
    auto reader = input.reader();
    return lexy::try_match_token(rule, reader) && reader.peek() == lexy::utf8_char_encoding::eof();
}
} // namespace

TEST_CASE("dsl::ascii::control")
{
    constexpr auto rule = dsl::ascii::control;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.control", rule, [](int c) { return std::iscntrl(c); });

    CHECK(test_swar(rule, "\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20"));
    CHECK(!test_swar(rule, "\1\2\3\4\5\6\7\10 \12\13\14\15\16\17\20"));
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
    CHECK(test_swar(rule, "12345678901234567890"));
    CHECK(!test_swar(rule, "2134567890a1234567890"));
}

TEST_CASE("dsl::ascii::lower")
{
    constexpr auto rule = dsl::ascii::lower;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.lower", rule, [](int c) { return std::islower(c); });
    CHECK(test_swar(rule, "abcdefghijklmnopqrstuvwxyz"));
    CHECK(!test_swar(rule, "abcdefghiJklmnopqrstuvwxyz"));
}

TEST_CASE("dsl::ascii::upper")
{
    constexpr auto rule = dsl::ascii::upper;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.upper", rule, [](int c) { return std::isupper(c); });
    CHECK(test_swar(rule, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    CHECK(!test_swar(rule, "ABCDEFGHIjKLMNOPQRSTUVWXYZ"));
}

TEST_CASE("dsl::ascii::alpha")
{
    constexpr auto rule = dsl::ascii::alpha;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.alpha", rule, [](int c) { return std::isalpha(c); });
    CHECK(test_swar(rule, "abcdefghijklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmnopqrstuvwxyz"));
    CHECK(!test_swar(rule, "abcdefghiJklmno1pqrstuvwxyz"));
}

TEST_CASE("dsl::ascii::alpha_underscore")
{
    constexpr auto rule = dsl::ascii::alpha_underscore;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.alpha-underscore", rule, [](int c) { return (std::isalpha(c) != 0) || c == '_'; });
    CHECK(test_swar(rule, "abcdefghijklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmnopqrstuvwxyz"));
    CHECK(!test_swar(rule, "abcdefghiJklmno1pqrstuvwxyz"));
}

TEST_CASE("dsl::ascii::alpha_digit")
{
    constexpr auto rule = dsl::ascii::alpha_digit;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.alpha-digit", rule, [](int c) { return std::isalnum(c); });
    CHECK(test_swar(rule, "abcdefghijklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmno1pqrstuvwxyz"));
    CHECK(!test_swar(rule, "abcdefghiJklmno-pqrstuvwxyz"));
}

TEST_CASE("dsl::ascii::alnum")
{
    constexpr auto rule = dsl::ascii::alnum;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::ascii::alpha_digit));
    CHECK(test_swar(rule, "abcdefghijklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmno1pqrstuvwxyz"));
    CHECK(!test_swar(rule, "abcdefghiJklmno-pqrstuvwxyz"));
}

TEST_CASE("dsl::ascii::word")
{
    constexpr auto rule = dsl::ascii::word;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.word", rule, [](int c) { return (std::isalnum(c) != 0) || c == '_'; });
    CHECK(test_swar(rule, "abcdefghijklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmnopqrstuvwxyz"));
    CHECK(test_swar(rule, "abcdefghiJklmno1pqrstuvwxyz"));
    CHECK(!test_swar(rule, "abcdefghiJklmno-pqrstuvwxyz"));
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
    CHECK(test_swar(rule, "abcdefGHIJKLMNOpqrstuvwxyz23456789!@#^%&*(&%"));
    CHECK(!test_swar(rule, "abcdefGHIJKLMNOpqrstuvw\nxyz23456789!@#^%&*(&%"));
    CHECK(!test_swar(rule, "abcdefGHIJKLMNOpqrst uvwxyz23456789!@#^%&*(&%"));
}

TEST_CASE("dsl::ascii::print")
{
    constexpr auto rule = dsl::ascii::print;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII.print", rule, [](int c) { return std::isprint(c); });
    CHECK(test_swar(rule, "abcdefGHIJKLMNOpqrstuvwxyz23456789!@#^%&*(&%"));
    CHECK(test_swar(rule, "abcdefGHIJKLMNOpqrst uvwxyz23456789!@#^%&*(&%"));
    CHECK(!test_swar(rule, "abcdefGHIJKLMNOpqrstuvw\nxyz23456789!@#^%&*(&%"));
}

TEST_CASE("dsl::ascii::character")
{
    constexpr auto rule = dsl::ascii::character;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    test("ASCII", rule, [](int c) { return 0x00 <= c && c <= 0x7F; });
    CHECK(test_swar(rule, "abcdefGHIJKLMNOpqrstuvwxyz23456789!@#^%&*(&%"));
    CHECK(test_swar(rule, "abcdefGHIJKLMNOpqrst uvwxyz23456789!@#^%&*(&%"));
    CHECK(test_swar(rule, "abcdefGHIJKLMNOpqrstuvw\nxyz23456789!@#^%&*(&%"));
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

