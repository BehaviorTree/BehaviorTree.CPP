// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/identifier.hpp>

namespace lexy
{
struct different_identifier
{
    static LEXY_CONSTEVAL auto name()
    {
        return "different identifier";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Id, typename Reader>
using _ctx_id = lexy::_detail::parse_context_var<Id, lexy::lexeme<Reader>>;

template <typename Id>
struct _ctx_icreate : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _ctx_id<Id, Reader> var({});
            var.link(context);
            auto result = NextParser::parse(context, reader, LEXY_FWD(args)...);
            var.unlink(context);
            return result;
        }
    };
};

template <typename Id, typename Identifier>
struct _ctx_icap : branch_base
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // The last argument will be a lexeme.
            _ctx_id<Id, Reader>::get(context.control_block) = (args, ...);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
    template <typename Reader>
    using bp = lexy::continuation_branch_parser<Identifier, Reader, _pc>;
    template <typename NextParser>
    using p = lexy::parser_for<Identifier, _pc<NextParser>>;
};

template <typename Id, typename Identifier, typename Tag>
struct _ctx_irem : branch_base
{
    // We only need the pattern:
    // We don't want a value and don't need to check for reserved identifiers,
    // as it needs to match a previously parsed identifier, which wasn't reserved.
    using _pattern = decltype(Identifier{}.pattern());

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader& reader)
        {
            // Parse the pattern.
            lexy::token_parser_for<_pattern, Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // The two lexemes need to be equal.
            auto lexeme = lexy::lexeme<Reader>(reader.position(), end.position());
            return lexy::_detail::equal_lexemes(_ctx_id<Id, Reader>::get(cb), lexeme);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish parsing the token.
            context.on(_ev::token{}, lexy::identifier_token_kind, reader.position(),
                       end.position());
            reader.reset(end);
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename... PrevArgs>
        struct _cont
        {
            template <typename Context, typename Reader>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, PrevArgs&&... args,
                                               lexy::lexeme<Reader> lexeme)
            {
                if (!lexy::_detail::equal_lexemes(_ctx_id<Id, Reader>::get(context.control_block),
                                                  lexeme))
                {
                    // The lexemes weren't equal.
                    using tag = lexy::_detail::type_or<Tag, lexy::different_identifier>;
                    auto err  = lexy::error<Reader, tag>(lexeme.begin(), lexeme.end());
                    context.on(_ev::error{}, err);

                    // But we can trivially recover.
                }

                // Continue parsing with the symbol value.
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Capture the pattern and continue with special continuation.
            return lexy::parser_for<_cap<_pattern>, _cont<Args...>>::parse(context, reader,
                                                                           LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _ctx_irem<Id, Identifier, Error> error = {};
};
} // namespace lexyd

namespace lexyd
{
template <typename Id, typename Identifier>
struct _ctx_id_dsl
{
    constexpr auto create() const
    {
        return _ctx_icreate<Id>{};
    }

    constexpr auto capture() const
    {
        return _ctx_icap<Id, Identifier>{};
    }

    constexpr auto rematch() const
    {
        return _ctx_irem<Id, Identifier, void>{};
    }
};

/// Declares a context variable that stores one instance of the given identifier.
template <typename Id, typename Leading, typename Trailing, typename... Reserved>
constexpr auto context_identifier(_id<Leading, Trailing, Reserved...>)
{
    return _ctx_id_dsl<Id, _id<Leading, Trailing, Reserved...>>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CONTEXT_IDENTIFIER_HPP_INCLUDED

