// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SEQUENCE_HPP_INCLUDED
#define LEXY_DSL_SEQUENCE_HPP_INCLUDED

#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename... R>
struct _seq_impl;
template <>
struct _seq_impl<>
{
    template <typename NextParser>
    using p = NextParser;
};
template <typename R1>
struct _seq_impl<R1>
{
    template <typename NextParser>
    using p = lexy::parser_for<R1, NextParser>;
};
template <typename R1, typename R2>
struct _seq_impl<R1, R2>
{
    template <typename NextParser>
    using p = lexy::parser_for<R1, lexy::parser_for<R2, NextParser>>;
};
template <typename R1, typename R2, typename R3>
struct _seq_impl<R1, R2, R3>
{
    template <typename NextParser>
    using p = lexy::parser_for<R1, lexy::parser_for<R2, lexy::parser_for<R3, NextParser>>>;
};
template <typename R1, typename R2, typename R3, typename R4>
struct _seq_impl<R1, R2, R3, R4>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1, lexy::parser_for<R2, lexy::parser_for<R3, lexy::parser_for<R4, NextParser>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5>
struct _seq_impl<R1, R2, R3, R4, R5>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1, lexy::parser_for<
                R2, lexy::parser_for<R3, lexy::parser_for<R4, lexy::parser_for<R5, NextParser>>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5, typename R6>
struct _seq_impl<R1, R2, R3, R4, R5, R6>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1,
        lexy::parser_for<
            R2,
            lexy::parser_for<
                R3, lexy::parser_for<R4, lexy::parser_for<R5, lexy::parser_for<R6, NextParser>>>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5, typename R6, typename R7>
struct _seq_impl<R1, R2, R3, R4, R5, R6, R7>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1,
        lexy::parser_for<
            R2, lexy::parser_for<
                    R3, lexy::parser_for<
                            R4, lexy::parser_for<
                                    R5, lexy::parser_for<R6, lexy::parser_for<R7, NextParser>>>>>>>;
};
template <typename R1, typename R2, typename R3, typename R4, typename R5, typename R6, typename R7,
          typename... T>
struct _seq_impl<R1, R2, R3, R4, R5, R6, R7, T...>
{
    template <typename NextParser>
    using p = lexy::parser_for<
        R1,
        lexy::parser_for<
            R2,
            lexy::parser_for<
                R3,
                lexy::parser_for<
                    R4, lexy::parser_for<
                            R5, lexy::parser_for<
                                    R6, lexy::parser_for<R7, lexy::parser_for<_seq_impl<T...>,
                                                                              NextParser>>>>>>>>;
};

template <typename... R>
struct _seq : rule_base
{
    static_assert(sizeof...(R) > 1);

    template <typename NextParser>
    using p = lexy::parser_for<_seq_impl<R...>, NextParser>;
};

template <typename R, typename S, typename = std::enable_if_t<lexy::is_rule<R> && lexy::is_rule<S>>>
constexpr auto operator+(R, S)
{
    return _seq<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator+(_seq<R...>, S)
{
    return _seq<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator+(R, _seq<S...>)
{
    return _seq<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator+(_seq<R...>, _seq<S...>)
{
    return _seq<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_SEQUENCE_HPP_INCLUDED

