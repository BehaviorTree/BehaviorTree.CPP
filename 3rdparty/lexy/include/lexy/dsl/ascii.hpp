// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ASCII_HPP_INCLUDED
#define LEXY_DSL_ASCII_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>

namespace lexyd::ascii
{
//=== control ===//
struct _control : char_class_base<_control>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.control";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x00, 0x1F);
        result.insert(0x7F);
        return result;
    }
};
inline constexpr auto control = _control{};

//=== whitespace ===//
struct _blank : char_class_base<_blank>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.blank";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(' ');
        result.insert('\t');
        return result;
    }
};
inline constexpr auto blank = _blank{};

struct _newline : char_class_base<_newline>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.newline";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('\n');
        result.insert('\r');
        return result;
    }
};
inline constexpr auto newline = _newline{};

struct _other_space : char_class_base<_other_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.other-space";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('\f');
        result.insert('\v');
        return result;
    }
};
inline constexpr auto other_space = _other_space{};

struct _space : char_class_base<_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.space";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(_blank::char_class_ascii());
        result.insert(_newline::char_class_ascii());
        result.insert(_other_space::char_class_ascii());
        return result;
    }
};
inline constexpr auto space = _space{};

//=== alpha ===//
struct _lower : char_class_base<_lower>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.lower";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('a', 'z');
        return result;
    }
};
inline constexpr auto lower = _lower{};

struct _upper : char_class_base<_upper>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.upper";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('A', 'Z');
        return result;
    }
};
inline constexpr auto upper = _upper{};

struct _alpha : char_class_base<_alpha>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.alpha";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('a', 'z');
        result.insert('A', 'Z');
        return result;
    }
};
inline constexpr auto alpha = _alpha{};

struct _alphau : char_class_base<_alphau>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.alpha-underscore";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('a', 'z');
        result.insert('A', 'Z');
        result.insert('_');
        return result;
    }
};
inline constexpr auto alpha_underscore = _alphau{};

//=== digit ===//
struct _digit : char_class_base<_digit>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.digit";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('0', '9');
        return result;
    }
};
inline constexpr auto digit = _digit{};

struct _alnum : char_class_base<_alnum>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.alpha-digit";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(_alpha::char_class_ascii());
        result.insert(_digit::char_class_ascii());
        return result;
    }
};
inline constexpr auto alnum       = _alnum{};
inline constexpr auto alpha_digit = _alnum{};

struct _word : char_class_base<_word>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.word";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(_alphau::char_class_ascii());
        result.insert(_digit::char_class_ascii());
        return result;
    }
};
inline constexpr auto word                   = _word{};
inline constexpr auto alpha_digit_underscore = _word{};

//=== punct ===//
struct _punct : char_class_base<_punct>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.punct";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert('!');
        result.insert('"');
        result.insert('#');
        result.insert('$');
        result.insert('%');
        result.insert('&');
        result.insert('\'');
        result.insert('(');
        result.insert(')');
        result.insert('*');
        result.insert('+');
        result.insert(',');
        result.insert('-');
        result.insert('.');
        result.insert('/');
        result.insert(':');
        result.insert(';');
        result.insert('<');
        result.insert('=');
        result.insert('>');
        result.insert('?');
        result.insert('@');
        result.insert('[');
        result.insert('\\');
        result.insert(']');
        result.insert('^');
        result.insert('_');
        result.insert('`');
        result.insert('{');
        result.insert('|');
        result.insert('}');
        result.insert('~');
        return result;
    }
};
inline constexpr auto punct = _punct{};

//=== categories ===//
struct _graph : char_class_base<_graph>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.graph";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x21, 0x7E);
        return result;
    }
};
inline constexpr auto graph = _graph{};

struct _print : char_class_base<_print>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII.print";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x20, 0x7E);
        return result;
    }
};
inline constexpr auto print = _print{};

struct _char : char_class_base<_char>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "ASCII";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        result.insert(0x00, 0x7F);
        return result;
    }
};
inline constexpr auto character = _char{};
} // namespace lexyd::ascii

namespace lexyd::ascii
{
template <char... C>
struct _alt : char_class_base<_alt<C...>>
{
    static_assert(sizeof...(C) > 0);

    static LEXY_CONSTEVAL auto char_class_name()
    {
        return lexy::_detail::type_string<char, C...>::template c_str<char>;
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        lexy::_detail::ascii_set result;
        (result.insert(C), ...);
        return result;
    }
};

template <typename CharT, CharT... C>
struct _one_of
{
    static_assert((std::is_same_v<CharT, char> && ... && lexy::_detail::is_ascii(C)),
                  "only ASCII characters are supported");

    using rule = _alt<C...>;
};

#if LEXY_HAS_NTTP
/// Matches one of the ASCII characters.
template <lexy::_detail::string_literal Str>
constexpr auto one_of = typename lexy::_detail::to_type_string<_one_of, Str>::rule{};
#endif

#define LEXY_ASCII_ONE_OF(Str)                                                                     \
    LEXY_NTTP_STRING(::lexyd::ascii::_one_of, Str)::rule {}
} // namespace lexyd::ascii

#endif // LEXY_DSL_ASCII_HPP_INCLUDED

