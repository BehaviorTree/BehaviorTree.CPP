// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/code_point.hpp>

#include "verify.hpp"

using lexy::_detail::cp_error;

namespace
{
struct parse_result
{
    std::size_t count;
    cp_error    ec;
    char32_t    value;

    constexpr explicit operator bool() const
    {
        return ec == cp_error::success;
    }
};

template <typename Encoding>
constexpr parse_result parse_cp(const typename Encoding::char_type* str)
{
    auto input  = lexy::zstring_input<Encoding>(str);
    auto reader = input.reader();

    auto result = lexy::_detail::parse_code_point(reader);

    if (result.error != cp_error::success)
    {
        lexy::_detail::recover_code_point(reader, result);
        return {std::size_t(reader.position() - input.data()), result.error, result.cp};
    }
    else
    {
        return {std::size_t(result.end - input.data()), result.error, result.cp};
    }
}
} // namespace

TEST_CASE("ASCII code point parsing")
{
    auto parse = [](auto str) { return parse_cp<lexy::ascii_encoding>(str); };

    SUBCASE("basic")
    {
        constexpr auto empty = parse("");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == cp_error::eof);

        constexpr auto a = parse("a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value == 'a');

        constexpr auto out_of_range = parse("\x90");
        CHECK(!out_of_range);
        CHECK(out_of_range.count == 1);
        CHECK(out_of_range.ec == cp_error::out_of_range);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const char str[]  = {char(i), char(i), char(i), '\0'};
            auto       result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value == i);
        }
    }
    SUBCASE("non ASCII")
    {
        for (auto i = 0x80; i < 0xFF; ++i)
        {
            INFO(i);

            const char str[]  = {char(i), char(i), char(i), '\0'};
            auto       result = parse(str);
            CHECK(!result);
            CHECK(result.count == 1);
            CHECK(result.ec == cp_error::out_of_range);
        }
    }
}

