// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ENCODING_HPP_INCLUDED
#define LEXY_ENCODING_HPP_INCLUDED

#include <cstdint>
#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>

//=== encoding classes ===//
namespace lexy
{
/// The endianness used by an encoding.
enum class encoding_endianness
{
    /// Little endian.
    little,
    /// Big endian.
    big,
    /// Checks for a BOM and uses its endianness.
    /// If there is no BOM, assumes big endian.
    bom,
};

/// An encoding where the input is some 8bit encoding (ASCII, UTF-8, extended ASCII etc.).
struct default_encoding
{
    using char_type = char;
    using int_type  = int;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return false;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        return -1;
    }

    static constexpr int_type to_int_type(char_type c)
    {
        if constexpr (std::is_unsigned_v<char_type>)
            // We can just convert it to int directly.
            return static_cast<int_type>(c);
        else
        {
            // We first need to prevent negative values, by making it unsigned.
            auto value = static_cast<unsigned char>(c);
            return static_cast<int_type>(value);
        }
    }
};

// An encoding where the input is assumed to be valid ASCII.
struct ascii_encoding
{
    using char_type = char;
    using int_type  = char;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return false;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        if constexpr (std::is_signed_v<char_type>)
            return int_type(-1);
        else
            return int_type(0xFFu);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};

/// An encoding where the input is assumed to be valid UTF-8.
struct utf8_encoding
{
    using char_type = LEXY_CHAR8_T;
    using int_type  = LEXY_CHAR8_T;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return std::is_same_v<OtherCharType, char>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        // 0xFF is not part of valid UTF-8.
        return int_type(0xFF);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};

/// An encoding where the input is assumed to be valid UTF-8, but the char type is char.
struct utf8_char_encoding
{
    using char_type = char;
    using int_type  = char;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return std::is_same_v<OtherCharType, LEXY_CHAR8_T>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        // 0xFF is not part of valid UTF-8.
        return int_type(0xFF);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};

/// An encoding where the input is assumed to be valid UTF-16.
struct utf16_encoding
{
    using char_type = char16_t;
    using int_type  = std::int_least32_t;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return sizeof(wchar_t) == sizeof(char16_t) && std::is_same_v<OtherCharType, wchar_t>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        // Every value of char16_t is valid UTF16.
        return int_type(-1);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};

/// An encoding where the input is assumed to be valid UTF-32.
struct utf32_encoding
{
    using char_type = char32_t;
    using int_type  = char32_t;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return sizeof(wchar_t) == sizeof(char32_t) && std::is_same_v<OtherCharType, wchar_t>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        // The highest unicode code point is U+10'FFFF, so this is never a valid code point.
        return int_type(0xFFFF'FFFF);
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return c;
    }
};

/// An encoding where the input is just raw bytes, not characters.
struct byte_encoding
{
    using char_type = unsigned char;
    using int_type  = int;

    template <typename OtherCharType>
    static constexpr bool is_secondary_char_type()
    {
        return std::is_same_v<OtherCharType, char> || std::is_same_v<OtherCharType, std::byte>;
    }

    static LEXY_CONSTEVAL int_type eof()
    {
        return -1;
    }

    static constexpr int_type to_int_type(char_type c)
    {
        return int_type(c);
    }
};
} // namespace lexy

//=== deduce_encoding ===//
namespace lexy
{
template <typename CharT>
struct _deduce_encoding;
template <typename CharT>
using deduce_encoding = typename _deduce_encoding<CharT>::type;

template <>
struct _deduce_encoding<char>
{
#if defined(LEXY_ENCODING_OF_CHAR)
    using type = LEXY_ENCODING_OF_CHAR;
    static_assert(std::is_same_v<type, default_encoding>      //
                      || std::is_same_v<type, ascii_encoding> //
                      || std::is_same_v<type, utf8_encoding>  //
                      || std::is_same_v<type, utf8_char_encoding>,
                  "invalid value for LEXY_ENCODING_OF_CHAR");
#else
    using type = default_encoding; // Don't know the exact encoding.
#endif
};

#if LEXY_HAS_CHAR8_T
template <>
struct _deduce_encoding<LEXY_CHAR8_T>
{
    using type = utf8_encoding;
};
#endif
template <>
struct _deduce_encoding<char16_t>
{
    using type = utf16_encoding;
};
template <>
struct _deduce_encoding<char32_t>
{
    using type = utf32_encoding;
};

template <>
struct _deduce_encoding<unsigned char>
{
    using type = byte_encoding;
};
template <>
struct _deduce_encoding<std::byte>
{
    using type = byte_encoding;
};
} // namespace lexy

//=== encoding traits ===//
namespace lexy
{
template <typename Encoding>
constexpr auto is_unicode_encoding
    = std::is_same_v<Encoding, ascii_encoding> || std::is_same_v<Encoding, utf8_encoding>
      || std::is_same_v<Encoding, utf8_char_encoding> || std::is_same_v<Encoding, utf16_encoding>
      || std::is_same_v<Encoding, utf32_encoding>;

template <typename Encoding>
constexpr auto is_text_encoding
    = is_unicode_encoding<Encoding> || std::is_same_v<Encoding, default_encoding>;

template <typename Encoding>
constexpr auto is_byte_encoding = std::is_same_v<Encoding, byte_encoding>;

template <typename Encoding>
constexpr auto is_char_encoding = is_text_encoding<Encoding> || is_byte_encoding<Encoding>;

template <typename Encoding>
constexpr auto is_node_encoding = false;
} // namespace lexy

//=== impls ===//
namespace lexy::_detail
{
template <typename Encoding, typename CharT>
constexpr bool is_compatible_char_type = std::is_same_v<typename Encoding::char_type, CharT>
                                         || Encoding::template is_secondary_char_type<CharT>();

template <typename Encoding, typename CharT>
using require_secondary_char_type
    = std::enable_if_t<Encoding::template is_secondary_char_type<CharT>()>;

template <typename CharT>
constexpr bool is_ascii(CharT c)
{
    if constexpr (std::is_signed_v<CharT>)
        return 0 <= c && c <= 0x7F;
    else
        return c <= 0x7F;
}

template <typename TargetCharT, typename CharT>
LEXY_CONSTEVAL TargetCharT transcode_char(CharT c)
{
    if constexpr (std::is_same_v<CharT, TargetCharT>)
    {
        return c;
    }
#if !LEXY_HAS_CHAR8_T
    else if constexpr (std::is_same_v<CharT, char> && std::is_same_v<TargetCharT, LEXY_CHAR8_T>)
    {
        // If we don't have char8_t, `LEXY_LIT(u8"ä")` would have the type char, not LEXY_CHAR8_T
        // (which is unsigned char). So we disable checking in that case, to allow such usage. Note
        // that this prevents catching `LEXY_LIT("ä")`, but there is nothing we can do.
        return static_cast<LEXY_CHAR8_T>(c);
    }
#endif
    else
    {
        LEXY_ASSERT(is_ascii(c), "character type of string literal didn't match, "
                                 "so only ASCII characters are supported");
        // Note that we don't need to worry about signed/unsigned conversion, all ASCII values are
        // positive.
        return static_cast<TargetCharT>(c);
    }
}

template <typename Encoding, typename CharT>
LEXY_CONSTEVAL auto transcode_int(CharT c) -> typename Encoding::int_type
{
    return Encoding::to_int_type(transcode_char<typename Encoding::char_type>(c));
}
} // namespace lexy::_detail

#endif // LEXY_ENCODING_HPP_INCLUDED

