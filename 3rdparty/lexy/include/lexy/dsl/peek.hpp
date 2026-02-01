// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PEEK_HPP_INCLUDED
#define LEXY_DSL_PEEK_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexy
{
/// We've failed to match a peek.
struct peek_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "peek failure";
    }
};

/// We've failed to match a peek not.
struct unexpected
{
    static LEXY_CONSTEVAL auto name()
    {
        return "unexpected";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Rule, typename Tag>
struct _peek : branch_base
{
    template <typename Reader>
    struct bp
    {
        typename Reader::iterator begin;
        typename Reader::marker   end;

        constexpr bool try_parse(const void*, Reader reader)
        {
            // We need to match the entire rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            begin       = reader.position();
            auto result = parser.try_parse(reader);
            end         = parser.end;

            return result;
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            context.on(_ev::backtracked{}, begin, end.position());
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::backtracked{}, begin, end.position());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::peek_failure>;
                auto err  = lexy::error<Reader, tag>(impl.begin, impl.end.position());
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            context.on(_ev::backtracked{}, impl.begin, impl.end.position());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _peek<Rule, Error> error = {};
};

template <typename Rule, typename Tag>
struct _peekn : branch_base
{
    template <typename Reader>
    struct bp
    {
        typename Reader::iterator begin;
        typename Reader::marker   end;

        constexpr bool try_parse(const void*, Reader reader)
        {
            // We must not match the rule.
            lexy::token_parser_for<decltype(lexy::dsl::token(Rule{})), Reader> parser(reader);

            begin       = reader.position();
            auto result = !parser.try_parse(reader);
            end         = parser.end;

            return result;
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            context.on(_ev::backtracked{}, begin, end.position());
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::backtracked{}, begin, end.position());
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::unexpected>;
                auto err  = lexy::error<Reader, tag>(impl.begin, impl.end.position());
                context.on(_ev::error{}, err);

                // And recover by consuming the input.
                context.on(_ev::recovery_start{}, impl.begin);
                context.on(_ev::token{}, lexy::error_token_kind, impl.begin, impl.end.position());
                context.on(_ev::recovery_finish{}, impl.end.position());

                reader.reset(impl.end);
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                context.on(_ev::backtracked{}, impl.begin, impl.end.position());
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        }
    };

    template <typename Error>
    static constexpr _peekn<Rule, Error> error = {};
};

/// Check if at this reader position, the rule would match, but don't actually consume any
/// characters if it does.
template <typename Rule>
constexpr auto peek(Rule)
{
    return _peek<Rule, void>{};
}

/// Checks if at this reader position, the rule would not match.
template <typename Rule>
constexpr auto peek_not(Rule)
{
    return _peekn<Rule, void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_PEEK_HPP_INCLUDED