TEST_CASE("UTF-8 code point parsing")
{
    auto parse     = [](auto str) { return parse_cp<lexy::utf8_encoding>(str); };
    auto parse_seq = [](auto... c) {
        LEXY_CHAR8_T str[] = {LEXY_CHAR8_T(c)..., 0x0};
        return parse_cp<lexy::utf8_encoding>(str);
    };

    SUBCASE("basic")
    {
        constexpr auto empty = parse(LEXY_CHAR8_STR(""));
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == cp_error::eof);

        constexpr auto a = parse(LEXY_CHAR8_STR("a"));
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value == 'a');
        constexpr auto umlaut = parse(LEXY_CHAR8_STR("ä"));
        CHECK(umlaut);
        CHECK(umlaut.count == 2);
        CHECK(umlaut.value == 0xE4);
        constexpr auto euro = parse(LEXY_CHAR8_STR("€"));
        CHECK(euro);
        CHECK(euro.count == 3);
        CHECK(euro.value == 0x20AC);
        constexpr auto emojii = parse(LEXY_CHAR8_STR("🙂"));
        CHECK(emojii);
        CHECK(emojii.count == 4);
        CHECK(emojii.value == 0x1F642);

        constexpr auto leads_with_trailing = parse_seq(0b1000'0001);
        CHECK(!leads_with_trailing);
        CHECK(leads_with_trailing.count == 1);
        CHECK(leads_with_trailing.ec == cp_error::leads_with_trailing);

        constexpr auto missing_first1 = parse_seq(0b1101'0000);
        CHECK(!missing_first1);
        CHECK(missing_first1.count == 1);
        CHECK(missing_first1.ec == cp_error::missing_trailing);
        constexpr auto missing_first2 = parse_seq(0b1110'1000);
        CHECK(!missing_first2);
        CHECK(missing_first2.count == 1);
        CHECK(missing_first2.ec == cp_error::missing_trailing);
        constexpr auto missing_first3 = parse_seq(0b1111'0100);
        CHECK(!missing_first3);
        CHECK(missing_first3.count == 1);
        CHECK(missing_first3.ec == cp_error::missing_trailing);
        constexpr auto missing_second2 = parse_seq(0b1110'1000, 0b1000'0001);
        CHECK(!missing_second2);
        CHECK(missing_second2.count == 2);
        CHECK(missing_second2.ec == cp_error::missing_trailing);
        constexpr auto missing_second3 = parse_seq(0b1111'0100, 0b1000'0001);
        CHECK(!missing_second3);
        CHECK(missing_second3.count == 2);
        CHECK(missing_second3.ec == cp_error::missing_trailing);
        constexpr auto missing_third3 = parse_seq(0b1111'0100, 0b1000'0001, 0b1000'0001);
        CHECK(!missing_third3);
        CHECK(missing_third3.count == 3);
        CHECK(missing_third3.ec == cp_error::missing_trailing);

        constexpr auto invalid_first1 = parse_seq(0b1101'0000, 0b1111);
        CHECK(!invalid_first1);
        CHECK(invalid_first1.count == 1);
        CHECK(invalid_first1.ec == cp_error::missing_trailing);
        constexpr auto invalid_first2 = parse_seq(0b1110'1000, 0b1111);
        CHECK(!invalid_first2);
        CHECK(invalid_first2.count == 1);
        CHECK(invalid_first2.ec == cp_error::missing_trailing);
        constexpr auto invalid_first3 = parse_seq(0b1111'0100, 0b1111);
        CHECK(!invalid_first3);
        CHECK(invalid_first3.count == 1);
        CHECK(invalid_first3.ec == cp_error::missing_trailing);
        constexpr auto invalid_second2 = parse_seq(0b1110'1000, 0b1000'0001, 0b1111);
        CHECK(!invalid_second2);
        CHECK(invalid_second2.count == 2);
        CHECK(invalid_second2.ec == cp_error::missing_trailing);
        constexpr auto invalid_second3 = parse_seq(0b1111'0100, 0b1000'0001, 0b1111);
        CHECK(!invalid_second3);
        CHECK(invalid_second3.count == 2);
        CHECK(invalid_second3.ec == cp_error::missing_trailing);
        constexpr auto invalid_third3 = parse_seq(0b1111'0100, 0b1000'0001, 0b1000'0001, 0b1111);
        CHECK(!invalid_third3);
        CHECK(invalid_third3.count == 3);
        CHECK(invalid_third3.ec == cp_error::missing_trailing);

        constexpr auto surrogate = parse_seq(0b1110'1101, 0b1011'1111, 0b1011'1111);
        CHECK(!surrogate);
        CHECK(surrogate.count == 3);
        CHECK(surrogate.ec == cp_error::surrogate);
        constexpr auto out_of_range = parse_seq(0b1111'0111, 0b1011'1111, 0b1011'1111, 0b1011'1111);
        CHECK(!out_of_range);
        CHECK(out_of_range.count == 4);
        CHECK(out_of_range.ec == cp_error::out_of_range);

        constexpr auto overlong_two1 = parse_seq(0xC0, 0x84);
        CHECK(!overlong_two1);
        CHECK(overlong_two1.count == 2);
        CHECK(overlong_two1.ec == cp_error::overlong_sequence);
        constexpr auto overlong_two2 = parse_seq(0xC1, 0x84);
        CHECK(!overlong_two2);
        CHECK(overlong_two2.count == 2);
        CHECK(overlong_two2.ec == cp_error::overlong_sequence);
        constexpr auto overlong_three = parse_seq(0xE0, 0x80, 0x80);
        CHECK(!overlong_three);
        CHECK(overlong_three.count == 3);
        CHECK(overlong_three.ec == cp_error::overlong_sequence);
        constexpr auto overlong_four = parse_seq(0xF0, 0x80, 0x80, 0x80);
        CHECK(!overlong_four);
        CHECK(overlong_four.count == 4);
        CHECK(overlong_four.ec == cp_error::overlong_sequence);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const LEXY_CHAR8_T str[]  = {LEXY_CHAR8_T(i), LEXY_CHAR8_T(i), LEXY_CHAR8_T(i), '\0'};
            auto               result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value == i);
        }
    }
}

TEST_CASE("UTF-16 code point parsing")
{
    auto parse = [](auto str) { return parse_cp<lexy::utf16_encoding>(str); };

    SUBCASE("basic")
    {
        constexpr auto empty = parse(u"");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == cp_error::eof);

        constexpr auto a = parse(u"a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value == 'a');
        constexpr auto umlaut = parse(u"ä");
        CHECK(umlaut);
        CHECK(umlaut.count == 1);
        CHECK(umlaut.value == 0xE4);
        constexpr auto euro = parse(u"€");
        CHECK(euro);
        CHECK(euro.count == 1);
        CHECK(euro.value == 0x20AC);
        constexpr auto emojii = parse(u"🙂");
        CHECK(emojii);
        CHECK(emojii.count == 2);
        CHECK(emojii.value == 0x1F642);

        constexpr char16_t leads_with_trailing_str[] = {0xDC44, 0x0};
        constexpr auto     leads_with_trailing       = parse(leads_with_trailing_str);
        CHECK(!leads_with_trailing);
        CHECK(leads_with_trailing.count == 1);
        CHECK(leads_with_trailing.ec == cp_error::leads_with_trailing);

        constexpr char16_t missing_trailing_str[] = {0xDA44, 0x0};
        constexpr auto     missing_trailing       = parse(missing_trailing_str);
        CHECK(!missing_trailing);
        CHECK(missing_trailing.count == 1);
        CHECK(missing_trailing.ec == cp_error::missing_trailing);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const char16_t str[]  = {char16_t(i), char16_t(i), char16_t(i), '\0'};
            auto           result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value == i);
        }
    }
    SUBCASE("BMP")
    {
        for (auto i = 0x80; i <= 0xFFFF; ++i)
        {
            INFO(i);
            auto cp = lexy::code_point(char32_t(i));

            const char16_t str[]  = {char16_t(i), char16_t(i), char16_t(i), '\0'};
            auto           result = parse(str);
            if (cp.is_surrogate())
            {
                CHECK(!result);
                if (i < 0xDC00)
                {
                    CHECK(result.count == 1);
                    CHECK(result.ec == cp_error::missing_trailing);
                }
                else
                {
                    CHECK(result.count == 1);
                    CHECK(result.ec == cp_error::leads_with_trailing);
                }
            }
            else
            {
                CHECK(result);
                CHECK(result.count == 1);
                CHECK(result.value == i);
            }
        }
    }
}

TEST_CASE("UTF-32 code point parsing")
{
    auto parse = [](auto str) { return parse_cp<lexy::utf32_encoding>(str); };

    SUBCASE("basic")
    {
        constexpr auto empty = parse(U"");
        CHECK(!empty);
        CHECK(empty.count == 0);
        CHECK(empty.ec == cp_error::eof);

        constexpr auto a = parse(U"a");
        CHECK(a);
        CHECK(a.count == 1);
        CHECK(a.value == 'a');
        constexpr auto umlaut = parse(U"ä");
        CHECK(umlaut);
        CHECK(umlaut.count == 1);
        CHECK(umlaut.value == 0xE4);
        constexpr auto euro = parse(U"€");
        CHECK(euro);
        CHECK(euro.count == 1);
        CHECK(euro.value == 0x20AC);
        constexpr auto emojii = parse(U"🙂");
        CHECK(emojii);
        CHECK(emojii.count == 1);
        CHECK(emojii.value == 0x1F642);

        constexpr char32_t surrogate_str[] = {0xD844, 0x0};
        constexpr auto     surrogate       = parse(surrogate_str);
        CHECK(!surrogate);
        CHECK(surrogate.count == 1);
        CHECK(surrogate.ec == cp_error::surrogate);

        constexpr char32_t out_of_range_str[] = {0xFF1234, 0x0};
        constexpr auto     out_of_range       = parse(out_of_range_str);
        CHECK(!out_of_range);
        CHECK(out_of_range.count == 1);
        CHECK(out_of_range.ec == cp_error::out_of_range);
    }
    SUBCASE("ASCII")
    {
        for (auto i = 0x01; i <= 0x7F; ++i)
        {
            INFO(i);

            const char32_t str[]  = {char32_t(i), char32_t(i), char32_t(i), '\0'};
            auto           result = parse(str);
            CHECK(result);
            CHECK(result.count == 1);
            CHECK(result.value == i);
        }
    }
    SUBCASE("BMP")
    {
        for (auto i = 0x80; i <= 0xFFFF; ++i)
        {
            INFO(i);
            auto cp = lexy::code_point(char32_t(i));

            const char32_t str[]  = {char32_t(i), char32_t(i), char32_t(i), '\0'};
            auto           result = parse(str);
            if (cp.is_surrogate())
            {
                CHECK(!result);
                CHECK(result.count == 1);
                CHECK(result.ec == cp_error::surrogate);
            }
            else
            {
                CHECK(result);
                CHECK(result.count == 1);
                CHECK(result.value == i);
            }
        }
    }
}

TEST_CASE("dsl::code_point")
{
    // Only basic sanity checks needed, the actual parsing code is tested extensively above.
    constexpr auto rule = lexy::dsl::code_point;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "code-point").cancel());

    auto ascii = LEXY_VERIFY(u"a");
    CHECK(ascii.status == test_result::success);
    CHECK(ascii.trace == test_trace().token("any", "a"));

    auto bmp = LEXY_VERIFY(u"ä");
    CHECK(bmp.status == test_result::success);
    CHECK(bmp.trace == test_trace().token("any", "\\u00E4"));

    auto emoji = LEXY_VERIFY(u"🙂");
    CHECK(emoji.status == test_result::success);
    CHECK(emoji.trace == test_trace().token("any", "\\U0001F642"));
}

TEST_CASE("dsl::code_point.if_()")
{
    struct predicate
    {
        static constexpr auto name()
        {
            return "predicate";
        }

        constexpr bool operator()(lexy::code_point cp) const
        {
            return cp.is_ascii();
        }
    };

    constexpr auto rule = lexy::dsl::code_point.if_<predicate>();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "predicate").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a"));

    auto ab = LEXY_VERIFY(u"a");
    CHECK(ab.status == test_result::success);
    CHECK(ab.trace == test_trace().token("a"));

    auto bmp = LEXY_VERIFY(u"ä");
    CHECK(bmp.status == test_result::fatal_error);
    CHECK(bmp.trace == test_trace().expected_char_class(0, "predicate").cancel());
}

TEST_CASE("dsl::code_point.ascii()")
{
    constexpr auto rule = lexy::dsl::code_point.ascii();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "code-point.ASCII").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a"));

    auto ab = LEXY_VERIFY(u"a");
    CHECK(ab.status == test_result::success);
    CHECK(ab.trace == test_trace().token("a"));

    auto bmp = LEXY_VERIFY(u"ä");
    CHECK(bmp.status == test_result::fatal_error);
    CHECK(bmp.trace == test_trace().expected_char_class(0, "code-point.ASCII").cancel());
    auto outside_bmp = LEXY_VERIFY(u"🙂");
    CHECK(outside_bmp.status == test_result::fatal_error);
    CHECK(outside_bmp.trace == test_trace().expected_char_class(0, "code-point.ASCII").cancel());
}

