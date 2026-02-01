// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PARSE_AS_HPP_INCLUDED
#define LEXY_DSL_PARSE_AS_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/callback/object.hpp>
#include <lexy/dsl/base.hpp>

namespace lexyd
{
// Custom handler that forwards events but overrides the value callback.
template <typename Handler>
struct _pas_handler
{
    Handler& _handler;

    using event_handler = typename Handler::event_handler;

    // We are implicitly convertible to all handler types the original handler is convertible to.
    // This is because the handler is passed to event_handler::on.
    template <typename H, typename = decltype(static_cast<H&>(LEXY_DECLVAL(Handler&)))>
    constexpr operator H&() const
    {
        return static_cast<H&>(_handler);
    }

    // We use ::value to get a value.
    // We can't use it unconditionally, as the initial production that contains the parse_as might
    // not have one. So we silently fallback if that's the case - this might cause worse errors if
    // the value is missing.
    template <typename Production, typename State>
    using value_callback
        = std::conditional_t<lexy::production_has_value_callback<Production, State>,
                             lexy::production_value_callback<Production, State>,
                             lexy::_detail::void_value_callback>;
};

struct _pas_final_parser
{
    template <typename Context, typename Reader, typename T, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context&, Reader&, lexy::_detail::lazy_init<T>& value,
                                       Args&&... args)
    {
        value.emplace_result(lexy::construct<T>, LEXY_FWD(args)...);
        return true;
    }
};

template <typename Handler>
constexpr auto _make_pas_handler(Handler& handler)
{
    return _pas_handler<Handler>{handler};
}
// Prevent infinite nesting when parse_as itself is recursive.
template <typename Handler>
constexpr auto _make_pas_handler(_pas_handler<Handler>& handler)
{
    return handler;
}

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
            auto handler = _make_pas_handler(context.control_block->parse_handler);
            lexy::_detail::parse_context_control_block cb(LEXY_MOV(handler), context.control_block);
            using context_type
                = lexy::_pc<decltype(handler), typename Context::state_type,
                            typename Context::production, typename Context::whitespace_production>;
            context_type sub_context(&cb);
            sub_context.handler = LEXY_MOV(context).handler;

            lexy::_detail::lazy_init<T> value;
            auto                        result
                = rule_parser.template finish<_pas_final_parser>(sub_context, reader, value);

            context.control_block->copy_vars_from(&cb);
            context.handler = LEXY_MOV(sub_context).handler;

            if (!result)
                return false;
            else if constexpr (std::is_void_v<T>)
                // NOLINTNEXTLINE: clang-tidy wrongly thinks the branch is repeated.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (Front)
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
            auto handler = _make_pas_handler(context.control_block->parse_handler);
            lexy::_detail::parse_context_control_block cb(LEXY_MOV(handler), context.control_block);
            using context_type
                = lexy::_pc<decltype(handler), typename Context::state_type,
                            typename Context::production, typename Context::whitespace_production>;
            context_type sub_context(&cb);
            sub_context.handler = LEXY_MOV(context).handler;

            lexy::_detail::lazy_init<T> value;
            auto                        result
                = lexy::parser_for<Rule, _pas_final_parser>::parse(sub_context, reader, value);

            context.control_block->copy_vars_from(&cb);
            context.handler = LEXY_MOV(sub_context).handler;

            if (!result)
                return false;
            else if constexpr (std::is_void_v<T>)
                // NOLINTNEXTLINE: clang-tidy wrongly thinks the branch is repeated.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (Front)
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

