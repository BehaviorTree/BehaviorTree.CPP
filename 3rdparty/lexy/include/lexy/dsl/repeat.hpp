// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_REPEAT_HPP_INCLUDED
#define LEXY_DSL_REPEAT_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/parse_as.hpp>
#include <lexy/lexeme.hpp>

namespace lexyd
{
template <typename Item, typename Sep>
struct _rep_impl
{
    // Sink can either be present or not;
    // we only use variadic arguments to get rid of code duplication.
    template <typename Context, typename Reader, typename... Sink>
    static constexpr bool loop(Context& context, Reader& reader, std::size_t count, Sink&... sink)
    {
        using sink_parser
            = std::conditional_t<sizeof...(Sink) == 0, lexy::pattern_parser<>, lexy::sink_parser>;

        if (count == 0)
            return true;

        using item_parser = lexy::parser_for<Item, sink_parser>;
        if (!item_parser::parse(context, reader, sink...))
            return false;

        for (std::size_t i = 1; i != count; ++i)
        {
            using sep_parser = lexy::parser_for<typename Sep::rule, sink_parser>;
            if (!sep_parser::parse(context, reader, sink...))
                return false;

            if (!item_parser::parse(context, reader, sink...))
                return false;
        }

        using trailing_parser = lexy::parser_for<typename Sep::trailing_rule, sink_parser>;
        return trailing_parser::parse(context, reader, sink...);
    }
};
template <typename Item>
struct _rep_impl<Item, void>
{
    // Sink can either be present or not;
    // we only use variadic arguments to get rid of code duplication.
    template <typename Context, typename Reader, typename... Sink>
    static constexpr bool loop(Context& context, Reader& reader, std::size_t count, Sink&... sink)
    {
        using sink_parser
            = std::conditional_t<sizeof...(Sink) == 0, lexy::pattern_parser<>, lexy::sink_parser>;

        if (count == 0)
            return true;

        using item_parser = lexy::parser_for<Item, sink_parser>;
        if (!item_parser::parse(context, reader, sink...))
            return false;

        for (std::size_t i = 1; i != count; ++i)
        {
            if (!item_parser::parse(context, reader, sink...))
                return false;
        }

        return true;
    }
};

template <typename Item, typename Sep>
struct _repd : rule_base // repeat, discard
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, std::size_t count,
                                           Args&&... args)
        {
            if (!_rep_impl<Item, Sep>::loop(context, reader, count))
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep>
struct _repl : rule_base // repeat, list
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, std::size_t count,
                                           Args&&... args)
        {
            auto sink = context.value_callback().sink();
            if (!_rep_impl<Item, Sep>::loop(context, reader, count, sink))
                return false;

            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };
};
template <typename Item, typename Sep>
struct _repc : rule_base // repeat, capture
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, std::size_t count,
                                           Args&&... args)
        {
            auto begin = reader.position();
            if (!_rep_impl<Item, Sep>::loop(context, reader, count))
                return false;

            return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                     lexy::lexeme(reader, begin));
        }
    };
};

template <typename Count, typename Loop>
struct _rep : decltype(_maybe_branch(_pas<std::size_t, Count, true>{}, Loop{}))
{};

template <typename Count>
struct _rep_dsl
{
    template <typename Item>
    constexpr auto operator()(Item) const
    {
        return _rep<Count, _repd<Item, void>>{};
    }
    template <typename Item, typename Sep>
    constexpr auto operator()(Item, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _rep<Count, _repd<Item, Sep>>{};
    }

    template <typename Item>
    constexpr auto list(Item) const
    {
        return _rep<Count, _repl<Item, void>>{};
    }
    template <typename Item, typename Sep>
    constexpr auto list(Item, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _rep<Count, _repl<Item, Sep>>{};
    }

    template <typename Item>
    constexpr auto capture(Item) const
    {
        return _rep<Count, _repc<Item, void>>{};
    }
    template <typename Item, typename Sep>
    constexpr auto capture(Item, Sep) const
    {
        static_assert(lexy::is_separator<Sep>);
        return _rep<Count, _repc<Item, Sep>>{};
    }
};

/// Parses a rule `n` times, where `n` is the value produced by `Count`.
template <typename Count>
constexpr auto repeat(Count)
{
    return _rep_dsl<Count>{};
}
} // namespace lexyd

#endif // LEXY_DSL_REPEAT_HPP_INCLUDED

