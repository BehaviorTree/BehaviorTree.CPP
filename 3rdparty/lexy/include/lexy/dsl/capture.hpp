// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CAPTURE_HPP_INCLUDED
#define LEXY_DSL_CAPTURE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
template <typename Token>
struct _cap : _copy_base<Token>
{
    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<Token, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, Token{}, begin, end.position());
            reader.reset(end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end.position()));
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            if (!Token::token_parse(context, reader))
                return false;
            auto end = reader.position();

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }
    };
};

template <typename Rule>
struct _capr : _copy_base<Rule>
{
    template <typename NextParser, typename... PrevArgs>
    struct _pc : lexy::_detail::disable_whitespace_skipping
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           PrevArgs&&... prev_args, typename Reader::iterator begin,
                                           Args&&... args)
        {
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(prev_args)...,
                                       lexy::lexeme(reader, begin), LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Rule, Reader> rule;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
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
            // Forward to the rule, but remember the current reader position.
            using continuation = _pc<NextParser, Args...>;
            return rule.template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                      reader.position());
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Forward to the rule, but remember the current reader position.
            using parser = lexy::parser_for<Rule, _pc<NextParser, Args...>>;
            return parser::parse(context, reader, LEXY_FWD(args)..., reader.position());
        }
    };
};

template <typename Production>
struct _prd;

/// Captures whatever the token matches as a lexeme; does not include trailing whitespace.
template <typename Token>
constexpr auto capture(Token)
{
    static_assert(lexy::is_token_rule<Token>);
    return _cap<Token>{};
}

/// Captures whatever the token production matches as lexeme; does not include trailing whitespace.
template <typename Production>
constexpr auto capture(_prd<Production>)
{
    static_assert(lexy::is_token_production<Production>);
    return _capr<_prd<Production>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CAPTURE_HPP_INCLUDED

