// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_FOLLOW_HPP_INCLUDED
#define LEXY_DSL_FOLLOW_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/literal.hpp>

namespace lexy
{
struct follow_restriction
{
    static LEXY_CONSTEVAL auto name()
    {
        return "follow restriction";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Literal, typename CharClass>
struct _nf : token_base<_nf<Literal, CharClass>>, _lit_base
{
    static constexpr auto lit_max_char_count = Literal::lit_max_char_count;

    static constexpr auto lit_char_classes = lexy::_detail::char_class_list<CharClass>{};

    using lit_case_folding = typename Literal::lit_case_folding;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        return Literal::template lit_first_char<Encoding>();
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos,
                                                 std::size_t char_class)
    {
        auto end                  = Literal::lit_insert(trie, pos, char_class);
        trie.node_char_class[end] = char_class;
        return end;
    }

    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Literal, Reader> impl;
        typename Reader::marker                 end;
        bool                                    literal_success;

        constexpr explicit tp(const Reader& reader)
        : impl(reader), end(reader.current()), literal_success(false)
        {}

        constexpr bool try_parse(Reader reader)
        {
            literal_success = false;

            // Need to match the literal.
            if (!impl.try_parse(reader))
                return false;
            end             = impl.end;
            literal_success = true;

            // To match, we must not match the char class now.
            reader.reset(end);
            if constexpr (std::is_void_v<lit_case_folding>)
            {
                return !lexy::try_match_token(CharClass{}, reader);
            }
            else
            {
                typename lit_case_folding::template reader<Reader> case_folded{reader};
                return !lexy::try_match_token(CharClass{}, case_folded);
            }
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            if (!literal_success)
            {
                impl.report_error(context, reader);
            }
            else
            {
                auto err
                    = lexy::error<Reader, lexy::follow_restriction>(end.position(), end.position());
                context.on(_ev::error{}, err);
            }
        }
    };
};

/// Match a literal but only if not followed by the given char class.
template <typename Literal, typename CharClass>
constexpr auto not_followed_by(Literal, CharClass cc)
{
    static_assert(lexy::is_literal_rule<Literal> && Literal::lit_char_classes.size == 0);
    return _nf<Literal, decltype(_make_char_class(cc))>{};
}

/// Match a literal but only if followed by the given char class.
template <typename Literal, typename CharClass>
constexpr auto followed_by(Literal lit, CharClass cc)
{
    return not_followed_by(lit, -cc);
}
} // namespace lexyd

namespace lexy
{
template <typename Literal, typename CharClass>
constexpr auto token_kind_of<lexy::dsl::_nf<Literal, CharClass>> = lexy::literal_token_kind;
} // namespace lexy

#endif // LEXY_DSL_FOLLOW_HPP_INCLUDED

