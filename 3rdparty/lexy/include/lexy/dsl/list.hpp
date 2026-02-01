// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LIST_HPP_INCLUDED
#define LEXY_DSL_LIST_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/separator.hpp>

namespace lexyd
{
template <typename Item, typename Sep>
struct _lst : _copy_base<Item>
{
    template <typename Context, typename Reader, typename Sink>
    LEXY_PARSER_FUNC static bool _loop(Context& context, Reader& reader, Sink& sink)
    {
        while (true)
        {
            // Parse a separator if necessary.
            [[maybe_unused]] auto sep_begin = reader.position();
            if constexpr (!std::is_void_v<Sep>)
            {
                lexy::branch_parser_for<typename Sep::rule, Reader> sep{};
                if (!sep.try_parse(context.control_block, reader))
                {
                    // We didn't have a separator, list is definitely finished.
                    sep.cancel(context);
                    break;
                }

                if (!sep.template finish<lexy::sink_parser>(context, reader, sink))
                    return false;
            }
            [[maybe_unused]] auto sep_end = reader.position();

            // Parse the next item.
            if constexpr (lexy::is_branch_rule<Item>)
            {
                // It's a branch, so try parsing it to detect loop exit.
                lexy::branch_parser_for<Item, Reader> item{};
                if (!item.try_parse(context.control_block, reader))
                {
                    // We don't have a next item, exit the loop.
                    // If necessary, we report a trailing separator.
                    item.cancel(context);
                    if constexpr (!std::is_void_v<Sep>)
                        Sep::report_trailing_error(context, reader, sep_begin, sep_end);
                    break;
                }

                // We're having an item, finish it.
                if (!item.template finish<lexy::sink_parser>(context, reader, sink))
                    return false;
            }
            else
            {
                // Not a branch, so we need one item.
                if (!lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                    return false;
            }
        }

        return true;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Construct the sink.
            auto sink = context.value_callback().sink();

            // Parse the first item.
            if (!lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                return false;

            // Parse the remaining items.
            if (!_loop(context, reader, sink))
                return false;

            // We're done with the list, finish the sink and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Item, Reader> item;

        template <typename ControlBlock>
        constexpr bool try_parse(const ControlBlock* cb, const Reader& reader)
        {
            // We parse a list if we can parse its first item.
            return item.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            return item.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // At this point, we have a list so construct a sink.
            auto sink = context.value_callback().sink();

            // Finish the first item, passing all values to the sink.
            if (!item.template finish<lexy::sink_parser>(context, reader, sink))
                return false;

            // Parse the remaining items.
            if (!_loop(context, reader, sink))
                return false;

            // We're done with the list, finish the sink and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };
};

/// Parses a list of items without a separator.
template <typename Item>
constexpr auto list(Item)
{
    LEXY_REQUIRE_BRANCH_RULE(Item, "list() without a separator");
    return _lst<Item, void>{};
}

/// Parses a list of items with the specified separator.
template <typename Item, typename Sep, typename Tag>
constexpr auto list(Item, _sep<Sep, Tag>)
{
    return _lst<Item, _sep<Sep, Tag>>{};
}

/// Parses a list of items with the specified separator that can be trailing.
template <typename Item, typename Sep>
constexpr auto list(Item, _tsep<Sep>)
{
    LEXY_REQUIRE_BRANCH_RULE(Item, "list() with a trailing separator");
    return _lst<Item, _tsep<Sep>>{};
}

template <typename Item, typename Sep>
constexpr auto list(Item, _isep<Sep>)
{
    static_assert(lexy::_detail::error<Item, Sep>,
                  "list() does not support `dsl::ignore_trailing_sep()`");
    return _lst<Item, void>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Term, typename Item, typename Sep, typename Recover>
struct _lstt : rule_base
{
    // We're using an enum together with a switch to compensate a lack of goto in constexpr.
    // The simple state machine goes as follows on well-formed input:
    // terminator -> separator -> separator_trailing_check -> item -> terminator -> ... ->
    // done
    //
    // The interesting case is error recovery.
    // There we skip over characters until we either found the terminator, separator or
    // item. We then set the enum to jump to the appropriate state of the state machine.
    enum class _state
    {
        terminator,
        separator,
        separator_trailing_check,
        item,
        recovery,
    };

    template <typename TermParser, typename Context, typename Reader, typename Sink>
    LEXY_PARSER_FUNC static bool _loop(_state initial_state, TermParser& term, Context& context,
                                       Reader& reader, Sink& sink)
    {
        auto state = initial_state;

        [[maybe_unused]] auto sep_pos = reader.position();
        while (true)
        {
            switch (state)
            {
            case _state::terminator:
                if (term.try_parse(context.control_block, reader))
                    // We had the terminator, so the list is done.
                    return true;
                term.cancel(context);

                // Parse the following list separator next.
                state = _state::separator;
                break;

            case _state::separator:
                if constexpr (!std::is_void_v<Sep>)
                {
                    sep_pos = reader.position();
                    if (lexy::parser_for<typename Sep::rule, lexy::sink_parser>::parse(context,
                                                                                       reader,
                                                                                       sink))
                    {
                        // Check for a trailing separator next.
                        state = _state::separator_trailing_check;
                        break;
                    }
                    else if (sep_pos == reader.position())
                    {
                        // We don't have a separator at all.
                        // Assume it's missing and parse an item instead.

                        if constexpr (lexy::is_branch_rule<Item>)
                        {
                            lexy::branch_parser_for<Item, Reader> item{};
                            if (item.try_parse(context.control_block, reader)
                                && item.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Continue after an item has been parsed.
                                state = _state::terminator;
                                break;
                            }
                            else
                            {
                                // Not an item, recover.
                                item.cancel(context);
                                state = _state::recovery;
                                break;
                            }
                        }
                        else
                        {
                            // We cannot try and parse an item.
                            // To avoid generating wrong errors, immediately recover.
                            state = _state::recovery;
                            break;
                        }
                    }
                    else
                    {
                        // We did have something that looked like a separator initially, but
                        // wasn't one on closer inspection. Enter generic recovery as we've
                        // already consumed input. (If we ignore the case where the item and
                        // separator share a common prefix, we know it wasn't the start of an
                        // item so can't just pretend that there is one).
                        state = _state::recovery;
                        break;
                    }
                }
                else
                {
                    // List doesn't have a separator; immediately parse item next.
                    state = _state::item;
                    break;
                }

            case _state::separator_trailing_check:
                if constexpr (!std::is_void_v<Sep>)
                {
                    // We need to check whether we're having a trailing separator by checking
                    // for a terminating one.
                    if (term.try_parse(context.control_block, reader))
                    {
                        // We had the terminator, so the list is done.
                        // Report a trailing separator error if necessary.
                        Sep::report_trailing_error(context, reader, sep_pos, reader.position());
                        return true;
                    }
                    else
                    {
                        // We didn't have a separator, parse item next.
                        state = _state::item;
                        break;
                    }
                }
                break;

            case _state::item:
                if (lexy::parser_for<Item, lexy::sink_parser>::parse(context, reader, sink))
                {
                    // Loop back.
                    state = _state::terminator;
                    break;
                }
                else
                {
                    // Recover from missing item.
                    state = _state::recovery;
                    break;
                }

            case _state::recovery: {
                auto recovery_begin = reader.position();
                context.on(_ev::recovery_start{}, recovery_begin);
                while (true)
                {
                    // Recovery succeeds when we reach the next separator.
                    if constexpr (!std::is_void_v<Sep>)
                    {
                        sep_pos = reader.position();

                        lexy::branch_parser_for<typename Sep::rule, Reader> sep{};
                        if (sep.try_parse(context.control_block, reader))
                        {
                            auto recovery_end = reader.position();
                            context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                       recovery_end);
                            context.on(_ev::recovery_finish{}, recovery_end);

                            if (sep.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Continue the list with the trailing separator check.
                                state = _state::separator_trailing_check;
                                break;
                            }
                            else
                            {
                                // Need to recover from this.
                                state = _state::recovery;
                                break;
                            }
                        }
                        else
                        {
                            sep.cancel(context);
                        }
                    }
                    // When we don't have a separator, but the item is a branch, we also succeed
                    // when we reach the next item.
                    //
                    // Note that we're doing this check only if we don't have a separator.
                    // If we do have one, the heuristic "end of the invalid item" is better than
                    // "beginning of the next one".
                    else if constexpr (lexy::is_branch_rule<Item>)
                    {
                        lexy::branch_parser_for<Item, Reader> item{};
                        if (item.try_parse(context.control_block, reader))
                        {
                            auto recovery_end = reader.position();
                            context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                       recovery_end);
                            context.on(_ev::recovery_finish{}, recovery_end);

                            if (item.template finish<lexy::sink_parser>(context, reader, sink))
                            {
                                // Continue the list with the next terminator check.
                                state = _state::terminator;
                                break;
                            }
                            else
                            {
                                // Need to recover from this.
                                state = _state::recovery;
                                break;
                            }
                        }
                        else
                        {
                            item.cancel(context);
                        }
                    }

                    // At this point, we couldn't detect the next item.
                    // Recovery succeeds when we reach the terminator.
                    if (term.try_parse(context.control_block, reader))
                    {
                        // We're now done with the entire list.
                        auto recovery_end = reader.position();
                        context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                   recovery_end);
                        context.on(_ev::recovery_finish{}, recovery_end);
                        return true;
                    }
                    else
                    {
                        term.cancel(context);
                    }

                    // At this point, we couldn't detect the next item or a terminator.
                    // Recovery fails when we reach the limit.
                    using limit_rule = decltype(Recover{}.get_limit());
                    if (lexy::token_parser_for<limit_rule, Reader> limit(reader);
                        limit.try_parse(reader) || reader.peek() == Reader::encoding::eof())
                    {
                        // Recovery has failed, propagate error.
                        auto recovery_end = reader.position();
                        context.on(_ev::token{}, lexy::error_token_kind, recovery_begin,
                                   recovery_end);
                        context.on(_ev::recovery_cancel{}, recovery_end);
                        return false;
                    }

                    // Consume one code unit and try again.
                    reader.bump();
                }
                break;
            }
            }
        }

        return false; // unreachable
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Term, Reader> term{};
            auto                                  sink = context.value_callback().sink();

            // Parse initial item.
            using item_parser = lexy::parser_for<Item, lexy::sink_parser>;
            auto result       = item_parser::parse(context, reader, sink);

            // Parse the remaining items.
            if (!_loop(result ? _state::terminator : _state::recovery, term, context, reader, sink))
                return false;

            // At this point, we just need to finish parsing the terminator.
            if constexpr (std::is_same_v<typename decltype(sink)::return_type, void>)
            {
                LEXY_MOV(sink).finish();
                return term.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            }
            else
            {
                return term.template finish<NextParser>(context, reader, LEXY_FWD(args)...,
                                                        LEXY_MOV(sink).finish());
            }
        }
    };
};
} // namespace lexyd

#endif // LEXY_DSL_LIST_HPP_INCLUDED

