// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/encoding.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/string.hpp>
#include <lexy/input/string_input.hpp>
#include <string>

TEST_CASE("encoding deduction")
{
    auto char_input = lexy::zstring_input("str");
    CHECK(std::is_same_v<decltype(char_input)::encoding, lexy::default_encoding>);

#if LEXY_HAS_CHAR8_T
    auto char8_input = lexy::zstring_input(u8"str");
    CHECK(std::is_same_v<decltype(char8_input)::encoding, lexy::utf8_encoding>);
#endif
    auto char16_input = lexy::zstring_input(u"str");
    CHECK(std::is_same_v<decltype(char16_input)::encoding, lexy::utf16_encoding>);
    auto char32_input = lexy::zstring_input(U"str");
    CHECK(std::is_same_v<decltype(char32_input)::encoding, lexy::utf32_encoding>);

    constexpr unsigned char uchar_str[] = {0};
    auto                    uchar_input = lexy::zstring_input(uchar_str);
    CHECK(std::is_same_v<decltype(uchar_input)::encoding, lexy::byte_encoding>);
    CHECK(uchar_input.data() == uchar_str);

    constexpr std::byte byte_str[] = {std::byte(0)};
    auto                byte_input = lexy::zstring_input(byte_str);
    CHECK(std::is_same_v<decltype(byte_input)::encoding, lexy::byte_encoding>);
    CHECK(byte_input.data() == reinterpret_cast<const unsigned char*>(byte_str));
}

TEST_CASE("wchar_t encoding")
{
    if constexpr (sizeof(wchar_t) == sizeof(char16_t))
        CHECK(lexy::utf16_encoding::is_secondary_char_type<wchar_t>());
    else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
        CHECK(lexy::utf32_encoding::is_secondary_char_type<wchar_t>());
}

