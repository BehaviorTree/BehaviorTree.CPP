// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_DIGIT_HPP_INCLUDED
#define LEXY_DSL_DIGIT_HPP_INCLUDED

#include <lexy/_detail/swar.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

//=== bases ===//
// SWAR matching code adapted from:
// https://lemire.me/blog/2018/09/30/quickly-identifying-a-sequence-of-digits-in-a-string-of-characters/
namespace lexyd
{
template <int Radix>
struct _d;

template <>
struct _d<2> : char_class_base<_d<2>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.binary";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '1');
        return result;
    }

    static constexpr unsigned digit_radix = 2;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        constexpr auto mask     = lexy::_detail::swar_fill_compl(CharT(0xF));
        constexpr auto expected = lexy::_detail::swar_fill(CharT(0x30));
        constexpr auto offset   = lexy::_detail::swar_fill(CharT(0x0E));

        return (c & mask) == expected && ((c + offset) & mask) == expected;
    }
};
using binary = _d<2>;

template <>
struct _d<8> : char_class_base<_d<8>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.octal";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '7');
        return result;
    }

    static constexpr unsigned digit_radix = 8;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        constexpr auto mask     = lexy::_detail::swar_fill_compl(CharT(0xF));
        constexpr auto expected = lexy::_detail::swar_fill(CharT(0x30));
        constexpr auto offset   = lexy::_detail::swar_fill(CharT(0x08));

        return (c & mask) == expected && ((c + offset) & mask) == expected;
    }
};
using octal = _d<8>;

template <>
struct _d<10> : char_class_base<_d<10>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.decimal";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        return result;
    }

    static constexpr unsigned digit_radix = 10;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        return static_cast<unsigned>(c) - '0';
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        constexpr auto mask     = lexy::_detail::swar_fill_compl(CharT(0xF));
        constexpr auto expected = lexy::_detail::swar_fill(CharT(0x30));
        constexpr auto offset   = lexy::_detail::swar_fill(CharT(0x06));

        return (c & mask) == expected && ((c + offset) & mask) == expected;
    }
};
using decimal = _d<10>;

struct hex_lower : char_class_base<hex_lower>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.hex-lower";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        result.insert('a', 'f');
        return result;
    }

    static constexpr unsigned digit_radix = 16;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        if (c >= 'a')
            return static_cast<unsigned>(c) - 'a' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        // False negative for hex digits, but that's okay.
        return _d<10>::swar_matches<CharT>(c);
    }
};

struct hex_upper : char_class_base<hex_upper>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.hex-upper";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        result.insert('A', 'F');
        return result;
    }

    static constexpr unsigned digit_radix = 16;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        if (c >= 'A')
            return static_cast<unsigned>(c) - 'A' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        // False negative for hex digits, but that's okay.
        return _d<10>::swar_matches<CharT>(c);
    }
};

template <>
struct _d<16> : char_class_base<_d<16>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.hex";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        result.insert('a', 'f');
        result.insert('A', 'F');
        return result;
    }

    static constexpr unsigned digit_radix = 16;

    template <typename CharT>
    static constexpr unsigned digit_value(CharT c)
    {
        if (c >= 'a')
            return static_cast<unsigned>(c) - 'a' + 10;
        else if (c >= 'A')
            return static_cast<unsigned>(c) - 'A' + 10;
        else if (c <= '9')
            return static_cast<unsigned>(c) - '0';
        else
            return unsigned(-1);
    }

    template <typename CharT>
    static constexpr bool swar_matches(lexy::_detail::swar_int c)
    {
        // False negative for hex digits, but that's okay.
        return _d<10>::swar_matches<CharT>(c);
    }
};
using hex = _d<16>;
} // namespace lexyd

//=== digit ===//
namespace lexyd
{
struct _zero : char_class_base<_zero>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "digit.zero";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0');
        return result;
    }
};

/// Matches the zero digit.
constexpr auto zero = _zero{};

