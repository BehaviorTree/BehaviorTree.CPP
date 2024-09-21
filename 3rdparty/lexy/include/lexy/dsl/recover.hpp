// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_RECOVER_HPP_INCLUDED
#define LEXY_DSL_RECOVER_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/lookahead.hpp>

namespace lexyd
{
// Indicates that this rule already generates the recovery events.
struct _recovery_base : rule_base
{};

// Parses the rule but generates the appropriate recovery_start/finish/cancel events.
template <typename Rule>
struct _recovery_wrapper : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        struct _continuation
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                               bool& recovery_finished, Args&&... args)
            {
                recovery_finished = true;
                context.on(_ev::recovery_finish{}, reader.position());
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            }
        };

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::recovery_start{}, reader.position());
            auto recovery_finished = false;

            // As part of the recovery, we parse the rule and whitespace.
            using parser = lexy::parser_for<Rule, lexy::whitespace_parser<Context, _continuation>>;
            auto result  = parser::parse(context, reader, recovery_finished, LEXY_FWD(args)...);

            if (!recovery_finished)
                context.on(_ev::recovery_cancel{}, reader.position());
            return result;
        }
    };
};

struct _noop_recovery : rule_base
{
    template <typename NextParser>
    using p = NextParser;
};
} // namespace lexyd

namespace lexyd
{
template <typename Token, typename Limit>
struct _find : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Note that we can't use lookahead() directly as it's end position includes Needle/End,
            // here we want to exclude it, however.
            constexpr const auto& trie
                = _look_trie<typename Reader::encoding, Token, decltype(get_limit())>;
            using matcher = lexy::_detail::lit_trie_matcher<trie, 0>;

            auto begin = reader.position();
            context.on(_ev::recovery_start{}, begin);
            while (true)
            {
                auto end    = reader.current(); // *before* we've consumed Token/Limit
                auto result = matcher::try_match(reader);
                if (result == 0)
                {
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end.position());
                    context.on(_ev::recovery_finish{}, end.position());
                    reader.reset(end); // reset to before the token
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                }
                else if (result == 1 || reader.peek() == Reader::encoding::eof())
                {
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end.position());
                    context.on(_ev::recovery_cancel{}, end.position());
                    reader.reset(end); // reset to before the limit
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            // unreachable
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting literal tokens is found first.
    template <typename... Literals>
    constexpr auto limit(Literals... literals) const
    {
        static_assert(sizeof...(Literals) > 0);

        auto l = (get_limit() / ... / literals);
        return _find<Token, decltype(l)>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return literal_set();
        else
            return Limit{};
    }
};

/// Recovers once it finds one of the given literal tokens (without consuming them).
template <typename... Literals>
constexpr auto find(Literals... literals)
{
    static_assert(sizeof...(Literals) > 0);

    auto needle = (literal_set() / ... / literals);
    return _find<decltype(needle), void>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Limit, typename... R>
struct _reco : _recovery_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::recovery_start{}, begin);

            // Try to match one of the recovery rules.
            lexy::branch_parser_for<decltype((R{} | ...)), Reader> recovery{};
            while (!recovery.try_parse(context.control_block, reader))
            {
                recovery.cancel(context);

                if (lexy::token_parser_for<decltype(get_limit()), Reader> limit(reader);
                    limit.try_parse(reader) || reader.peek() == Reader::encoding::eof())
                {
                    // We've failed to recover as we've reached the limit.
                    auto end = reader.position();
                    context.on(_ev::token{}, lexy::error_token_kind, begin, end);
                    context.on(_ev::recovery_cancel{}, end);
                    return false;
                }
                else
                {
                    // Try again.
                    reader.bump();
                }
            }

            auto end = reader.position();
            context.on(_ev::token{}, lexy::error_token_kind, begin, end);
            context.on(_ev::recovery_finish{}, end);

            // Finish with the rule that matched.
            return recovery.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Fail error recovery if limiting literal tokens is found first.
    template <typename... Literals>
    constexpr auto limit(Literals... literals) const
    {
        static_assert(sizeof...(Literals) > 0);

        auto l = (get_limit() / ... / literals);
        return _reco<decltype(l), R...>{};
    }

    static constexpr auto get_limit()
    {
        if constexpr (std::is_void_v<Limit>)
            return literal_set();
        else
            return Limit{};
    }
};

/// Discards input until one of the branches matches to recover from an error.
template <typename... Branches>
constexpr auto recover(Branches...)
{
    static_assert(sizeof...(Branches) > 0);
    LEXY_REQUIRE_BRANCH_RULE(Branches..., "recover");
    return _reco<void, Branches...>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Terminator, typename Rule, typename Recover>
struct _tryt : rule_base
{
    template <typename NextParser>
    struct _pc
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           bool& continuation_reached, Args&&... args)
        {
            continuation_reached = true;

            // We need to parse the terminator on success as well, if we have one.
            if constexpr (std::is_void_v<Terminator>)
                return NextParser::parse(context, reader, LEXY_FWD(args)...);
            else
                return lexy::parser_for<Terminator, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool recover(Context& context, Reader& reader, Args&&... args)
        {
            if constexpr (std::is_void_v<Recover>)
            {
                using recovery_rule = _recovery_wrapper<_noop_recovery>;
                return lexy::parser_for<recovery_rule, NextParser>::parse(context, reader,
                                                                          LEXY_FWD(args)...);
            }
            else if constexpr (std::is_base_of_v<_recovery_base, Recover>)
            {
                using recovery_rule = Recover;
                return lexy::parser_for<recovery_rule, NextParser>::parse(context, reader,
                                                                          LEXY_FWD(args)...);
            }
            else
            {
                using recovery_rule = _recovery_wrapper<Recover>;
                return lexy::parser_for<recovery_rule, NextParser>::parse(context, reader,
                                                                          LEXY_FWD(args)...);
            }
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using parser = lexy::parser_for<Rule, _pc<NextParser>>;

            // Parse the rule and check whether it reached the continuation.
            auto continuation_reached = false;
            auto result = parser::parse(context, reader, continuation_reached, LEXY_FWD(args)...);
            if (continuation_reached)
                // Whatever happened, it is not our problem as we've reached the continuation.
                return result;

            // We haven't reached the continuation, so need to recover.
            LEXY_ASSERT(!result, "we've failed without reaching the continuation?!");
            return _pc<NextParser>::recover(context, reader, LEXY_FWD(args)...);
        }
    };
};

template <typename Rule, typename Recover>
struct _tryr : _copy_base<Rule>
{
    using impl = _tryt<void, Rule, Recover>;

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
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Finish the rule and check whether it reached the continuation.
            using continuation        = typename impl::template _pc<NextParser>;
            auto continuation_reached = false;
            auto result = rule.template finish<continuation>(context, reader, continuation_reached,
                                                             LEXY_FWD(args)...);
            if (continuation_reached)
                // Whatever happened, it is not our problem as we've reached the continuation.
                return result;

            // We haven't reached the continuation, so need to recover.
            LEXY_ASSERT(!result, "we've failed without reaching the continuation?!");
            return continuation::recover(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p : lexy::parser_for<impl, NextParser>
    {};
};

/// Parses Rule, if that fails, continues immediately.
template <typename Rule>
constexpr auto try_(Rule)
{
    return _tryr<Rule, void>{};
}

/// Parses Rule, if that fails, parses recovery rule.
template <typename Rule, typename Recover>
constexpr auto try_(Rule, Recover)
{
    return _tryr<Rule, Recover>{};
}
} // namespace lexyd

#endif // LEXY_DSL_RECOVER_HPP_INCLUDED

