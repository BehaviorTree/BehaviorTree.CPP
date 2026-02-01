// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_IDENTIFIER_HPP_INCLUDED
#define LEXY_DSL_IDENTIFIER_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/lexeme.hpp>

//=== identifier ===//
namespace lexy
{
/// Error when we matched a reserved.
struct reserved_identifier
{
    static LEXY_CONSTEVAL auto name()
    {
        return "reserved identifier";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Id, typename CharT, CharT... C>
struct _kw;
template <typename Literal, template <typename> typename CaseFolding>
struct _cfl;

template <typename Leading, typename Trailing>
struct _idp : token_base<_idp<Leading, Trailing>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);

            // Need to match Leading character.
            if (!lexy::try_match_token(Leading{}, reader))
                return false;

            // Match zero or more trailing characters.
            while (true)
            {
                if constexpr (lexy::_detail::is_swar_reader<Reader>)
                {
                    // If we have a swar reader, consume as much as possible at once.
                    while (Trailing{}.template char_class_match_swar<typename Reader::encoding>(
                        reader.peek_swar()))
                        reader.bump_swar();
                }

                if (!lexy::try_match_token(Trailing{}, reader))
                    break;
            }

            end = reader.current();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            Leading::template char_class_report_error<Reader>(context, reader.position());
        }
    };
};

template <typename Set>
struct _idrp // reserve predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        return lexy::try_match_token(Set{}, reader)
               && reader.peek() == decltype(reader)::encoding::eof();
    }
};
template <typename Set>
struct _idpp // reserve prefix predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        return lexy::try_match_token(Set{}, reader);
    }
};
template <typename Set>
struct _idcp // reserve contains predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        while (true)
        {
            if (lexy::try_match_token(Set{}, reader))
                return true;
            else if (reader.peek() == decltype(reader)::encoding::eof())
                return false;
            else
                reader.bump();
        }

        // unreachable
    }
};
template <typename Set>
struct _idsp // reserve suffix predicate
{
    template <typename Input>
    static constexpr bool is_reserved(const Input& input)
    {
        auto reader = input.reader();
        while (true)
        {
            if (lexy::try_match_token(Set{}, reader)
                && reader.peek() == decltype(reader)::encoding::eof())
                return true;
            else if (reader.peek() == decltype(reader)::encoding::eof())
                return false;
            else
                reader.bump();
        }

        // unreachable
    }
};

template <typename Leading, typename Trailing, typename... ReservedPredicate>
struct _id : branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Parse the pattern; this does not consume whitespace, so the range is accurate.
            auto begin = reader.position();
            if (!pattern().token_parse(context, reader))
                return false;
            auto end = reader.position();

            // Check for a reserved identifier.
            [[maybe_unused]] auto input = lexy::partial_input(reader, begin, end);
            if ((ReservedPredicate::is_reserved(input) || ...))
            {
                // It is reserved, report an error but trivially recover.
                auto err = lexy::error<Reader, lexy::reserved_identifier>(begin, end);
                context.on(_ev::error{}, err);
            }

            // Skip whitespace and continue with the value.
            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end));
        }
    };

    template <typename Reader>
    struct bp
    {
        typename Reader::marker end;

        constexpr bool try_parse(const void*, const Reader& reader)
        {
            // Parse the pattern.
            lexy::token_parser_for<decltype(pattern()), Reader> parser(reader);
            if (!parser.try_parse(reader))
                return false;
            end = parser.end;

            // We only succeed if it's not a reserved identifier.
            [[maybe_unused]] auto input
                = lexy::partial_input(reader, reader.position(), end.position());
            return !(ReservedPredicate::is_reserved(input) || ...);
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();

            context.on(_ev::token{}, lexy::identifier_token_kind, begin, end.position());
            reader.reset(end);

            using continuation = lexy::whitespace_parser<Context, NextParser>;
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       lexy::lexeme<Reader>(begin, end.position()));
        }
    };

    template <typename R>
    constexpr auto _make_reserve(R r) const
    {
        static_assert(lexy::is_literal_rule<R> || lexy::is_literal_set_rule<R>);
        return r;
    }
    template <typename Id, typename CharT, CharT... C>
    constexpr auto _make_reserve(_kw<Id, CharT, C...>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // No need to remember that it was a keyword originally.
        return _lit<CharT, C...>{};
    }
    template <typename Id, typename CharT, CharT... C, template <typename> typename CaseFolding>
    constexpr auto _make_reserve(_cfl<_kw<Id, CharT, C...>, CaseFolding>) const
    {
        static_assert(std::is_same_v<decltype(Id{}.pattern()), decltype(pattern())>,
                      "must not reserve keywords from another identifier");
        // No need to remember that it was a keyword originally.
        return _cfl<_lit<CharT, C...>, CaseFolding>{};
    }

    //=== dsl ===//
    /// Adds a set of reserved identifiers.
    template <typename... R>
    constexpr auto reserve(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idrp<decltype(set)>>{};
    }

    /// Reserves everything starting with the given rule.
    template <typename... R>
    constexpr auto reserve_prefix(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idpp<decltype(set)>>{};
    }

    /// Reservers everything containing the given rule.
    template <typename... R>
    constexpr auto reserve_containing(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idcp<decltype(set)>>{};
    }

    /// Reserves everything that ends with the given rule.
    template <typename... R>
    constexpr auto reserve_suffix(R... r) const
    {
        static_assert(sizeof...(R) > 0);
        auto set = (lexyd::literal_set() / ... / _make_reserve(r));
        return _id<Leading, Trailing, ReservedPredicate..., _idsp<decltype(set)>>{};
    }

    /// Matches every identifier, ignoring reserved ones.
    static constexpr auto pattern()
    {
        return _idp<Leading, Trailing>{};
    }

    /// Matches the initial char set of an identifier.
    constexpr auto leading_pattern() const
    {
        return Leading{};
    }

    /// Matches the trailing char set of an identifier.
    constexpr auto trailing_pattern() const
    {
        return Trailing{};
    }
};

