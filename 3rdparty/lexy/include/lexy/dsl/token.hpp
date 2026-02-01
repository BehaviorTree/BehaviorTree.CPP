// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_TOKEN_HPP_INCLUDED
#define LEXY_DSL_TOKEN_HPP_INCLUDED

#include <lexy/action/match.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct missing_token
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing token";
    }
};
} // namespace lexy

//=== token_base ===//
namespace lexyd
{
template <typename Tag, typename Token>
struct _toke;
template <auto Kind, typename Token>
struct _tokk;

template <typename ImplOrTag, bool IsImpl = lexy::is_token_rule<ImplOrTag>>
struct _token_inherit;
template <typename Impl>
struct _token_inherit<Impl, true> : Impl // no need to inherit from _token_base
{};
template <typename Tag>
struct _token_inherit<Tag, false> : _token_base, Tag // need to turn it into a token
{};

// ImplOrTag is either a branch base that indicates whether the token is an unconditional branch,
// or an actual token rule whose implementation will be used.
template <typename Derived, typename ImplOrTag = branch_base>
struct token_base : _token_inherit<ImplOrTag>
{
    using token_type = Derived;

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<Derived, Reader> parser(reader);
            auto                                    result = parser.try_parse(reader);
            end                                            = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::token{}, Derived{}, reader.position(), end.position());
            reader.reset(end);
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename Context, typename Reader>
    LEXY_PARSER_FUNC static bool token_parse(Context& context, Reader& reader)
    {
        auto                                    begin = reader.position();
        lexy::token_parser_for<Derived, Reader> parser(reader);

        using try_parse_result = decltype(parser.try_parse(reader));
        if constexpr (std::is_same_v<try_parse_result, std::true_type>)
        {
            parser.try_parse(reader);
        }
        else
        {
            if (!parser.try_parse(reader))
            {
                context.on(_ev::token{}, lexy::error_token_kind, reader.position(),
                           parser.end.position());
                parser.report_error(context, reader);
                reader.reset(parser.end);

                return false;
            }
        }

        context.on(_ev::token{}, typename Derived::token_type{}, begin, parser.end.position());
        reader.reset(parser.end);

        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (!token_parse(context, reader))
                return false;
            else
                return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                           LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    template <typename Tag>
    static constexpr _toke<Tag, Derived> error = _toke<Tag, Derived>{};

    template <auto Kind>
    static constexpr _tokk<Kind, Derived> kind = _tokk<Kind, Derived>{};
};

// We forward all implementation to Token.
// We cannot directly inherit from Token as that wouldn't override the token_type.
template <auto Kind, typename Token>
struct _tokk : token_base<_tokk<Kind, Token>, Token>
{};

template <typename Tag, typename Token>
struct _toke : token_base<_toke<Tag, Token>, Token>
{
    // If we're overriding the error for a char class rule, we also want to change its error
    // reporting. Otherwise, rules such as `dsl::delimited()` building on char classes will generate
    // the "wrong" error.
    //
    // If it's not a char class, adding this function doesn't hurt.
    template <typename Reader, typename Context>
    static constexpr void char_class_report_error(Context&                  context,
                                                  typename Reader::iterator position)
    {
        auto err = lexy::error<Reader, Tag>(position, position);
        context.on(_ev::error{}, err);
    }

    template <typename Reader>
    struct tp : lexy::token_parser_for<Token, Reader>
    {
        constexpr explicit tp(const Reader& reader) : lexy::token_parser_for<Token, Reader>(reader)
        {}

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            // Report a different error.
            auto err = lexy::error<Reader, Tag>(reader.position(), this->end.position());
            context.on(_ev::error{}, err);
        }
    };
};
} // namespace lexyd

namespace lexy
{
template <auto Kind, typename Token>
constexpr auto token_kind_of<lexy::dsl::_tokk<Kind, Token>> = Kind;
template <typename Tag, typename Token>
constexpr auto token_kind_of<lexy::dsl::_toke<Tag, Token>> = token_kind_of<Token>;
} // namespace lexy

//=== token rule ===//
namespace lexyd
{
template <typename Rule>
struct _token : token_base<_token<Rule>>
{
    struct _production
    {
        static constexpr auto name                = "<token>";
        static constexpr auto max_recursion_depth = 0;
        static constexpr auto rule                = Rule{};
    };

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // We match a dummy production that only consists of the rule.
            auto success = lexy::do_action<
                _production,
                lexy::match_action<void, Reader>::template result_type>(lexy::_mh(),
                                                                        lexy::no_parse_state,
                                                                        reader);
            end = reader.current();
            return success;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::missing_token>(reader.position(), end.position());
            context.on(_ev::error{}, err);
        }
    };
};

/// Turns the arbitrary rule into a token by matching it without producing any values.
template <typename Rule>
constexpr auto token(Rule)
{
    if constexpr (lexy::is_token_rule<Rule>)
        return Rule{};
    else
        return _token<Rule>{};
}
} // namespace lexyd

#endif // LEXY_DSL_TOKEN_HPP_INCLUDED

