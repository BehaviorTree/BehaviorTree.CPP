// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_TIMES_HPP_INCLUDED
#define LEXY_DSL_TIMES_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/sequence.hpp>

namespace lexyd
{
template <std::size_t N, typename Rule, typename Sep>
struct _times : rule_base
{
    template <std::size_t I = N>
    static constexpr auto _repeated_rule()
    {
        if constexpr (I == 1)
        {
            if constexpr (std::is_same_v<Sep, void>)
                return Rule{};
            else
                return Rule{} + typename Sep::trailing_rule{};
        }
        else
        {
            if constexpr (std::is_same_v<Sep, void>)
                return Rule{} + _repeated_rule<I - 1>();
            else
                return Rule{} + typename Sep::rule{} + _repeated_rule<I - 1>();
        }
    }

    template <typename NextParser>
    using p = lexy::parser_for<decltype(_repeated_rule()), NextParser>;
};

/// Repeats the rule N times in sequence.
template <std::size_t N, typename Rule>
constexpr auto times(Rule)
{
    static_assert(N > 0);
    return _times<N, Rule, void>{};
}

/// Repeates the rule N times in sequence separated by a separator.
template <std::size_t N, typename Rule, typename Sep>
constexpr auto times(Rule, Sep)
{
    static_assert(N > 0);
    static_assert(lexy::is_separator<Sep>);
    return _times<N, Rule, Sep>{};
}

template <typename Rule>
constexpr auto twice(Rule rule)
{
    return times<2>(rule);
}
template <typename Rule, typename Sep>
constexpr auto twice(Rule rule, Sep sep)
{
    return times<2>(rule, sep);
}
} // namespace lexyd

#endif // LEXY_DSL_TIMES_HPP_INCLUDED

