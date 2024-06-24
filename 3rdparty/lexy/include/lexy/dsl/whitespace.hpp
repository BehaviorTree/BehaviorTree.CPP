// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/token.hpp>

//=== implementation ===//
namespace lexy::_detail
{
template <typename Rule>
struct ws_production
{
    static constexpr auto max_recursion_depth = 0;
    static constexpr auto rule                = lexy::dsl::loop(Rule{} | lexy::dsl::break_);
};

// A special handler for parsing whitespace.
// It only forwards errors to the context and ignores all other events.
template <typename Context>
class whitespace_handler
{
public:
    constexpr explicit whitespace_handler(Context& context) : _context(&context) {}

    template <typename Production>
    struct event_handler
    {
        static_assert(_detail::error<Production>,
                      "whitespace rule must not contain `dsl::p` or `dsl::recurse`;"
                      "use `dsl::inline_` instead");
    };
    template <typename Rule>
    class event_handler<ws_production<Rule>>
    {
    public:
        template <typename Error>
        constexpr void on(whitespace_handler& handler, parse_events::error ev, Error&& error)
        {
            handler._context->on(ev, LEXY_FWD(error));
        }

        template <typename Event, typename... Args>
        constexpr int on(whitespace_handler&, Event, const Args&...)
        {
            return 0; // an operation_start event returns something
        }
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr bool get_result_void(bool rule_parse_result) &&
    {
        return rule_parse_result;
    }

private:
    Context* _context;
};

template <typename Rule, typename NextParser>
struct manual_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        auto result = true;
        auto begin  = reader.position();
        if constexpr (lexy::is_token_rule<Rule>)
        {
            // Parsing a token repeatedly cannot fail, so we can optimize it.
            while (lexy::try_match_token(Rule{}, reader))
            {}
        }
        else
        {
            // Parse the rule using a special handler that only forwards errors.
            using production = ws_production<Rule>;
            result = lexy::do_action<production>(whitespace_handler(context), lexy::no_parse_state,
                                                 reader);
        }
        auto end = reader.position();

        if (result)
        {
            // Add a whitespace token node.
            context.on(lexy::parse_events::token{}, lexy::whitespace_token_kind, begin, end);
            // And continue.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Add an error token node.
            context.on(lexy::parse_events::token{}, lexy::error_token_kind, begin, end);
            // And cancel.
            return false;
        }
    }
};
template <typename NextParser>
struct manual_ws_parser<void, NextParser> : NextParser
{};

template <typename Context>
using context_whitespace = lexy::production_whitespace<typename Context::production,
                                                       typename Context::whitespace_production>;

// Inherit from it in a continuation to disable automatic whitespace skipping.
struct disable_whitespace_skipping
{};

template <typename NextParser>
struct automatic_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if (!std::is_base_of_v<disable_whitespace_skipping,
                               NextParser> && context.control_block->enable_whitespace_skipping)
        {
            // Skip the appropriate whitespace.
            using rule = context_whitespace<Context>;
            return manual_ws_parser<rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            // Automatic whitespace skipping is disabled.
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    }
};
} // namespace lexy::_detail

//=== whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsr : rule_base
{
    template <typename NextParser>
    using p = lexy::_detail::manual_ws_parser<Rule, NextParser>;

    template <typename R>
    friend constexpr auto operator|(_wsr<Rule>, R r)
    {
        return _wsr<decltype(Rule{} | r)>{};
    }
    template <typename R>
    friend constexpr auto operator|(R r, _wsr<Rule>)
    {
        return _wsr<decltype(r | Rule{})>{};
    }
};

template <typename Rule>
constexpr auto whitespace(Rule)
{
    return _wsr<Rule>{};
}
} // namespace lexyd

//=== no_whitespace ===//
namespace lexyd
{
template <typename Rule>
struct _wsn : _copy_base<Rule>
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Enable automatic whitespace skipping again.
            context.control_block->enable_whitespace_skipping = true;
            // And skip whitespace once.
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            // Note that this can't skip whitespace as there is no way to access the whitespace
            // rule.
            return rule.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            rule.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the rule with whitespace skipping disabled.
            context.control_block->enable_whitespace_skipping = false;
            return rule.template finish<_pc<NextParser>>(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<lexy::_detail::context_whitespace<Context>>)
            {
                // No whitespace, just parse the rule.
                return lexy::parser_for<Rule, NextParser>::parse(context, reader,
                                                                 LEXY_FWD(args)...);
            }
            else
            {
                // Parse the rule with whitespace skipping disabled.
                context.control_block->enable_whitespace_skipping = false;
                using parser = lexy::parser_for<Rule, _pc<NextParser>>;
                return parser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };
};

/// Disables automatic skipping of whitespace for all tokens of the given rule.
template <typename Rule>
constexpr auto no_whitespace(Rule)
{
    if constexpr (lexy::is_token_rule<Rule>)
        return Rule{}; // Token already behaves that way.
    else
        return _wsn<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_WHITESPACE_HPP_INCLUDED

