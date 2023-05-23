// Copyright (C) 2020-2023 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PARSE_AS_HPP_INCLUDED
#define LEXY_DSL_PARSE_AS_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/callback/object.hpp>
#include <lexy/dsl/base.hpp>

namespace lexyd
{
// Custom handler that forwards events but overrides the value callback.
template <typename T, typename CurProduction, typename Handler>
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

    // For child productions, use ::value to get a value.
    template <typename Production, typename State>
    struct value_callback : lexy::production_value_callback<Production, State>
    {
        using lexy::production_value_callback<Production, State>::production_value_callback;
    };
    // For the production that contains parse_as, use lexy::construct.
    template <typename State>
    struct value_callback<CurProduction, State> : lexy::_construct<T>
    {
        constexpr value_callback() = default;
        constexpr value_callback(State*) {}
    };
};

template <typename T, typename CurProduction, typename Handler>
constexpr auto _make_pas_handler(Handler& handler)
{
    return _pas_handler<T, CurProduction, Handler>{handler};
}
// Prevent infinite nesting when parse_as itself is recursive.
template <typename T, typename CurProduction, typename U, typename P, typename Handler>
constexpr auto _make_pas_handler(_pas_handler<U, P, Handler>& handler)
{
    return _pas_handler<T, CurProduction, Handler>{handler._handler};
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
            auto handler = _make_pas_handler<T, typename Context::production>(
                context.control_block->parse_handler);
            lexy::_detail::parse_context_control_block cb(LEXY_MOV(handler), context.control_block);
            using context_type
                = lexy::_pc<decltype(handler), typename Context::state_type,
                            typename Context::production, typename Context::whitespace_production>;
            context_type sub_context(&cb);

            auto result
                = rule_parser.template finish<lexy::_detail::final_parser>(sub_context, reader);

            context.control_block->copy_vars_from(&cb);

            if (!result)
                return false;
            else if constexpr (std::is_void_v<T>)
                // NOLINTNEXTLINE: clang-tidy wrongly thinks the branch is repeated.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (Front)
                return NextParser::parse(context, reader, *LEXY_MOV(sub_context.value),
                                         LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                         *LEXY_MOV(sub_context.value));
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto handler = _make_pas_handler<T, typename Context::production>(
                context.control_block->parse_handler);
            lexy::_detail::parse_context_control_block cb(LEXY_MOV(handler), context.control_block);
            using context_type
                = lexy::_pc<decltype(handler), typename Context::state_type,
                            typename Context::production, typename Context::whitespace_production>;
            context_type sub_context(&cb);

            auto result
                = lexy::parser_for<Rule, lexy::_detail::final_parser>::parse(sub_context, reader);

            context.control_block->copy_vars_from(&cb);

            if (!result)
                return false;
            else if constexpr (std::is_void_v<T>)
                // NOLINTNEXTLINE: clang-tidy wrongly thinks the branch is repeated.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (Front)
                return NextParser::parse(context, reader, *LEXY_MOV(sub_context.value),
                                         LEXY_FWD(args)...);
            else
                return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                         *LEXY_MOV(sub_context.value));
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

