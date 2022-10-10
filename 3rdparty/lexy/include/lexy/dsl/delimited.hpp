// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_DELIMITED_HPP_INCLUDED
#define LEXY_DSL_DELIMITED_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/symbol.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace lexy
{
/// The reader ends before the closing delimiter was found.
struct missing_delimiter
{
    static LEXY_CONSTEVAL auto name()
    {
        return "missing delimiter";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename CharClass, typename Reader>
struct _del_chars
{
    typename Reader::iterator begin;

    constexpr _del_chars(const Reader& reader) : begin(reader.position()) {}

    template <typename Context>
    constexpr void _recover(Context& context, typename Reader::iterator recover_begin,
                            typename Reader::iterator recover_end)
    {
        CharClass::template char_class_report_error<Reader>(context, recover_begin);

        // We recovery by discarding the ASCII character.
        // (We've checked for EOF before, so that's not the error.)
        context.on(_ev::recovery_start{}, recover_begin);
        context.on(_ev::token{}, lexy::error_token_kind, recover_begin, recover_end);
        context.on(_ev::recovery_finish{}, recover_end);

        // Restart the next character here.
        begin = recover_end;
    }

    template <typename Context, typename Sink>
    constexpr void parse(Context& context, Reader& reader, Sink& sink)
    {
        // First try to match the ASCII characters.
        using matcher = lexy::_detail::ascii_set_matcher<_cas<CharClass>>;
        if (matcher::template match<typename Reader::encoding>(reader.peek()))
        {
            reader.bump();
        }
        else if constexpr (!std::is_same_v<decltype(CharClass::char_class_match_cp(char32_t())),
                                           std::false_type>)
        {
            // Try to match any code point in default_encoding or byte_encoding.
            if constexpr (std::is_same_v<typename Reader::encoding, lexy::default_encoding> //
                          || std::is_same_v<typename Reader::encoding, lexy::byte_encoding>)
            {
                static_assert(!CharClass::char_class_unicode(),
                              "cannot use this character class with default/byte_encoding");
                LEXY_ASSERT(reader.peek() != Reader::encoding::eof(),
                            "EOF should be checked before calling this");

                auto recover_begin = reader.position();
                auto cp            = static_cast<char32_t>(reader.peek());
                reader.bump();

                if (!CharClass::char_class_match_cp(cp))
                {
                    finish(context, sink, recover_begin);
                    _recover(context, recover_begin, reader.position());
                }
            }
            // Otherwise, try to match Unicode characters.
            else
            {
                static_assert(CharClass::char_class_unicode(),
                              "cannot use this character class with Unicode encoding");

                auto result = lexy::_detail::parse_code_point(reader);
                if (result.error == lexy::_detail::cp_error::success
                    && CharClass::char_class_match_cp(result.cp))
                {
                    reader.set_position(result.end);
                }
                else
                {
                    finish(context, sink, reader.position());
                    _recover(context, reader.position(), result.end);
                    reader.set_position(result.end);
                }
            }
        }
        // It doesn't match Unicode characters.
        else
        {
            // We can just discard the invalid ASCII character.
            LEXY_ASSERT(reader.peek() != Reader::encoding::eof(),
                        "EOF should be checked before calling this");
            auto recover_begin = reader.position();
            reader.bump();
            auto recover_end = reader.position();

            finish(context, sink, recover_begin);
            _recover(context, recover_begin, recover_end);
        }
    }

    template <typename Context, typename Sink>
    constexpr void finish(Context& context, Sink& sink, typename Reader::iterator end)
    {
        if (begin == end)
            return;

        context.on(_ev::token{}, typename CharClass::token_type{}, begin, end);
        sink(lexy::lexeme<Reader>(begin, end));
    }
};

template <typename Token, typename Error = lexy::missing_delimiter>
struct _del_limit
{
    using error = Error;

    template <typename Reader>
    static constexpr bool peek(Reader reader)
    {
        return lexy::try_match_token(Token{}, reader) || reader.peek() == Reader::encoding::eof();
    }
};
template <typename Error>
struct _del_limit<void, Error>
{
    using error = Error;

    template <typename Reader>
    static constexpr bool peek(Reader reader)
    {
        return reader.peek() == Reader::encoding::eof();
    }
};

template <typename Close, typename Char, typename Limit, typename... Escapes>
struct _del : rule_base
{
    using _limit = std::conditional_t<std::is_void_v<Limit> || lexy::is_token_rule<Limit>,
                                      _del_limit<Limit>, Limit>;

    template <typename CloseParser, typename Context, typename Reader, typename Sink>
    static constexpr bool _loop(CloseParser& close, Context& context, Reader& reader, Sink& sink)
    {
        auto                     del_begin = reader.position();
        _del_chars<Char, Reader> cur_chars(reader);
        while (!close.try_parse(context.control_block, reader))
        {
            close.cancel(context);

            // Check for missing delimiter.
            if (_limit::peek(reader))
            {
                // We're done, so finish the current characters.
                auto end = reader.position();
                cur_chars.finish(context, sink, end);

                auto err = lexy::error<Reader, typename _limit::error>(del_begin, end);
                context.on(_ev::error{}, err);
                return false;
            }

            // Check for escape sequences.
            if ((Escapes::_try_parse(context, reader, sink, cur_chars) || ...))
                // We had an escape sequence, so do nothing in this iteration.
                continue;

            // Parse the next character.
            cur_chars.parse(context, reader, sink);
        }

        // Finish the currently active character sequence.
        cur_chars.finish(context, sink, reader.position());
        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto sink = context.value_callback().sink();

            // Parse characters until we have the closing delimiter.
            lexy::branch_parser_for<Close, Reader> close{};
            if (!_loop(close, context, reader, sink))
                return false;

            // We're done, finish the sink and then the closing delimiter.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return close.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return close.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                         LEXY_MOV(sink).finish());
            }
        }
    };
};

struct _escape_base
{};

template <typename Open, typename Close, typename Limit = void>
struct _delim_dsl
{
    /// Add literal tokens that will limit the delimited to detect a missing terminator.
    template <typename LimitCharClass>
    constexpr auto limit(LimitCharClass) const
    {
        static_assert(std::is_void_v<Limit> && lexy::is_char_class_rule<LimitCharClass>);

        return _delim_dsl<Open, Close, LimitCharClass>{};
    }
    /// Add literal tokens that will limit the delimited and specify the error.
    template <typename Error, typename LimitCharClass>
    constexpr auto limit(LimitCharClass) const
    {
        static_assert(std::is_void_v<Limit> && lexy::is_char_class_rule<LimitCharClass>);
        return _delim_dsl<Open, Close, _del_limit<LimitCharClass, Error>>{};
    }

    //=== rules ===//
    /// Sets the content.
    template <typename Char, typename... Escapes>
    constexpr auto operator()(Char, Escapes...) const
    {
        static_assert(lexy::is_char_class_rule<Char>);
        static_assert((std::is_base_of_v<_escape_base, Escapes> && ...));
        return no_whitespace(open() >> _del<Close, Char, Limit, Escapes...>{});
    }

    //=== access ===//
    /// Matches the open delimiter.
    constexpr auto open() const
    {
        return Open{};
    }
    /// Matches the closing delimiter.
    constexpr auto close() const
    {
        // Close never has any whitespace.
        return Close{};
    }
};

/// Parses everything between the two delimiters and captures it.
template <typename Open, typename Close>
constexpr auto delimited(Open, Close)
{
    static_assert(lexy::is_branch_rule<Open> && lexy::is_branch_rule<Close>);
    return _delim_dsl<Open, Close>{};
}

/// Parses everything between a paired delimiter.
template <typename Delim>
constexpr auto delimited(Delim)
{
    static_assert(lexy::is_branch_rule<Delim>);
    return _delim_dsl<Delim, Delim>{};
}

constexpr auto quoted        = delimited(LEXY_LIT("\""));
constexpr auto triple_quoted = delimited(LEXY_LIT("\"\"\""));

constexpr auto single_quoted = delimited(LEXY_LIT("'"));

constexpr auto backticked        = delimited(LEXY_LIT("`"));
constexpr auto double_backticked = delimited(LEXY_LIT("``"));
constexpr auto triple_backticked = delimited(LEXY_LIT("```"));
} // namespace lexyd