TEST_CASE("dsl::code_point.bmp()")
{
    constexpr auto rule = lexy::dsl::code_point.bmp();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "code-point.BMP").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a"));

    auto ab = LEXY_VERIFY(u"a");
    CHECK(ab.status == test_result::success);
    CHECK(ab.trace == test_trace().token("a"));

    auto bmp = LEXY_VERIFY(u"ä");
    CHECK(bmp.status == test_result::success);
    CHECK(bmp.trace == test_trace().token("\\u00E4"));

    auto outside_bmp = LEXY_VERIFY(u"🙂");
    CHECK(outside_bmp.status == test_result::fatal_error);
    CHECK(outside_bmp.trace == test_trace().expected_char_class(0, "code-point.BMP").cancel());
}

TEST_CASE("dsl::code_point.noncharacter()")
{
    constexpr auto rule = lexy::dsl::code_point.noncharacter();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "code-point.non-character").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == test_trace().expected_char_class(0, "code-point.non-character").cancel());
    auto bmp = LEXY_VERIFY(u"ä");
    CHECK(bmp.status == test_result::fatal_error);
    CHECK(bmp.trace == test_trace().expected_char_class(0, "code-point.non-character").cancel());
    auto outside_bmp = LEXY_VERIFY(u"🙂");
    CHECK(outside_bmp.status == test_result::fatal_error);
    CHECK(outside_bmp.trace
          == test_trace().expected_char_class(0, "code-point.non-character").cancel());

    auto noncharacter = LEXY_VERIFY(u"\uFDDF");
    CHECK(noncharacter.status == test_result::success);
    CHECK(noncharacter.trace == test_trace().token("\\uFDDF"));
}

