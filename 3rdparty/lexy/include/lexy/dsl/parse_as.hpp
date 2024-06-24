// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PARSE_AS_HPP_INCLUDED
#define LEXY_DSL_PARSE_AS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/scan.hpp>

namespace lexyd
{
template <typename T, typename Rule, bool Front = false>
struct _pas : _copy_base<Rule>
{
    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule_parser;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return rule_parser.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            // No need to use the special context here; it doesn't produce any values.
            rule_parser.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            lexy::_detail::lazy_init<T> value;

            if (lexy::_detail::spc scan_context(value, context);
                !rule_parser.template finish<lexy::_detail::final_parser>(scan_context, reader))
                return false;

            if constexpr (Front)
                return NextParser::parse(context, reader, *LEXY_MOV(value), LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., *LEXY_MOV(value));
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::_detail::lazy_init<T> value;

            if (lexy::_detail::spc scan_context(value, context);
                !lexy::parser_for<Rule, lexy::_detail::final_parser>::parse(scan_context, reader))
                return false;

            if constexpr (Front)
                return NextParser::parse(context, reader, *LEXY_MOV(value), LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)..., *LEXY_MOV(value));
        }
    };
};

template <typename T, typename Rule>
constexpr auto parse_as(Rule)
{
    return _pas<T, Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PARSE_AS_HPP_INCLUDED

