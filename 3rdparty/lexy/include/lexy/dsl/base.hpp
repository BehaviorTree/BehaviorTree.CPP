// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BASE_HPP_INCLUDED
#define LEXY_DSL_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/lazy_init.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/base.hpp>

//=== parse_events ===//
namespace lexy::parse_events
{
/// Parsing started.
/// Arguments: position
struct grammar_start
{};
/// Parsing finished succesfully.
/// Arguments: the reader at the final parse position.
struct grammar_finish
{};
/// Parsing finished unsuccesfully.
/// Arguments: the reader at the final parse position.
struct grammar_cancel
{};

/// Start of the current production.
/// Arguments: position
struct production_start
{};
/// End of the current production.
/// Arguments: position
struct production_finish
{};
/// Production is canceled.
/// Arguments: position
struct production_cancel
{};

/// Start of a chain of left-associative operations.
/// Arguments: position
/// Returns: a handle that needs to be passed to finish.
struct operation_chain_start
{};
/// Operation inside a chain.
/// Arguments: operation, position
struct operation_chain_op
{};
/// End of a chain of operations.
/// Arguments: handle, position
struct operation_chain_finish
{};

/// A token was consumed.
/// Arguments: kind, begin, end
struct token
{};

/// The input backtracked from end to begin.
/// Only meaningful for begin != end.
/// Arguments: begin, end
struct backtracked
{};

/// A parse error occurrs.
/// Arguments: error object
struct error
{};

/// Non-trivial error recovery started,
/// i.e. it is currently discarding input.
/// Arguments: position
struct recovery_start
{};
/// Non-trivial error recovery succeeded.
/// It will now continue with normal parsing.
/// Arguments: position
struct recovery_finish
{};
/// Non-trivial error recovery failed because it reaches the limit.
/// It will now cancel until the next recovery point.
/// Arguments: position
struct recovery_cancel
{};
} // namespace lexy::parse_events

namespace lexyd
{
namespace _ev = lexy::parse_events;

// Does not copy token tags.
template <typename Rule>
auto _copy_base_impl()
{
    if constexpr (lexy::is_unconditional_branch_rule<Rule>)
        return unconditional_branch_base{};
    else if constexpr (lexy::is_branch_rule<Rule>)
        return branch_base{};
    else
        return rule_base{};
}
template <typename Rule>
using _copy_base = decltype(_copy_base_impl<Rule>());
} // namespace lexyd

//=== parser ===//
#define LEXY_PARSER_FUNC LEXY_FORCE_INLINE constexpr

namespace lexy
{
template <typename Rule, typename NextParser>
using parser_for = typename Rule::template p<NextParser>;

template <typename BranchRule, typename Reader>
using branch_parser_for = typename BranchRule::template bp<Reader>;

template <typename Production, typename Reader>
struct _pb : lexy::branch_parser_for<lexy::production_rule<Production>, Reader>
{};
// We create a new type here to shorten its name.
template <typename Production, typename Reader>
using production_branch_parser = _pb<Production, Reader>;

/// A branch parser that takes a branch unconditionally and forwards to the regular parser.
template <typename Rule, typename Reader>
struct unconditional_branch_parser
{
    constexpr std::true_type try_parse(const void*, const Reader&)
    {
        return {};
    }

    template <typename Context>
    constexpr void cancel(Context&)
    {}

    template <typename NextParser, typename Context, typename... Args>
    LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
    {
        return parser_for<Rule, NextParser>::parse(context, reader, LEXY_FWD(args)...);
    }
};

/// A branch parser that parses a branch rule but with a special continuation.
template <typename BranchRule, typename Reader, template <typename> typename Continuation>
struct continuation_branch_parser
{
    branch_parser_for<BranchRule, Reader> impl;

    template <typename ControlBlock>
    constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
    {
        return impl.try_parse(cb, reader);
    }

    template <typename Context>
    constexpr void cancel(Context& context)
    {
        impl.cancel(context);
    }

    template <typename NextParser, typename Context, typename... Args>
    LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
    {
        return impl.template finish<Continuation<NextParser>>(context, reader, LEXY_FWD(args)...);
    }
};

/// A parser that does not support any arguments.
template <typename... PrevArgs>
struct pattern_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static std::true_type parse(Context&, Reader&, const PrevArgs&..., Args&&...)
    {
        // A rule is used inside a loop or similar situation, where it must not produce values, but
        // it did.
        static_assert(sizeof...(Args) == 0, "pattern rule must not produce any values");
        return {};
    }
};

/// A parser that forwards all arguments to a sink, which is the first argument.
struct sink_parser
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_PARSER_FUNC static std::true_type parse(Context&, Reader&, Sink& sink, Args&&... args)
    {
        if constexpr (sizeof...(Args) > 0)
            sink(LEXY_FWD(args)...);

        return {};
    }
};

/// A parser that finishes a sink and continues with the next one.
template <typename NextParser>
struct sink_finish_parser
{
    template <typename Context, typename Reader, typename Sink, typename... Args>
    LEXY_PARSER_FUNC static auto parse(Context& context, Reader& reader, Sink& sink, Args&&... args)
    {
        if constexpr (std::is_same_v<typename Sink::return_type, void>)
        {
            LEXY_MOV(sink).finish();
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
        else
        {
            return NextParser::parse(context, reader, LEXY_FWD(args)..., LEXY_MOV(sink).finish());
        }
    }
};
} // namespace lexy

//=== whitespace ===//
namespace lexy::_detail
{
template <typename NextParser>
struct automatic_ws_parser;
} // namespace lexy::_detail

namespace lexy
{
template <typename Context, typename NextParser,
          typename = lexy::production_whitespace<typename Context::production,
                                                 typename Context::whitespace_production>>
struct whitespace_parser : _detail::automatic_ws_parser<NextParser>
{};
// If we know the whitespace rule is void, go to NextParser immediately.
// This is both an optimization and also doesn't require inclusion of whitespace.hpp.
template <typename Context, typename NextParser>
struct whitespace_parser<Context, NextParser, void> : NextParser
{};
} // namespace lexy

//=== token parser ===//
namespace lexy
{
template <typename TokenRule, typename Reader>
using token_parser_for = typename TokenRule::template tp<Reader>;

template <typename TokenRule, typename Reader>
LEXY_FORCE_INLINE constexpr auto try_match_token(TokenRule, Reader& reader)
{
    lexy::token_parser_for<TokenRule, Reader> parser(reader);

    using try_parse_result = decltype(parser.try_parse(reader));
    if constexpr (std::is_same_v<try_parse_result, std::true_type>)
    {
        parser.try_parse(reader);
        reader.reset(parser.end);
        return std::true_type{};
    }
    else if constexpr (std::is_same_v<try_parse_result, std::false_type>)
    {
        // try_parse() is pure and we don't want to advance the reader, so no need to call it.
        return std::false_type{};
    }
    else
    {
        if (!parser.try_parse(reader))
            return false;

        reader.reset(parser.end);
        return true;
    }
}
} // namespace lexy

#endif // LEXY_DSL_BASE_HPP_INCLUDED