TEST_CASE("dsl::code_point.general_category()")
{
    constexpr auto rule = lexy::dsl::code_point.general_category<lexy::code_point::Ll>();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace
          == test_trace().expected_char_class(0, "code-point.lowercase-letter").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a"));
    auto b = LEXY_VERIFY(u"b");
    CHECK(b.status == test_result::success);
    CHECK(b.trace == test_trace().token("b"));
    auto c = LEXY_VERIFY(u"c");
    CHECK(c.status == test_result::success);
    CHECK(c.trace == test_trace().token("c"));

    auto umlaut = LEXY_VERIFY(u"ä");
    CHECK(umlaut.status == test_result::success);
    CHECK(umlaut.trace == test_trace().token("\\u00E4"));
    auto cyrillic = LEXY_VERIFY(u"ҁ");
    CHECK(cyrillic.status == test_result::success);
    CHECK(cyrillic.trace == test_trace().token("\\u0481"));
    auto greek = LEXY_VERIFY(u"φ");
    CHECK(greek.status == test_result::success);
    CHECK(greek.trace == test_trace().token("\\u03C6"));
    auto math = LEXY_VERIFY(u"𝐚");
    CHECK(math.status == test_result::success);
    CHECK(math.trace == test_trace().token("\\U0001D41A"));

    auto A = LEXY_VERIFY(u"A");
    CHECK(A.status == test_result::fatal_error);
    CHECK(A.trace == test_trace().expected_char_class(0, "code-point.lowercase-letter").cancel());
    auto Umlaut = LEXY_VERIFY(u"Ä");
    CHECK(Umlaut.status == test_result::fatal_error);
    CHECK(Umlaut.trace
          == test_trace().expected_char_class(0, "code-point.lowercase-letter").cancel());
    auto Cyrillic = LEXY_VERIFY(u"Ҁ");
    CHECK(Cyrillic.status == test_result::fatal_error);
    CHECK(Cyrillic.trace
          == test_trace().expected_char_class(0, "code-point.lowercase-letter").cancel());
    auto Greek = LEXY_VERIFY(u"Φ");
    CHECK(Greek.status == test_result::fatal_error);
    CHECK(Greek.trace
          == test_trace().expected_char_class(0, "code-point.lowercase-letter").cancel());
    auto Math = LEXY_VERIFY(u"𝐀");
    CHECK(Math.status == test_result::fatal_error);
    CHECK(Math.trace
          == test_trace().expected_char_class(0, "code-point.lowercase-letter").cancel());

    auto ab = LEXY_VERIFY(u"a");
    CHECK(ab.status == test_result::success);
    CHECK(ab.trace == test_trace().token("a"));
}