namespace lexy
{
struct invalid_escape_sequence
{
    static LEXY_CONSTEVAL auto name()
    {
        return "invalid escape sequence";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Escape, typename... Branches>
struct _escape : _escape_base
{
    template <typename Context, typename Reader, typename Sink, typename Char>
    static constexpr bool _try_parse(Context& context, Reader& reader, Sink& sink,
                                     _del_chars<Char, Reader>& cur_chars)
    {
        auto begin = reader.position();

        // Check whether we're having the initial escape character.
        lexy::branch_parser_for<Escape, Reader> token{};
        if (!token.try_parse(context.control_block, reader))
            // No need to call `.cancel()`; it's a token.
            return false;

        // We do, so finish current character sequence and consume the escape token.
        cur_chars.finish(context, sink, begin);
        // It's a token, so this can't fail.
        token.template finish<lexy::pattern_parser<>>(context, reader);

        // Try to parse the correct branch.
        auto try_parse_branch = [&](auto branch) {
            lexy::branch_parser_for<decltype(branch), Reader> parser{};
            if (!parser.try_parse(context.control_block, reader))
            {
                parser.cancel(context);
                return false;
            }

            // This might fail, but we don't care:
            // it will definitely consume the escape token, and everything that is a valid prefix.
            // The remaining stuff is then just treated as part of the delimited.
            parser.template finish<lexy::sink_parser>(context, reader, sink);
            return true;
        };
        auto found = (try_parse_branch(Branches{}) || ...);

        if constexpr ((lexy::is_unconditional_branch_rule<Branches> || ...))
        {
            LEXY_ASSERT(found, "there is an unconditional branch");
        }
        else if (!found)
        {
            // We haven't found any branch of the escape sequence.
            auto err = lexy::error<Reader, lexy::invalid_escape_sequence>(begin, reader.position());
            context.on(_ev::error{}, err);
        }

        // Restart the current character sequence after the escape sequence.
        cur_chars.begin = reader.position();
        return true;
    }

    /// Adds a generic escape rule.
    template <typename Branch>
    constexpr auto rule(Branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return _escape<Escape, Branches..., Branch>{};
    }

    /// Adds an escape rule that captures the branch.
    template <typename Branch>
    constexpr auto capture(Branch branch) const
    {
        static_assert(lexy::is_branch_rule<Branch>);
        return this->rule(lexy::dsl::capture(branch));
    }

    /// Adds an escape rule that parses the symbol.
    template <const auto& Table, typename Rule>
    constexpr auto symbol(Rule rule) const
    {
        return this->rule(lexyd::symbol<Table>(rule));
    }
    template <const auto& Table>
    constexpr auto symbol() const
    {
        return this->rule(lexyd::symbol<Table>);
    }
};

/// Creates an escape rule.
/// The token is the initial rule to begin,
/// and then you can add rules that match after it.
template <typename EscapeToken>
constexpr auto escape(EscapeToken)
{
    static_assert(lexy::is_token_rule<EscapeToken>);
    return _escape<EscapeToken>{};
}

constexpr auto backslash_escape = escape(lit_c<'\\'>);
constexpr auto dollar_escape    = escape(lit_c<'$'>);
} // namespace lexyd

#endif // LEXY_DSL_DELIMITED_HPP_INCLUDED