/// Matches a single digit.
template <typename Base = decimal, int = Base::digit_radix>
constexpr auto digit = Base{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_zero> = lexy::digits_token_kind;

template <int Radix>
constexpr auto token_kind_of<lexy::dsl::_d<Radix>> = lexy::digits_token_kind;
template <>
inline constexpr auto token_kind_of<lexy::dsl::hex_lower> = lexy::digits_token_kind;
template <>
inline constexpr auto token_kind_of<lexy::dsl::hex_upper> = lexy::digits_token_kind;
} // namespace lexy

//=== digits ===//
namespace lexy
{
struct forbidden_leading_zero
{
    static LEXY_CONSTEVAL auto name()
    {
        return "forbidden leading zero";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Base, typename Reader>
constexpr bool _match_digits(Reader& reader)
{
    // Need at least one digit.
    // Checking for a single digit is also cheaper than doing a SWAR comparison,
    // so we do that manually in either case.
    if (!lexy::try_match_token(digit<Base>, reader))
        return false;

    // Now we consume as many digits as possible.
    // First using SWAR...
    if constexpr (lexy::_detail::is_swar_reader<Reader>)
    {
        using char_type = typename Reader::encoding::char_type;
        while (Base::template swar_matches<char_type>(reader.peek_swar()))
            reader.bump_swar();
    }

    // ... then manually to get any trailing digits.
    while (lexy::try_match_token(digit<Base>, reader))
    {
    }

    return true;
}
template <typename Base, typename Sep, typename Reader>
constexpr bool _match_digits_sep(Reader& reader)
{
    // Need at least one digit.
    if (!lexy::try_match_token(digit<Base>, reader))
        return false;

    // Might have following digits.
    while (true)
    {
        if (lexy::try_match_token(Sep{}, reader))
        {
            // Need a digit after a separator.
            if (!lexy::try_match_token(digit<Base>, reader))
                return false;
        }
        else
        {
            // Attempt to consume as many digits as possible.
            if constexpr (lexy::_detail::is_swar_reader<Reader>)
            {
                using char_type = typename Reader::encoding::char_type;
                while (Base::template swar_matches<char_type>(reader.peek_swar()))
                    reader.bump_swar();
            }

            if (!lexy::try_match_token(digit<Base>, reader))
                // If we're not having a digit, we're done.
                break;
        }
    }

    return true;
}

template <typename Base, typename Sep>
struct _digits_st : token_base<_digits_st<Base, Sep>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;
        bool                    forbidden_leading_zero;

        constexpr explicit tp(const Reader& reader)
        : end(reader.current()), forbidden_leading_zero(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            using char_type = typename Reader::encoding::char_type;
            auto begin      = reader.current();
            auto result     = _match_digits_sep<Base, Sep>(reader);
            end             = reader.current();

            if (result && lexy::_detail::next(begin.position()) != end.position()
                && *begin.position() == lexy::_detail::transcode_char<char_type>('0'))
            {
                reader.reset(begin);
                reader.bump();
                end = reader.current();

                forbidden_leading_zero = true;
                return false;
            }

            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (forbidden_leading_zero)
            {
                auto err = lexy::error<Reader, lexy::forbidden_leading_zero>(reader.position(),
                                                                             end.position());
                context.on(_ev::error{}, err);
            }
            else
            {
                auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                          Base::char_class_name());
                context.on(_ev::error{}, err);
            }
        }
    };
};

template <typename Base, typename Sep>
struct _digits_s : token_base<_digits_s<Base, Sep>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto result = _match_digits_sep<Base, Sep>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                      Base::char_class_name());
            context.on(_ev::error{}, err);
        }
    };

    constexpr auto no_leading_zero() const
    {
        return _digits_st<Base, Sep>{};
    }
};