TEST_CASE("dsl::code_point.general_category() group")
{
    constexpr auto rule = lexy::dsl::code_point.general_category<lexy::code_point::L>();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "code-point.letter").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a"));
    auto b = LEXY_VERIFY(u"b");
    CHECK(b.status == test_result::success);
    CHECK(b.trace == test_trace().token("b"));
    auto c = LEXY_VERIFY(u"c");
    CHECK(c.status == test_result::success);
    CHECK(c.trace == test_trace().token("c"));

    auto umlaut = LEXY_VERIFY(u"ä");
    CHECK(umlaut.status == test_result::success);
    CHECK(umlaut.trace == test_trace().token("\\u00E4"));
    auto cyrillic = LEXY_VERIFY(u"ҁ");
    CHECK(cyrillic.status == test_result::success);
    CHECK(cyrillic.trace == test_trace().token("\\u0481"));
    auto greek = LEXY_VERIFY(u"φ");
    CHECK(greek.status == test_result::success);
    CHECK(greek.trace == test_trace().token("\\u03C6"));
    auto math = LEXY_VERIFY(u"𝐚");
    CHECK(math.status == test_result::success);
    CHECK(math.trace == test_trace().token("\\U0001D41A"));

    auto A = LEXY_VERIFY(u"A");
    CHECK(A.status == test_result::success);
    CHECK(A.trace == test_trace().token("A"));
    auto Umlaut = LEXY_VERIFY(u"Ä");
    CHECK(Umlaut.status == test_result::success);
    CHECK(Umlaut.trace == test_trace().token("\\u00C4"));
    auto Cyrillic = LEXY_VERIFY(u"Ҁ");
    CHECK(Cyrillic.status == test_result::success);
    CHECK(Cyrillic.trace == test_trace().token("\\u0480"));
    auto Greek = LEXY_VERIFY(u"Φ");
    CHECK(Greek.status == test_result::success);
    CHECK(Greek.trace == test_trace().token("\\u03A6"));
    auto Math = LEXY_VERIFY(u"𝐀");
    CHECK(Math.status == test_result::success);
    CHECK(Math.trace == test_trace().token("\\U0001D400"));

    auto digit = LEXY_VERIFY(u"1");
    CHECK(digit.status == test_result::fatal_error);
    CHECK(digit.trace == test_trace().expected_char_class(0, "code-point.letter").cancel());

    auto ab = LEXY_VERIFY(u"a");
    CHECK(ab.status == test_result::success);
    CHECK(ab.trace == test_trace().token("a"));
}

