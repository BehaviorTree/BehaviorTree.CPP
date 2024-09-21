// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_CODE_POINT_HPP_INCLUDED
#define LEXY_DETAIL_CODE_POINT_HPP_INCLUDED

#include <lexy/input/base.hpp>

//=== encoding ===//
namespace lexy::_detail
{
template <typename Encoding>
constexpr std::size_t encode_code_point(char32_t cp, typename Encoding::char_type* buffer,
                                        std::size_t size)
{
    if constexpr (std::is_same_v<Encoding, lexy::ascii_encoding>)
    {
        LEXY_PRECONDITION(size >= 1);

        *buffer = char(cp);
        return 1;
    }
    else if constexpr (std::is_same_v<Encoding,
                                      lexy::utf8_encoding> //
                       || std::is_same_v<Encoding, lexy::utf8_char_encoding>)
    {
        using char_type = typename Encoding::char_type;
        // Taken from http://www.herongyang.com/Unicode/UTF-8-UTF-8-Encoding-Algorithm.html.
        if (cp <= 0x7F)
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char_type(cp);
            return 1;
        }
        else if (cp <= 0x07'FF)
        {
            LEXY_PRECONDITION(size >= 2);

            auto first  = (cp >> 6) & 0x1F;
            auto second = (cp >> 0) & 0x3F;

            buffer[0] = char_type(0xC0 | first);
            buffer[1] = char_type(0x80 | second);
            return 2;
        }
        else if (cp <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 3);

            auto first  = (cp >> 12) & 0x0F;
            auto second = (cp >> 6) & 0x3F;
            auto third  = (cp >> 0) & 0x3F;

            buffer[0] = char_type(0xE0 | first);
            buffer[1] = char_type(0x80 | second);
            buffer[2] = char_type(0x80 | third);
            return 3;
        }
        else
        {
            LEXY_PRECONDITION(size >= 4);

            auto first  = (cp >> 18) & 0x07;
            auto second = (cp >> 12) & 0x3F;
            auto third  = (cp >> 6) & 0x3F;
            auto fourth = (cp >> 0) & 0x3F;

            buffer[0] = char_type(0xF0 | first);
            buffer[1] = char_type(0x80 | second);
            buffer[2] = char_type(0x80 | third);
            buffer[3] = char_type(0x80 | fourth);
            return 4;
        }
    }
    else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
    {
        if (cp <= 0xFF'FF)
        {
            LEXY_PRECONDITION(size >= 1);

            buffer[0] = char16_t(cp);
            return 1;
        }
        else
        {
            // Algorithm implemented from
            // https://en.wikipedia.org/wiki/UTF-16#Code_points_from_U+010000_to_U+10FFFF.
            LEXY_PRECONDITION(size >= 2);

            auto u_prime       = cp - 0x1'0000;
            auto high_ten_bits = u_prime >> 10;
            auto low_ten_bits  = u_prime & 0b0000'0011'1111'1111;

            buffer[0] = char16_t(0xD800 + high_ten_bits);
            buffer[1] = char16_t(0xDC00 + low_ten_bits);
            return 2;
        }
    }
    else if constexpr (std::is_same_v<Encoding, lexy::utf32_encoding>)
    {
        LEXY_PRECONDITION(size >= 1);

        *buffer = char32_t(cp);
        return 1;
    }
    else
    {
        static_assert(lexy::_detail::error<Encoding>,
                      "cannot encode a code point in this encoding");
        (void)cp;
        (void)buffer;
        (void)size;
        return 0;
    }
}
} // namespace lexy::_detail

