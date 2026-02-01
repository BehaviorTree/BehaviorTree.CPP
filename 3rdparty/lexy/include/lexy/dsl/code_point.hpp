// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CODE_POINT_HPP_INCLUDED
#define LEXY_DSL_CODE_POINT_HPP_INCLUDED

#include <lexy/code_point.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>

namespace lexyd
{
template <typename Predicate>
struct _cp : char_class_base<_cp<Predicate>>
{
    static LEXY_CONSTEVAL auto char_class_name()
    {
        if constexpr (std::is_void_v<Predicate>)
            return "code-point";
        else
            return lexy::_detail::type_name<Predicate>();
    }

    static LEXY_CONSTEVAL auto char_class_ascii()
    {
        if constexpr (std::is_void_v<Predicate>)
        {
            lexy::_detail::ascii_set result;
            result.insert(0x00, 0x7F);
            return result;
        }
        else
        {
            lexy::_detail::ascii_set result;
            for (auto c = 0; c <= 0x7F; ++c)
                if (Predicate{}(lexy::code_point(char32_t(c))))
                    result.insert(c);
            return result;
        }
    }

    static constexpr bool char_class_match_cp([[maybe_unused]] char32_t cp)
    {
        if constexpr (std::is_void_v<Predicate>)
            return true;
        else
            return Predicate{}(lexy::code_point(cp));
    }

    //=== dsl ===//
    template <typename P>
    constexpr auto if_() const
    {
        static_assert(std::is_void_v<Predicate>);
        return _cp<P>{};
    }

    template <char32_t Low, char32_t High>
    constexpr auto range() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.range";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return Low <= cp.value() && cp.value() <= High;
            }
        };

        return if_<predicate>();
    }

    template <char32_t... CPs>
    constexpr auto set() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.set";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return ((cp.value() == CPs) || ...);
            }
        };

        return if_<predicate>();
    }

    constexpr auto ascii() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.ASCII";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return cp.is_ascii();
            }
        };

        return if_<predicate>();
    }
    constexpr auto bmp() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.BMP";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return cp.is_bmp();
            }
        };

        return if_<predicate>();
    }
    constexpr auto noncharacter() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "code-point.non-character";
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                return cp.is_noncharacter();
            }
        };

        return if_<predicate>();
    }

    template <lexy::code_point::general_category_t Category>
    constexpr auto general_category() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return lexy::_detail::general_category_name(Category);
            }

            constexpr bool operator()(lexy::code_point cp) const
            {
                // Note: can't use `cp.is_noncharacter()` for `Cn` as `Cn` also includes all code
                // points that are currently unassigned.
                if constexpr (Category == lexy::code_point::Cc)
                    return cp.is_control();
                else if constexpr (Category == lexy::code_point::Cs)
                    return cp.is_surrogate();
                else if constexpr (Category == lexy::code_point::Co)
                    return cp.is_private_use();
                else
                    return cp.general_category() == Category;
            }
        };

        return if_<predicate>();
    }

    template <const auto& GcGroup>
    struct _group_pred;
    template <lexy::code_point::general_category_t... Cats,
              const lexy::code_point::_gc_group<Cats...>& GcGroup>
    struct _group_pred<GcGroup>
    {
        static LEXY_CONSTEVAL auto name()
        {
            return GcGroup.name;
        }

        constexpr bool operator()(lexy::code_point cp) const
        {
            return cp.general_category() == GcGroup;
        }
    };
    template <const auto& GcGroup>
    constexpr auto general_category() const
    {
        return if_<_group_pred<GcGroup>>();
    }
};

/// Matches a single unicode code point in the current unicode encoding.
constexpr auto code_point = _cp<void>{};
} // namespace lexyd

namespace lexy
{
// The void-version without predicate logically matches any input (modulo encoding errors, of
// course).
template <>
inline constexpr auto token_kind_of<lexy::dsl::_cp<void>> = lexy::any_token_kind;
} // namespace lexy

#endif // LEXY_DSL_CODE_POINT_HPP_INCLUDED

