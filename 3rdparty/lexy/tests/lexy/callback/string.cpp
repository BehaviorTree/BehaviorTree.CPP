// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/string.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/case_folding.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/input/string_input.hpp>
#include <string>

TEST_CASE("_detail::encode_code_point")
{
    // We test it indirectly by going through lexy::as_string for convenience.
    auto encode = [](auto encoding, lexy::code_point cp) {
        if constexpr (std::is_same_v<decltype(encoding), lexy::utf8_encoding> && !LEXY_HAS_CHAR8_T)
        {
            using string_type = std::string;
            return lexy::as_string<string_type, decltype(encoding)>(cp);
        }
        else
        {
            using char_type   = typename decltype(encoding)::char_type;
            using string_type = std::basic_string<char_type>;
            return lexy::as_string<string_type, decltype(encoding)>(cp);
        }
    };

    SUBCASE("ASCII")
    {
        // ASCII is always guaranteed to round-trip.
        for (auto c = 0u; c <= 0x7F; ++c)
        {
            auto cp = lexy::code_point(c);
            CHECK(cp.is_valid());
            CHECK(cp.is_ascii());

            CHECK(encode(lexy::ascii_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf8_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf16_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf32_encoding{}, cp)[0] == c);
        }
    }
    SUBCASE("BMP")
    {
        // BMP is guaranteed to round-trip for UTF-16 and UTF-32.
        for (auto c = 0u; c <= 0xFFFF; ++c)
        {
            auto cp = lexy::code_point(c);
            if (cp.is_surrogate())
                continue;
            CHECK(cp.is_bmp());

            CHECK(encode(lexy::utf16_encoding{}, cp)[0] == c);
            CHECK(encode(lexy::utf32_encoding{}, cp)[0] == c);
        }
    }

    SUBCASE("UTF-8 multi-unit sequences")
    {
        // ä - 2 bytes
        CHECK(encode(lexy::utf8_encoding{}, lexy::code_point(0x00E4)) == u8"\u00E4");
        // € - 3 bytes
        CHECK(encode(lexy::utf8_encoding{}, lexy::code_point(0x20AC)) == u8"\u20AC");
        // 🙂 - 4 bytes
        CHECK(encode(lexy::utf8_encoding{}, lexy::code_point(0x1'F642)) == u8"\U0001F642");
    }
    SUBCASE("UTF-16 multi-unit sequences")
    {
        // €
        CHECK(encode(lexy::utf16_encoding{}, lexy::code_point(0x20AC)) == u"\u20AC");
        // 🙂
        CHECK(encode(lexy::utf16_encoding{}, lexy::code_point(0x1'F642)) == u"\U0001F642");
    }
    SUBCASE("UTF-32 selected characters")
    {
        // €
        CHECK(encode(lexy::utf32_encoding{}, lexy::code_point(0x20AC)) == U"\u20AC");
        // 🙂
        CHECK(encode(lexy::utf32_encoding{}, lexy::code_point(0x1'F642)) == U"\U0001F642");
    }
}

TEST_CASE("as_string")
{
    auto char_lexeme = [] {
        auto input  = lexy::zstring_input("AbC");
        auto reader = input.reader();

        auto begin = reader.position();
        reader.bump();
        reader.bump();
        reader.bump();

        return lexy::lexeme(reader, begin);
    }();
    auto uchar_lexeme = [] {
        auto input  = lexy::zstring_input<lexy::byte_encoding>("AbC");
        auto reader = input.reader();

        auto begin = reader.position();
        reader.bump();
        reader.bump();
        reader.bump();

        return lexy::lexeme(reader, begin);
    }();

    SUBCASE("basic")
    {
        std::string from_nullopt = lexy::as_string<std::string>(lexy::nullopt{});
        CHECK(from_nullopt.empty());

        std::string from_rvalue = lexy::as_string<std::string>(std::string("test"));
        CHECK(from_rvalue == "test");

        std::string from_char_range
            = lexy::as_string<std::string>(char_lexeme.begin(), char_lexeme.end());
        CHECK(from_char_range == "AbC");
        std::string from_char_range_alloc
            = lexy::as_string<std::string>(std::allocator<char>{}, char_lexeme.begin(),
                                           char_lexeme.end());
        CHECK(from_char_range_alloc == "AbC");

        std::string from_char_lexeme = lexy::as_string<std::string>(char_lexeme);
        CHECK(from_char_lexeme == "AbC");
        std::string from_char_lexeme_alloc
            = lexy::as_string<std::string>(std::allocator<char>{}, char_lexeme);
        CHECK(from_char_lexeme_alloc == "AbC");

        std::string from_uchar_lexeme = lexy::as_string<std::string>(uchar_lexeme);
        CHECK(from_uchar_lexeme == "AbC");
        std::string from_uchar_lexeme_alloc
            = lexy::as_string<std::string>(std::allocator<char>{}, uchar_lexeme);
        CHECK(from_uchar_lexeme_alloc == "AbC");

        std::string from_ascii_cp
            = lexy::as_string<std::string, lexy::ascii_encoding>(lexy::code_point('a'));
        CHECK(from_ascii_cp == "a");
        std::string from_ascii_cp_alloc
            = lexy::as_string<std::string, lexy::ascii_encoding>(std::allocator<char>{},
                                                                 lexy::code_point('a'));
        CHECK(from_ascii_cp_alloc == "a");

        std::string from_unicode_cp
            = lexy::as_string<std::string, lexy::utf8_encoding>(lexy::code_point(0x00E4));
        CHECK(from_unicode_cp == "\u00E4");
        std::string from_unicode_cp_alloc
            = lexy::as_string<std::string, lexy::utf8_encoding>(std::allocator<char>{},
                                                                lexy::code_point(0x00E4));
        CHECK(from_unicode_cp == "\u00E4");

        std::string from_sink = [&] {
            auto sink = lexy::as_string<std::string, lexy::utf8_encoding>.sink();
            sink('a');
            sink(char_lexeme.begin(), char_lexeme.end());
            sink(char_lexeme);
            sink(uchar_lexeme);
            sink(std::string("hi"));
            sink(lexy::code_point('a'));
            sink(lexy::code_point(0x00E4));

            return LEXY_MOV(sink).finish();
        }();
        CHECK(from_sink == "aAbCAbCAbChia\u00E4");

        std::string from_alloc_sink = [&] {
            auto sink
                = lexy::as_string<std::string, lexy::utf8_encoding>.sink(std::allocator<int>());
            sink('a');
            sink(char_lexeme.begin(), char_lexeme.end());
            sink(char_lexeme);
            sink(uchar_lexeme);
            sink(std::string("hi"));
            sink(lexy::code_point('a'));
            sink(lexy::code_point(0x00E4));

            return LEXY_MOV(sink).finish();
        }();
        CHECK(from_alloc_sink == "aAbCAbCAbChia\u00E4");
    }
    SUBCASE("ASCII case folding")
    {
        constexpr auto callback = lexy::as_string<std::string, lexy::utf8_encoding>.case_folding(
            lexy::dsl::ascii::case_folding);

        std::string from_nullopt = callback(lexy::nullopt{});
        CHECK(from_nullopt.empty());

        std::string from_rvalue = callback(std::string("TeSt"));
        CHECK(from_rvalue == "test");

        std::string from_char_range = callback(char_lexeme.begin(), char_lexeme.end());
        CHECK(from_char_range == "abc");
        std::string from_char_range_alloc
            = callback(std::allocator<char>{}, char_lexeme.begin(), char_lexeme.end());
        CHECK(from_char_range_alloc == "abc");

        std::string from_char_lexeme = callback(char_lexeme);
        CHECK(from_char_lexeme == "abc");
        std::string from_char_lexeme_alloc = callback(std::allocator<char>{}, char_lexeme);
        CHECK(from_char_lexeme_alloc == "abc");

        std::string from_uchar_lexeme = callback(uchar_lexeme);
        CHECK(from_uchar_lexeme == "abc");
        std::string from_uchar_lexeme_alloc = callback(std::allocator<char>{}, uchar_lexeme);
        CHECK(from_uchar_lexeme_alloc == "abc");

        std::string from_cp = callback(lexy::code_point(0x00C4));
        CHECK(from_cp == "\u00C4");
        std::string from_cp_alloc = callback(std::allocator<char>{}, lexy::code_point(0x00C4));
        CHECK(from_cp == "\u00C4");

        std::string from_sink = [&] {
            auto sink = callback.sink();
            sink('a');
            sink(char_lexeme.begin(), char_lexeme.end());
            sink(char_lexeme);
            sink(uchar_lexeme);
            sink(std::string("hi"));
            sink(lexy::code_point('a'));
            sink(lexy::code_point(0x00C4));

            return LEXY_MOV(sink).finish();
        }();
        CHECK(from_sink == "aabcabcabchia\u00C4");
    }
    SUBCASE("Unicode case folding")
    {
        constexpr auto callback = lexy::as_string<std::string, lexy::utf8_encoding>.case_folding(
            lexy::dsl::unicode::simple_case_folding);

        std::string from_nullopt = callback(lexy::nullopt{});
        CHECK(from_nullopt.empty());

        std::string from_rvalue = callback(std::string("TeSt"));
        CHECK(from_rvalue == "test");

        std::string from_char_range = callback(char_lexeme.begin(), char_lexeme.end());
        CHECK(from_char_range == "abc");
        std::string from_char_range_alloc
            = callback(std::allocator<char>{}, char_lexeme.begin(), char_lexeme.end());
        CHECK(from_char_range_alloc == "abc");

        std::string from_char_lexeme = callback(char_lexeme);
        CHECK(from_char_lexeme == "abc");
        std::string from_char_lexeme_alloc = callback(std::allocator<char>{}, char_lexeme);
        CHECK(from_char_lexeme_alloc == "abc");

        std::string from_uchar_lexeme = callback(uchar_lexeme);
        CHECK(from_uchar_lexeme == "abc");
        std::string from_uchar_lexeme_alloc = callback(std::allocator<char>{}, uchar_lexeme);
        CHECK(from_uchar_lexeme_alloc == "abc");

        std::string from_cp = callback(lexy::code_point(0x00C4));
        CHECK(from_cp == "\u00E4");
        std::string from_cp_alloc = callback(std::allocator<char>{}, lexy::code_point(0x00C4));
        CHECK(from_cp == "\u00E4");

        std::string from_sink = [&] {
            auto sink = callback.sink();
            sink('a');
            sink(char_lexeme.begin(), char_lexeme.end());
            sink(char_lexeme);
            sink(uchar_lexeme);
            sink(std::string("hi"));
            sink(lexy::code_point('a'));
            sink(lexy::code_point(0x00C4));

            return LEXY_MOV(sink).finish();
        }();
        CHECK(from_sink == "aabcabcabchia\u00E4");
    }
}

