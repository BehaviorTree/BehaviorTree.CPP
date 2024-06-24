// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_WHITESPACE_HPP_INCLUDED
#define LEXY_DSL_WHITESPACE_HPP_INCLUDED

#include <lexy/_detail/swar.hpp>
#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Rule>
struct _wsr;
}

//=== implementation ===//
namespace lexy::_detail
{
// Parse this production to skip whitespace.
template <typename WhitespaceRule>
struct ws_production
{
    static constexpr auto name                = "<whitespace>";
    static constexpr auto max_recursion_depth = 0;
    static constexpr auto rule = lexy::dsl::loop(WhitespaceRule{} | lexy::dsl::break_);
};
// If the whitespace rule itself uses `dsl::whitespace`, strip it to avoid infinite recursion.
template <typename Rule>
struct ws_production<lexyd::_wsr<Rule>> : ws_production<Rule>
{};

// A special handler for parsing whitespace.
// It only forwards error events and ignores all others.
template <typename Handler>
class ws_handler
{
public:
    constexpr explicit ws_handler(Handler& handler, typename Handler::event_handler& evh)
    : _handler(&handler), _event_handler(&evh)
    {}
    template <typename Context>
    constexpr explicit ws_handler(Context& context)
    : ws_handler(context.control_block->parse_handler, context.handler)
    {}

    class event_handler
    {
    public:
        template <typename Rule>
        constexpr event_handler(ws_production<Rule>)
        {}
        template <typename Production>
        constexpr event_handler(Production)
        {
            static_assert(_detail::error<Production>,
                          "whitespace rule must not contain `dsl::p` or `dsl::recurse`;"
                          "use `dsl::inline_` instead");
        }

        template <typename Error>
        constexpr void on(ws_handler& handler, parse_events::error ev, const Error& error)
        {
            handler._event_handler->on(*handler._handler, ev, error);
        }

        template <typename Event, typename... Args>
        constexpr int on(ws_handler&, Event, const Args&...)
        {
            return 0; // an operation_start event returns something
        }
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename>
    constexpr bool get_result(bool rule_parse_result) &&
    {
        return rule_parse_result;
    }

    template <typename Event, typename... Args>
    constexpr auto real_on(Event ev, Args&&... args)
    {
        return _event_handler->on(*_handler, ev, LEXY_FWD(args)...);
    }

private:
    Handler*                         _handler;
    typename Handler::event_handler* _event_handler;
};
template <typename Context>
ws_handler(Context& context) -> ws_handler<typename Context::handler_type>;

template <typename>
using ws_result = bool;

template <typename WhitespaceRule>
constexpr bool space_is_definitely_whitespace()
{
    if constexpr (lexy::is_char_class_rule<WhitespaceRule>)
        return WhitespaceRule::char_class_ascii().contains[int(' ')];
    else
        return false;
}

template <typename WhitespaceRule, typename Handler, typename Reader>
constexpr auto skip_whitespace(ws_handler<Handler>&& handler, Reader& reader)
{
    auto begin = reader.position();

    if constexpr (lexy::is_token_rule<WhitespaceRule>)
    {
        // Parsing a token repeatedly cannot fail, so we can optimize it.

        if constexpr (_detail::is_swar_reader<Reader> //
                      && space_is_definitely_whitespace<WhitespaceRule>())
        {
            while (true)
            {
                // Skip as many spaces as possible.
                using char_type = typename Reader::encoding::char_type;
                while (reader.peek_swar() == _detail::swar_fill(char_type(' ')))
                    reader.bump_swar();

                // We no longer have a space, skip the entire whitespace rule once.
                if (!lexy::try_match_token(WhitespaceRule{}, reader))
                    // If that fails, we definitely have no more whitespace.
                    break;
            }
        }
        else
        {
            // Without SWAR, we just repeatedly skip the whitespace rule.
            while (lexy::try_match_token(WhitespaceRule{}, reader))
            {
            }
        }

        handler.real_on(lexy::parse_events::token{}, lexy::whitespace_token_kind, begin,
                        reader.position());
        return std::true_type{};
    }
    else if constexpr (!std::is_void_v<WhitespaceRule>)
    {
        using production = ws_production<WhitespaceRule>;

        // Parse the production using a special handler that only forwards errors.
        auto result = lexy::do_action<production, ws_result>(LEXY_MOV(handler),
                                                             lexy::no_parse_state, reader);

        handler.real_on(lexy::parse_events::token{},
                        result ? lexy::whitespace_token_kind : lexy::error_token_kind, begin,
                        reader.position());
        return result;
    }
    else
    {
        (void)handler;
        (void)reader;
        (void)begin;
        return std::true_type{};
    }
}

// Inherit from it in a continuation to disable automatic whitespace skipping.
struct disable_whitespace_skipping
{};

template <typename NextParser>
struct automatic_ws_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
    {
        if (!std::is_base_of_v<disable_whitespace_skipping, NextParser> //
            && context.control_block->enable_whitespace_skipping)
        {
            using whitespace = lexy::production_whitespace<typename Context::production,
                                                           typename Context::whitespace_production>;
            if (!skip_whitespace<whitespace>(ws_handler(context), reader))
                return false;
        }

        return NextParser::parse(context, reader, LEXY_FWD(args)...);
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
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        constexpr static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto result
                = lexy::_detail::skip_whitespace<Rule>(lexy::_detail::ws_handler(context), reader);
            if (!result)
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

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
            // rule. We thus don't need to disable anything.
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
            using whitespace = lexy::production_whitespace<typename Context::production,
                                                           typename Context::whitespace_production>;
            if constexpr (std::is_void_v<whitespace>)
            {
                // No whitespace, just parse the rule normally.
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

