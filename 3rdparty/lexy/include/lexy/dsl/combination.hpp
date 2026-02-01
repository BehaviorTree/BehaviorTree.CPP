// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_COMBINATION_HPP_INCLUDED
#define LEXY_DSL_COMBINATION_HPP_INCLUDED

#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexy
{
struct combination_duplicate
{
    static LEXY_CONSTEVAL auto name()
    {
        return "combination duplicate";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Sink>
struct _comb_control
{
    // The sink to store values of the item.
    Sink& sink;
    // Whether or not the state has already been handled.
    const bool* handled;
    // Write the index of the item in here.
    std::size_t idx = 0;
    // Whether or not we should break.
    bool loop_break = false;
};

// Final rule for one item in the combination.
template <std::size_t Idx>
struct _comb_it : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename Sink, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context&, Reader&, _comb_control<Sink>& ctrl,
                                           Args&&... args)
        {
            ctrl.idx = Idx;
            if constexpr (sizeof...(Args) > 0)
            {
                if (!ctrl.handled[Idx])
                    // Only call the sink if it is not a duplicate.
                    ctrl.sink(LEXY_FWD(args)...);
            }
            return true;
        }
    };
};

template <typename DuplicateError, typename ElseRule, typename... R>
struct _comb : rule_base
{
    template <std::size_t... Idx>
    static auto _comb_choice_(lexy::_detail::index_sequence<Idx...>)
    {
        if constexpr (std::is_void_v<ElseRule>)
            return ((R{} >> _comb_it<Idx>{}) | ...);
        else
            return ((R{} >> _comb_it<Idx>{}) | ... | ElseRule{});
    }
    using _comb_choice = decltype(_comb_choice_(lexy::_detail::index_sequence_for<R...>{}));

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto N = sizeof...(R);

            auto                          sink       = context.value_callback().sink();
            bool                          handled[N] = {};
            _comb_control<decltype(sink)> control{sink, handled};

            // Parse all iterations of the choice.
            for (auto count = 0; count < int(N); ++count)
            {
                auto begin = reader.position();

                using parser
                    = lexy::parser_for<_comb_choice, lexy::pattern_parser<decltype(control)>>;
                if (!parser::parse(context, reader, control))
                    return false;
                else if (control.loop_break)
                    break; // Partial combination and we're done.

                if (handled[control.idx])
                {
                    using tag = lexy::_detail::type_or<DuplicateError, lexy::combination_duplicate>;
                    auto err  = lexy::error<Reader, tag>(begin, reader.position());
                    context.on(_ev::error{}, err);
                    // We can trivially recover, but need to do another iteration.
                    --count;
                }
                else
                {
                    handled[control.idx] = true;
                }
            }

            // Obtain the final result and continue.
            return lexy::sink_finish_parser<NextParser>::parse(context, reader, sink,
                                                               LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    template <typename Tag>
    static constexpr _comb<Tag, ElseRule, R...> duplicate_error = {};

    template <typename Tag>
    static constexpr _comb<DuplicateError, _err<Tag, void>, R...> missing_error = {};
};

/// Matches each of the rules in an arbitrary order.
/// Only matches each rule exactly once.
template <typename... R>
constexpr auto combination(R...)
{
    LEXY_REQUIRE_BRANCH_RULE(R..., "combination()");
    static_assert((!lexy::is_unconditional_branch_rule<R> && ...),
                  "combination() does not support unconditional branches");
    return _comb<void, void, R...>{};
}

/// Matches some of the rules in an arbitrary order.
/// Only matches a rule at most once.
template <typename... R>
constexpr auto partial_combination(R...)
{
    LEXY_REQUIRE_BRANCH_RULE(R..., "partial_combination()");
    static_assert((!lexy::is_unconditional_branch_rule<R> && ...),
                  "partial_combination() does not support unconditional branches");
    // If the choice no longer matches, we just break.
    return _comb<void, decltype(break_), R...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_COMBINATION_HPP_INCLUDED

