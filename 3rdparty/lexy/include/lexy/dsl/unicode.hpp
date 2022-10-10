// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_UNICODE_HPP_INCLUDED
#define LEXY_DSL_UNICODE_HPP_INCLUDED

#include <lexy/code_point.hpp>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>

namespace lexyd::unicode
{
struct _control : char_class_base<_control>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.control";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_control::char_class_ascii();
    }

    static constexpr bool char_class_match_cp(char32_t cp)
    {
        return lexy::code_point(cp).is_control();
    }
};
inline constexpr auto control = _control{};

//=== whitespace ===//
struct _blank : char_class_base<_blank>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.blank";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_blank::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // tab already handled as part of ASCII
        return lexy::code_point(cp).general_category() == lexy::code_point::space_separator;
    }
};
inline constexpr auto blank = _blank{};

struct _newline : char_class_base<_newline>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.newline";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_newline::char_class_ascii();
    }

    static constexpr bool char_class_match_cp(char32_t cp)
    {
        // NEL, PARAGRAPH SEPARATOR, LINE SEPARATOR
        return cp == 0x85 || cp == 0x2029 || cp == 0x2028;
    }
};
inline constexpr auto newline = _newline{};

struct _other_space : char_class_base<_other_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.other-space";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_other_space::char_class_ascii();
    }

    // The same as in ASCII, so no match function needed.
};
inline constexpr auto other_space = _other_space{};

struct _space : char_class_base<_space>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.whitespace";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_space::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(whitespace)>(cp);
    }
};
inline constexpr auto space = _space{};

//=== alpha ===//
struct _lower : char_class_base<_lower>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.lowercase";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_lower::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(lowercase)>(cp);
    }
};
inline constexpr auto lower = _lower{};

struct _upper : char_class_base<_upper>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.uppercase";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_upper::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(uppercase)>(cp);
    }
};
inline constexpr auto upper = _upper{};

struct _alpha : char_class_base<_alpha>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.alphabetic";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alpha::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(alphabetic)>(cp);
    }
};
inline constexpr auto alpha = _alpha{};

//=== digit ===//
struct _digit : char_class_base<_digit>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.decimal-number";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_digit::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::code_point(cp).general_category() == lexy::code_point::decimal_number;
    }
};
inline constexpr auto digit = _digit{};

struct _alnum : char_class_base<_alnum>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.alphabetic-decimal";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alnum::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(alphabetic)>(cp)
               || lexy::code_point(cp).general_category() == lexy::code_point::decimal_number;
    }
};
inline constexpr auto alnum       = _alnum{};
inline constexpr auto alpha_digit = alnum;

struct _word : char_class_base<_word>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.word";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_word::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        auto cat = lexy::code_point(cp).general_category();
        if (cat == lexy::code_point::mark || cat == lexy::code_point::connector_punctuation
            || cat == lexy::code_point::decimal_number)
            return true;

        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(alphabetic),
                                                        LEXY_UNICODE_PROPERTY(join_control)>(cp);
    }
};
inline constexpr auto word = _word{};

//=== categories ===//
struct _graph : char_class_base<_graph>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.graph";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_graph::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t _cp)
    {
        auto cp = lexy::code_point(_cp);

        // Everything that isn't control, surrogate, unassigned, or space.
        return !cp.is_control() && !cp.is_surrogate()
               && cp.general_category() != lexy::code_point::unassigned
               && !_space::char_class_match_cp(cp.value());
    }
};
inline constexpr auto graph = _graph{};

struct _print : char_class_base<_print>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.print";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_print::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // blank or graph without control.
        return !_control::char_class_match_cp(cp)
               && (_blank::char_class_match_cp(cp) || _graph::char_class_match_cp(cp));
    }
};
inline constexpr auto print = _print{};

struct _char : char_class_base<_char>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.character";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_char::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::code_point(cp).general_category() != lexy::code_point::unassigned;
    }
};
inline constexpr auto character = _char{};
} // namespace lexyd::unicode

namespace lexyd::unicode
{
struct _xid_start : char_class_base<_xid_start>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.XID-start";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alpha::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(xid_start)>(cp);
    }
};
inline constexpr auto xid_start = _xid_start{};

struct _xid_start_underscore : char_class_base<_xid_start_underscore>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.XID-start-underscore";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_alphau::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // underscore handled as part of ASCII.
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(xid_start)>(cp);
    }
};
inline constexpr auto xid_start_underscore = _xid_start_underscore{};

struct _xid_continue : char_class_base<_xid_continue>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        return "code-point.XID-continue";
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        return ascii::_word::char_class_ascii();
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp)
    {
        // underscore handled as part of ASCII.
        return lexy::_detail::code_point_has_properties<LEXY_UNICODE_PROPERTY(xid_continue)>(cp);
    }
};
inline constexpr auto xid_continue = _xid_continue{};
} // namespace lexyd::unicode

#endif // LEXY_DSL_UNICODE_HPP_INCLUDED