TEST_CASE("dsl::code_point.range()")
{
    constexpr auto rule = lexy::dsl::code_point.range<'a', 'c'>();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "code-point.range").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a"));
    auto b = LEXY_VERIFY(u"b");
    CHECK(b.status == test_result::success);
    CHECK(b.trace == test_trace().token("b"));
    auto c = LEXY_VERIFY(u"c");
    CHECK(c.status == test_result::success);
    CHECK(c.trace == test_trace().token("c"));

    auto d = LEXY_VERIFY(u"d");
    CHECK(d.status == test_result::fatal_error);
    CHECK(d.trace == test_trace().expected_char_class(0, "code-point.range").cancel());

    auto ab = LEXY_VERIFY(u"a");
    CHECK(ab.status == test_result::success);
    CHECK(ab.trace == test_trace().token("a"));
}

TEST_CASE("dsl::code_point.set()")
{
    constexpr auto rule = lexy::dsl::code_point.set<'a', 'b', 'c'>();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(u"");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "code-point.set").cancel());

    auto a = LEXY_VERIFY(u"a");
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a"));
    auto b = LEXY_VERIFY(u"b");
    CHECK(b.status == test_result::success);
    CHECK(b.trace == test_trace().token("b"));
    auto c = LEXY_VERIFY(u"c");
    CHECK(c.status == test_result::success);
    CHECK(c.trace == test_trace().token("c"));

    auto d = LEXY_VERIFY(u"d");
    CHECK(d.status == test_result::fatal_error);
    CHECK(d.trace == test_trace().expected_char_class(0, "code-point.set").cancel());

    auto ab = LEXY_VERIFY(u"a");
    CHECK(ab.status == test_result::success);
    CHECK(ab.trace == test_trace().token("a"));
}

