// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CODE_POINT_HPP_INCLUDED
#define LEXY_CODE_POINT_HPP_INCLUDED

#include <cstdint>
#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>

#if LEXY_HAS_UNICODE_DATABASE
#    define LEXY_UNICODE_CONSTEXPR constexpr
#else
#    define LEXY_UNICODE_CONSTEXPR
#endif

namespace lexy
{
/// A unicode code point.
class code_point
{
public:
    constexpr code_point() noexcept : _value(0xFFFF'FFFF) {}
    constexpr explicit code_point(char32_t value) noexcept : _value(value) {}

    constexpr auto value() const noexcept
    {
        return _value;
    }

    //=== classification ===//
    constexpr bool is_ascii() const noexcept
    {
        return _value <= 0x7F;
    }
    constexpr bool is_bmp() const noexcept
    {
        return _value <= 0xFFFF;
    }
    constexpr bool is_valid() const noexcept
    {
        return _value <= 0x10'FFFF;
    }

    constexpr bool is_control() const noexcept
    {
        return _value <= 0x1F || (0x7F <= _value && _value <= 0x9F);
    }
    constexpr bool is_surrogate() const noexcept
    {
        return 0xD800 <= _value && _value <= 0xDFFF;
    }
    constexpr bool is_private_use() const noexcept
    {
        return (0xE000 <= _value && _value <= 0xF8FF)
               || (0x0F'0000 <= _value && _value <= 0x0F'FFFD)
               || (0x10'0000 <= _value && _value <= 0x10'FFFD);
    }
    constexpr bool is_noncharacter() const noexcept
    {
        // Contiguous range of 32 non-characters.
        if (0xFDD0 <= _value && _value <= 0xFDEF)
            return true;

        // Last two code points of every plane.
        auto in_plane = _value & 0xFFFF;
        return in_plane == 0xFFFE || in_plane == 0xFFFF;
    }

    constexpr bool is_scalar() const noexcept
    {
        return is_valid() && !is_surrogate();
    }

    //=== general category ===//
    enum general_category_t
    {
    // NOLINTNEXTLINE: can't use parentheses here
#define LEXY_UNICODE_CATEGORY(Short, Long) Short, Long = Short

        LEXY_UNICODE_CATEGORY(Lu, uppercase_letter),
        LEXY_UNICODE_CATEGORY(Ll, lowercase_letter),
        LEXY_UNICODE_CATEGORY(Lt, titlecase_letter),
        LEXY_UNICODE_CATEGORY(Lm, modifier_letter),
        LEXY_UNICODE_CATEGORY(Lo, other_letter),

        LEXY_UNICODE_CATEGORY(Mn, nonspacing_mark),
        LEXY_UNICODE_CATEGORY(Mc, spacing_mark),
        LEXY_UNICODE_CATEGORY(Me, enclosing_mark),

        LEXY_UNICODE_CATEGORY(Nd, decimal_number),
        LEXY_UNICODE_CATEGORY(Nl, letter_number),
        LEXY_UNICODE_CATEGORY(No, other_number),

        LEXY_UNICODE_CATEGORY(Pc, connector_punctuation),
        LEXY_UNICODE_CATEGORY(Pd, dash_punctuation),
        LEXY_UNICODE_CATEGORY(Ps, open_punctuation),
        LEXY_UNICODE_CATEGORY(Pe, closing_punctuation),
        LEXY_UNICODE_CATEGORY(Pi, initial_puncutation),
        LEXY_UNICODE_CATEGORY(Pf, final_puncutation),
        LEXY_UNICODE_CATEGORY(Po, other_punctuation),

        LEXY_UNICODE_CATEGORY(Sm, math_symbol),
        LEXY_UNICODE_CATEGORY(Sc, currency_symbol),
        LEXY_UNICODE_CATEGORY(Sk, modifier_symbol),
        LEXY_UNICODE_CATEGORY(So, other_symbol),

        LEXY_UNICODE_CATEGORY(Zs, space_separator),
        LEXY_UNICODE_CATEGORY(Zl, line_separator),
        LEXY_UNICODE_CATEGORY(Zp, paragraph_separator),

        LEXY_UNICODE_CATEGORY(Cc, control),
        LEXY_UNICODE_CATEGORY(Cf, format),
        LEXY_UNICODE_CATEGORY(Cs, surrogate),
        LEXY_UNICODE_CATEGORY(Co, private_use),
        LEXY_UNICODE_CATEGORY(Cn, unassigned),

#undef LEXY_UNICODE_CATEGORY
    };

    template <general_category_t... Cats>
    struct _gc_group
    {
        const char* name;

        friend constexpr bool operator==(_gc_group, general_category_t cat)
        {
            return ((cat == Cats) || ...);
        }
        friend constexpr bool operator==(general_category_t cat, _gc_group)
        {
            return ((cat == Cats) || ...);
        }

        friend constexpr bool operator!=(_gc_group, general_category_t cat)
        {
            return !(_gc_group{} == cat);
        }
        friend constexpr bool operator!=(general_category_t cat, _gc_group)
        {
            return !(_gc_group{} == cat);
        }
    };

#define LEXY_UNICODE_CATEGORY_GROUP(Name, Short, Long, ...)                                        \
    static constexpr _gc_group<__VA_ARGS__> Short{"code-point." Name};                             \
    static constexpr _gc_group<__VA_ARGS__> Long = Short

    LEXY_UNICODE_CATEGORY_GROUP("cased-letter", LC, cased_letter, Lu, Ll, Lt);
    LEXY_UNICODE_CATEGORY_GROUP("letter", L, letter, Lu, Ll, Lt, Lm, Lo);
    LEXY_UNICODE_CATEGORY_GROUP("mark", M, mark, Mn, Mc, Me);
    LEXY_UNICODE_CATEGORY_GROUP("number", N, number, Nd, Nl, No);
    LEXY_UNICODE_CATEGORY_GROUP("punctuation", P, punctuation, Pc, Pd, Ps, Pe, Pi, Pf, Po);
    LEXY_UNICODE_CATEGORY_GROUP("symbol", S, symbol, Sm, Sc, Sk, So);
    LEXY_UNICODE_CATEGORY_GROUP("separator", Z, separator, Zs, Zl, Zp);
    LEXY_UNICODE_CATEGORY_GROUP("other", C, other, Cc, Cf, Cs, Co, Cn);

#undef LEXY_UNICODE_CATEGORY_GROUP

    LEXY_UNICODE_CONSTEXPR general_category_t general_category() const noexcept;

    //=== comparision ===//
    friend constexpr bool operator==(code_point lhs, code_point rhs) noexcept
    {
        return lhs._value == rhs._value;
    }
    friend constexpr bool operator!=(code_point lhs, code_point rhs) noexcept
    {
        return lhs._value != rhs._value;
    }

private:
    char32_t _value;
};

LEXY_UNICODE_CONSTEXPR code_point simple_case_fold(code_point cp) noexcept;
} // namespace lexy

namespace lexy::_detail
{
constexpr const char* general_category_name(lexy::code_point::general_category_t category)
{
    switch (category)
    {
    case lexy::code_point::Lu:
        return "code-point.uppercase-letter";
    case lexy::code_point::Ll:
        return "code-point.lowercase-letter";
    case lexy::code_point::Lt:
        return "code-point.titlecase-letter";
    case lexy::code_point::Lm:
        return "code-point.modifier-letter";
    case lexy::code_point::Lo:
        return "code-point.other-letter";

    case lexy::code_point::Mn:
        return "code-point.nonspacing-mark";
    case lexy::code_point::Mc:
        return "code-point.combining-mark";
    case lexy::code_point::Me:
        return "code-point.enclosing-mark";

    case lexy::code_point::Nd:
        return "code-point.decimal-number";
    case lexy::code_point::Nl:
        return "code-point.letter-number";
    case lexy::code_point::No:
        return "code-point.other-number";

    case lexy::code_point::Pc:
        return "code-point.connector-punctuation";
    case lexy::code_point::Pd:
        return "code-point.dash-punctuation";
    case lexy::code_point::Ps:
        return "code-point.open-punctuation";
    case lexy::code_point::Pe:
        return "code-point.close-punctuation";
    case lexy::code_point::Pi:
        return "code-point.initial-quote-punctuation";
    case lexy::code_point::Pf:
        return "code-point.final-quote-punctuation";
    case lexy::code_point::Po:
        return "code-point.other-punctuation";

    case lexy::code_point::Sm:
        return "code-point.math-symbol";
    case lexy::code_point::Sc:
        return "code-point.currency-symbol";
    case lexy::code_point::Sk:
        return "code-point.modifier-symbol";
    case lexy::code_point::So:
        return "code-point.other-symbol";

    case lexy::code_point::Zs:
        return "code-point.space-separator";
    case lexy::code_point::Zl:
        return "code-point.line-separator";
    case lexy::code_point::Zp:
        return "code-point.paragraph-separator";

    case lexy::code_point::Cc:
        return "code-point.control";
    case lexy::code_point::Cf:
        return "code-point.format";
    case lexy::code_point::Cs:
        return "code-point.surrogate";
    case lexy::code_point::Co:
        return "code-point.private-use";
    case lexy::code_point::Cn:
        return "code-point.not-assigned";
    }

    return nullptr; // unreachable
}
} // namespace lexy::_detail

#if LEXY_HAS_UNICODE_DATABASE
#    include <lexy/_detail/unicode_database.hpp>

constexpr lexy::code_point::general_category_t lexy::code_point::general_category() const noexcept
{
    if (!is_valid())
        return general_category_t::unassigned;

    auto idx = _unicode_db::property_index(_value);
    return _unicode_db::category[idx];
}

constexpr lexy::code_point lexy::simple_case_fold(code_point cp) noexcept
{
    if (!cp.is_valid())
        return cp;

    auto idx    = _unicode_db::property_index(cp.value());
    auto offset = _unicode_db::case_folding_offset[idx];
    return code_point(char32_t(std::int_least32_t(cp.value()) + offset));
}

namespace lexy::_detail
{
template <lexy::_unicode_db::binary_properties_t... Props>
LEXY_FORCE_INLINE constexpr bool code_point_has_properties(char32_t cp)
{
    constexpr auto mask = ((1 << Props) | ...);

    auto idx   = _unicode_db::property_index(cp);
    auto props = _unicode_db::binary_properties[idx];
    return (props & mask) != 0;
}
} // namespace lexy::_detail

#    define LEXY_UNICODE_PROPERTY(Name) ::lexy::_unicode_db::Name

#else
namespace lexy::_detail
{
template <int... Props>
bool code_point_has_properties(char32_t cp); // not implemented
} // namespace lexy::_detail

#    define LEXY_UNICODE_PROPERTY(Name) 0

#endif

#endif // LEXY_CODE_POINT_HPP_INCLUDED