template <typename Base>
struct _digits_t : token_base<_digits_t<Base>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;
        bool                    forbidden_leading_zero;

        constexpr explicit tp(const Reader& reader)
        : end(reader.current()), forbidden_leading_zero(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            using char_type = typename Reader::encoding::char_type;
            auto begin      = reader.current();
            auto result     = _match_digits<Base>(reader);
            end             = reader.current();

            if (result && lexy::_detail::next(begin.position()) != end.position()
                && *begin.position() == lexy::_detail::transcode_char<char_type>('0'))
            {
                reader.reset(begin);
                reader.bump();
                end = reader.current();

                forbidden_leading_zero = true;
                return false;
            }

            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            if (forbidden_leading_zero)
            {
                auto err = lexy::error<Reader, lexy::forbidden_leading_zero>(reader.position(),
                                                                             end.position());
                context.on(_ev::error{}, err);
            }
            else
            {
                auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                          Base::char_class_name());
                context.on(_ev::error{}, err);
            }
        }
    };

    template <typename Token>
    constexpr auto sep(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _digits_st<Base, Token>{};
    }
};

template <typename Base>
struct _digits : token_base<_digits<Base>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            auto result = _match_digits<Base>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(),
                                                                      Base::char_class_name());
            context.on(_ev::error{}, err);
        }
    };

    template <typename Token>
    constexpr auto sep(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _digits_s<Base, Token>{};
    }

    constexpr auto no_leading_zero() const
    {
        return _digits_t<Base>{};
    }
};

/// Matches a non-empty list of digits.
template <typename Base = decimal>
constexpr auto digits = _digits<Base>{};

constexpr auto digit_sep_underscore = LEXY_LIT("_");
constexpr auto digit_sep_tick       = LEXY_LIT("'");
} // namespace lexyd

namespace lexy
{
template <typename Base>
constexpr auto token_kind_of<lexy::dsl::_digits<Base>> = lexy::digits_token_kind;
template <typename Base>
constexpr auto token_kind_of<lexy::dsl::_digits_t<Base>> = lexy::digits_token_kind;
template <typename Base, typename Sep>
constexpr auto token_kind_of<lexy::dsl::_digits_s<Base, Sep>> = lexy::digits_token_kind;
template <typename Base, typename Sep>
constexpr auto token_kind_of<lexy::dsl::_digits_st<Base, Sep>> = lexy::digits_token_kind;
} // namespace lexy

//=== n_digits ===//
namespace lexyd
{
template <std::size_t N, typename Base, typename Sep>
struct _ndigits_s : token_base<_ndigits_s<N, Base, Sep>>
{
    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<N - 1>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the Base one time.
            if (!lexy::try_match_token(digit<Base>, reader))
            {
                end = reader.current();
                return false;
            }

            // Match each other digit after a separator.
            auto success = (((void)Idx, lexy::try_match_token(Sep{}, reader),
                             lexy::try_match_token(digit<Base>, reader))
                            && ...);
            end          = reader.current();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                      Base::char_class_name());
            context.on(_ev::error{}, err);
        }
    };
};

template <std::size_t N, typename Base>
struct _ndigits : token_base<_ndigits<N, Base>>
{
    static_assert(N > 1);

    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<N>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Match the Base N times.
            auto success = (((void)Idx, lexy::try_match_token(digit<Base>, reader)) && ...);
            end          = reader.current();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(),
                                                                      Base::char_class_name());
            context.on(_ev::error{}, err);
        }
    };

    template <typename Token>
    constexpr auto sep(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        return _ndigits_s<N, Base, Token>{};
    }
};

/// Matches exactly N digits.
template <std::size_t N, typename Base = decimal>
constexpr auto n_digits = _ndigits<N, Base>{};
} // namespace lexyd

namespace lexy
{
template <std::size_t N, typename Base>
constexpr auto token_kind_of<lexy::dsl::_ndigits<N, Base>> = lexy::digits_token_kind;
template <std::size_t N, typename Base, typename Sep>
constexpr auto token_kind_of<lexy::dsl::_ndigits_s<N, Base, Sep>> = lexy::digits_token_kind;
} // namespace lexy

#endif // LEXY_DSL_DIGIT_HPP_INCLUDED