/// Creates an identifier that consists of one or more of the given characters.
template <typename CharClass>
constexpr auto identifier(CharClass)
{
    static_assert(lexy::is_char_class_rule<CharClass>);
    return _id<CharClass, CharClass>{};
}

/// Creates an identifier that consists of one leading token followed by zero or more trailing
/// tokens.
template <typename LeadingClass, typename TrailingClass>
constexpr auto identifier(LeadingClass, TrailingClass)
{
    static_assert(lexy::is_char_class_rule<LeadingClass> //
                  && lexy::is_char_class_rule<TrailingClass>);
    return _id<LeadingClass, TrailingClass>{};
}
} // namespace lexyd

namespace lexy
{
template <typename Leading, typename Trailing>
constexpr auto token_kind_of<lexy::dsl::_idp<Leading, Trailing>> = lexy::identifier_token_kind;
} // namespace lexy

//=== keyword ===//
namespace lexyd
{
template <typename Id, typename CharT, CharT... C>
struct _kw : token_base<_kw<Id, CharT, C...>>, _lit_base
{
    static constexpr auto lit_max_char_count = sizeof...(C);

    // We must not end on a trailing character.
    static constexpr auto lit_char_classes
        = lexy::_detail::char_class_list<decltype(Id{}.trailing_pattern())>{};

    using lit_case_folding = void;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        typename Encoding::char_type result = 0;
        (void)((result = lexy::_detail::transcode_char<decltype(result)>(C), true) || ...);
        return result;
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos,
                                                 std::size_t char_class)
    {
        auto end                  = ((pos = trie.insert(pos, C)), ...);
        trie.node_char_class[end] = char_class;
        return end;
    }

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Need to match the literal.
            if (!lexy::_detail::match_literal<0, CharT, C...>(reader))
                return false;
            end = reader.current();

            // To qualify as a keyword, and not just the prefix of an identifier,
            // we must not have a trailing identifier character.
            return !lexy::try_match_token(Id{}.trailing_pattern(), reader);
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            using char_type    = typename Reader::encoding::char_type;
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<char_type>;

            // Match the entire identifier.
            auto begin = reader.position();
            lexy::try_match_token(Id{}.pattern(), reader);
            auto end = reader.position();

            auto err = lexy::error<Reader, lexy::expected_keyword>(begin, end, str, sizeof...(C));
            context.on(_ev::error{}, err);
        }
    };
};

template <typename Id>
struct _keyword;
template <typename L, typename T, typename... R>
struct _keyword<_id<L, T, R...>>
{
    template <typename CharT, CharT... C>
    using get = _kw<_id<L, T>, CharT, C...>;
};

#if LEXY_HAS_NTTP
template <lexy::_detail::string_literal Str, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...>)
{
    return lexy::_detail::to_type_string<_keyword<_id<L, T>>::template get, Str>{};
}
#else
template <auto C, typename L, typename T, typename... R>
constexpr auto keyword(_id<L, T, R...>)
{
    return _kw<_id<L, T>, LEXY_DECAY_DECLTYPE(C), C>{};
}
#endif

#define LEXY_KEYWORD(Str, Id)                                                                      \
    LEXY_NTTP_STRING(::lexyd::_keyword<LEXY_DECAY_DECLTYPE(Id)>::template get, Str) {}
} // namespace lexyd

namespace lexy
{
template <typename Id, typename CharT, CharT... C>
constexpr auto token_kind_of<lexy::dsl::_kw<Id, CharT, C...>> = lexy::literal_token_kind;
} // namespace lexy

#endif // LEXY_DSL_IDENTIFIER_HPP_INCLUDED