//=== parsing ===//
namespace lexy::_detail
{
enum class cp_error
{
    success,
    eof,
    leads_with_trailing,
    missing_trailing,
    surrogate,
    overlong_sequence,
    out_of_range,
};

template <typename Reader>
struct cp_result
{
    char32_t                cp;
    cp_error                error;
    typename Reader::marker end;
};

template <typename Reader>
constexpr cp_result<Reader> parse_code_point(Reader reader)
{
    if constexpr (std::is_same_v<typename Reader::encoding, lexy::ascii_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.current()};

        auto cur = reader.peek();
        reader.bump();

        auto cp = static_cast<char32_t>(cur);
        if (cp <= 0x7F)
            return {cp, cp_error::success, reader.current()};
        else
            return {cp, cp_error::out_of_range, reader.current()};
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf8_encoding> //
                       || std::is_same_v<typename Reader::encoding, lexy::utf8_char_encoding>)
    {
        using uchar_t                = unsigned char;
        constexpr auto payload_lead1 = 0b0111'1111;
        constexpr auto payload_lead2 = 0b0001'1111;
        constexpr auto payload_lead3 = 0b0000'1111;
        constexpr auto payload_lead4 = 0b0000'0111;
        constexpr auto payload_cont  = 0b0011'1111;

        constexpr auto pattern_lead1 = 0b0 << 7;
        constexpr auto pattern_lead2 = 0b110 << 5;
        constexpr auto pattern_lead3 = 0b1110 << 4;
        constexpr auto pattern_lead4 = 0b11110 << 3;
        constexpr auto pattern_cont  = 0b10 << 6;

        auto first = uchar_t(reader.peek());
        if ((first & ~payload_lead1) == pattern_lead1)
        {
            // ASCII character.
            reader.bump();
            return {first, cp_error::success, reader.current()};
        }
        else if ((first & ~payload_cont) == pattern_cont)
        {
            return {{}, cp_error::leads_with_trailing, reader.current()};
        }
        else if ((first & ~payload_lead2) == pattern_lead2)
        {
            reader.bump();

            auto second = uchar_t(reader.peek());
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto result = char32_t(first & payload_lead2);
            result <<= 6;
            result |= char32_t(second & payload_cont);

            // C0 and C1 are overlong ASCII.
            if (first == 0xC0 || first == 0xC1)
                return {result, cp_error::overlong_sequence, reader.current()};
            else
                return {result, cp_error::success, reader.current()};
        }
        else if ((first & ~payload_lead3) == pattern_lead3)
        {
            reader.bump();

            auto second = uchar_t(reader.peek());
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto third = uchar_t(reader.peek());
            if ((third & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto result = char32_t(first & payload_lead3);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);

            auto cp = result;
            if (0xD800 <= cp && cp <= 0xDFFF)
                return {cp, cp_error::surrogate, reader.current()};
            else if (first == 0xE0 && second < 0xA0)
                return {cp, cp_error::overlong_sequence, reader.current()};
            else
                return {cp, cp_error::success, reader.current()};
        }
        else if ((first & ~payload_lead4) == pattern_lead4)
        {
            reader.bump();

            auto second = uchar_t(reader.peek());
            if ((second & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto third = uchar_t(reader.peek());
            if ((third & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto fourth = uchar_t(reader.peek());
            if ((fourth & ~payload_cont) != pattern_cont)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            auto result = char32_t(first & payload_lead4);
            result <<= 6;
            result |= char32_t(second & payload_cont);
            result <<= 6;
            result |= char32_t(third & payload_cont);
            result <<= 6;
            result |= char32_t(fourth & payload_cont);

            auto cp = result;
            if (cp > 0x10'FFFF)
                return {cp, cp_error::out_of_range, reader.current()};
            else if (first == 0xF0 && second < 0x90)
                return {cp, cp_error::overlong_sequence, reader.current()};
            else
                return {cp, cp_error::success, reader.current()};
        }
        else // FE or FF
        {
            return {{}, cp_error::eof, reader.current()};
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf16_encoding>)
    {
        constexpr auto payload1 = 0b0000'0011'1111'1111;
        constexpr auto payload2 = payload1;

        constexpr auto pattern1 = 0b110110 << 10;
        constexpr auto pattern2 = 0b110111 << 10;

        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.current()};

        auto first = char16_t(reader.peek());
        if ((first & ~payload1) == pattern1)
        {
            reader.bump();
            if (reader.peek() == Reader::encoding::eof())
                return {{}, cp_error::missing_trailing, reader.current()};

            auto second = char16_t(reader.peek());
            if ((second & ~payload2) != pattern2)
                return {{}, cp_error::missing_trailing, reader.current()};
            reader.bump();

            // We've got a valid code point.
            auto result = char32_t(first & payload1);
            result <<= 10;
            result |= char32_t(second & payload2);
            result |= 0x10000;
            return {result, cp_error::success, reader.current()};
        }
        else if ((first & ~payload2) == pattern2)
        {
            return {{}, cp_error::leads_with_trailing, reader.current()};
        }
        else
        {
            // Single code unit code point; always valid.
            reader.bump();
            return {first, cp_error::success, reader.current()};
        }
    }
    else if constexpr (std::is_same_v<typename Reader::encoding, lexy::utf32_encoding>)
    {
        if (reader.peek() == Reader::encoding::eof())
            return {{}, cp_error::eof, reader.current()};

        auto cur = reader.peek();
        reader.bump();

        auto cp = cur;
        if (cp > 0x10'FFFF)
            return {cp, cp_error::out_of_range, reader.current()};
        else if (0xD800 <= cp && cp <= 0xDFFF)
            return {cp, cp_error::surrogate, reader.current()};
        else
            return {cp, cp_error::success, reader.current()};
    }
    else
    {
        static_assert(lexy::_detail::error<typename Reader::encoding>,
                      "no known code point for this encoding");
        return {};
    }
}

template <typename Reader>
constexpr void recover_code_point(Reader& reader, cp_result<Reader> result)
{
    switch (result.error)
    {
    case cp_error::success:
        // Consume the entire code point.
        reader.reset(result.end);
        break;
    case cp_error::eof:
        // We don't need to do anything to "recover" from EOF.
        break;

    case cp_error::leads_with_trailing:
        // Invalid code unit, consume to recover.
        LEXY_PRECONDITION(result.end.position() == reader.position());
        reader.bump();
        break;

    case cp_error::missing_trailing:
    case cp_error::surrogate:
    case cp_error::out_of_range:
    case cp_error::overlong_sequence:
        // Consume all the invalid code units to recover.
        reader.reset(result.end);
        break;
    }
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_CODE_POINT_HPP_INCLUDED

