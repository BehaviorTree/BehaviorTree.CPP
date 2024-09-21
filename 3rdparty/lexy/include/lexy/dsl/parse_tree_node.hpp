// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PARSE_TREE_NODE_HPP_INCLUDED
#define LEXY_DSL_PARSE_TREE_NODE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

#if !LEXY_EXPERIMENTAL
#    error "lexy::dsl::tnode/pnode are experimental"
#endif

//=== impl ===//
namespace lexyd
{
template <typename Derived>
struct _n;

template <typename Derived, typename R>
struct _nr : branch_base
{
    template <typename NextParser>
    struct _cont
    {
        template <typename Context, typename ChildReader, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, ChildReader& child_reader,
                                           bool& rule_succeded, Reader& reader, Args&&... args)
        {
            rule_succeded = true;

            if (child_reader.peek() != ChildReader::encoding::eof())
            {
                auto begin = child_reader.position();
                auto end   = reader.position();
                context.on(_ev::token{}, lexy::error_token_kind, begin, end);

                auto err = lexy::error<Reader, typename Derived::node_end_error>(begin, end);
                context.on(_ev::error{}, err);
            }

            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename NextParser, typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool _parse_rule(Context& context, Reader& reader,
                                             typename Reader::marker end, Args&&... args)
    {
        auto child_reader = Derived::node_child_reader(reader);
        reader.reset(end);

        using rule_parser
            = lexy::whitespace_parser<Context, lexy::parser_for<R, _cont<NextParser>>>;
        if (auto rule_succeded = false;
            rule_parser::parse(context, child_reader, rule_succeded, reader, LEXY_FWD(args)...))
        {
            return true;
        }
        else
        {
            if (!rule_succeded)
                // Report an error token for the child span that wasn't able to be parsed.
                context.on(_ev::token{}, lexy::error_token_kind, child_reader.position(),
                           end.position());
            return false;
        }
    }

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr bool try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<_n<Derived>, Reader> parser(reader);
            auto                                        result = parser.try_parse(reader);
            end                                                = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            return _parse_rule<NextParser>(context, reader, end, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::token_parser_for<_n<Derived>, Reader> parser(reader);
            if (!parser.try_parse(reader))
            {
                LEXY_ASSERT(parser.end.position() == reader.position(), "impl should be LL(1)");
                parser.report_error(context, reader);
                return false;
            }

            return _parse_rule<NextParser>(context, reader, parser.end, LEXY_FWD(args)...);
        }
    };
};

template <typename Derived>
struct _n : token_base<Derived>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr auto try_parse(Reader reader)
        {
            if constexpr (lexy::is_node_encoding<typename Reader::encoding>)
            {
                if (!Reader::encoding::match(reader.peek(), Derived::node_kind()))
                    return false;

                reader.bump();
                end = reader.current();
                return true;
            }
            else
            {
                // This happens when it is used as whitespace, which is inherited while parsing the
                // token lexeme, we don't match anything in that case.
                return std::false_type{};
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            constexpr auto name = Derived::node_kind_name();

            auto err = lexy::error<Reader, lexy::expected_char_class>(reader.position(), name);
            context.on(_ev::error{}, err);
        }
    };

    template <typename Rule>
    constexpr auto operator()(Rule) const
    {
        return _nr<Derived, Rule>{};
    }
};
} // namespace lexyd

//=== dsl::tnode ===//
namespace lexy
{
struct expected_token_end
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected token end";
    }
};
} // namespace lexy

namespace lexyd
{
template <auto Kind>
struct _tn : _n<_tn<Kind>>
{
    static LEXY_CONSTEVAL auto node_kind()
    {
        return Kind;
    }

    static LEXY_CONSTEVAL auto node_kind_name()
    {
        using lexy::token_kind_name;
        return token_kind_name(Kind);
    }

    using node_end_error = lexy::expected_token_end;

    template <typename Reader>
    static constexpr auto node_child_reader(Reader& reader)
    {
        return reader.lexeme_reader();
    }
};

template <auto Kind>
constexpr auto tnode = _tn<Kind>{};
} // namespace lexyd

namespace lexy
{
template <auto Kind>
constexpr auto token_kind_of<lexy::dsl::_tn<Kind>> = Kind;
} // namespace lexy

//=== dsl::pnode ===//
namespace lexy
{
struct expected_production_end
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected production end";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Production>
struct _pn : _n<_pn<Production>>
{
    static_assert(lexy::is_production<Production>);

    static LEXY_CONSTEVAL auto node_kind()
    {
        return Production{};
    }

    static LEXY_CONSTEVAL auto node_kind_name()
    {
        return lexy::production_name<Production>();
    }

    using node_end_error = lexy::expected_production_end;

    template <typename Reader>
    static constexpr auto node_child_reader(Reader& reader)
    {
        return reader.child_reader();
    }
};

template <typename Production>
constexpr auto pnode = _pn<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PARSE_TREE_NODE_HPP_INCLUDED

