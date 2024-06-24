// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SEPARATOR_HPP_INCLUDED
#define LEXY_DSL_SEPARATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct unexpected_trailing_separator
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unexpected trailing separator";
    }
};
} // namespace lexy

namespace lexyd
{
// Reports the trailing sep error.
template <typename Branch, typename Tag>
struct _nsep : rule_base
{
    template <typename NextParser>
    struct p
    {
        struct _pc
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                               typename Reader::iterator sep_begin, Args&&... args)
            {
                auto sep_end = reader.position();

                using tag = lexy::_detail::type_or<Tag, lexy::unexpected_trailing_separator>;
                auto err  = lexy::error<Reader, tag>(sep_begin, sep_end);
                context.on(_ev::error{}, err);

                // Trivially recover.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (lexy::branch_parser_for<Branch, Reader> parser{};
                !parser.try_parse(context.control_block, reader))
            {
                // Didn't have the separator, everything is okay.
                parser.cancel(context);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                // Did have the separator, report error.
                return parser.template finish<_pc>(context, reader, reader.position(),
                                                   LEXY_FWD(args)...);
            }
        }
    };
};

template <typename Branch, typename Tag>
struct _sep : _sep_base
{
    using rule          = Branch;
    using trailing_rule = _nsep<Branch, Tag>;

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&                  context, Reader&,
                                                typename Reader::iterator sep_begin,
                                                typename Reader::iterator sep_end)
    {
        using tag = lexy::_detail::type_or<Tag, lexy::unexpected_trailing_separator>;
        auto err  = lexy::error<Reader, tag>(sep_begin, sep_end);
        context.on(_ev::error{}, err);
    }

    //=== dsl ===//
    template <typename NewTag>
    static constexpr _sep<Branch, NewTag> trailing_error = {};
};

/// Defines a separator for a list.
template <typename Branch>
constexpr auto sep(Branch)
{
    LEXY_REQUIRE_BRANCH_RULE(Branch, "sep");
    return _sep<Branch, void>{};
}

template <typename Branch>
struct _tsep : _sep_base
{
    using rule          = Branch;
    using trailing_rule = decltype(lexyd::if_(Branch{}));

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&, Reader&, typename Reader::iterator,
                                                typename Reader::iterator)
    {}
};

/// Defines a separator for a list that can be trailing.
template <typename Branch>
constexpr auto trailing_sep(Branch)
{
    LEXY_REQUIRE_BRANCH_RULE(Branch, "trailing_sep");
    return _tsep<Branch>{};
}

template <typename Branch>
struct _isep : _sep_base
{
    using rule          = Branch;
    using trailing_rule = _else;

    template <typename Context, typename Reader>
    static constexpr void report_trailing_error(Context&, Reader&, typename Reader::iterator,
                                                typename Reader::iterator)
    {}
};

/// Defines a separator for a list that ignores the existence of trailing separators.
template <typename Branch>
constexpr auto ignore_trailing_sep(Branch)
{
    static_assert(lexy::is_branch_rule<Branch>);
    return _isep<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEPARATOR_HPP_INCLUDED

