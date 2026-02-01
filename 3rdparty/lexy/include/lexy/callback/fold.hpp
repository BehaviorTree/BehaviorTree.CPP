// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_FOLD_HPP_INCLUDED
#define LEXY_CALLBACK_FOLD_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
template <bool Inplace>
struct _fold_sfinae;
template <>
struct _fold_sfinae<true>
{
    template <typename Op, typename T, typename... Args>
    using type = decltype(void(
        _detail::invoke(LEXY_DECLVAL(Op), LEXY_DECLVAL(T&), LEXY_DECLVAL(Args)...)));
};
template <>
struct _fold_sfinae<false>
{
    template <typename Op, typename T, typename... Args>
    using type = decltype(void(
        _detail::invoke(LEXY_DECLVAL(Op), LEXY_DECLVAL(T&&), LEXY_DECLVAL(Args)...)));
};

template <typename T, typename Arg, bool Inplace, typename Op>
struct _fold
{
    Arg                  _init;
    LEXY_EMPTY_MEMBER Op _op;

    struct _sink_callback
    {
        T  _result;
        Op _op;

        using return_type = T;

        template <typename... Args>
        constexpr auto operator()(Args&&... args) ->
            typename _fold_sfinae<Inplace>::template type<Op, T, Args&&...>
        {
            if constexpr (Inplace)
                _detail::invoke(_op, _result, LEXY_FWD(args)...);
            else
                _result = _detail::invoke(_op, LEXY_MOV(_result), LEXY_FWD(args)...);
        }

        constexpr T finish() &&
        {
            return LEXY_MOV(_result);
        }
    };

    constexpr auto sink() const
    {
        if constexpr (std::is_constructible_v<T, Arg>)
            return _sink_callback{T(_init), _op};
        else
            return _sink_callback{_init(), _op};
    }
};

/// Sink that folds all the arguments with the binary operation op.
template <typename T, typename Arg = T, typename... Op>
constexpr auto fold(Arg&& init, Op&&... op)
{
    auto fn = _make_overloaded(LEXY_FWD(op)...);
    return _fold<T, std::decay_t<Arg>, false, decltype(fn)>{LEXY_FWD(init), LEXY_MOV(fn)};
}

/// Sink that folds all the arguments with the binary operation op that modifies the
/// result in-place.
template <typename T, typename Arg = T, typename... Op>
constexpr auto fold_inplace(Arg&& init, Op&&... op)
{
    auto fn = _make_overloaded(LEXY_FWD(op)...);
    return _fold<T, std::decay_t<Arg>, true, decltype(fn)>{LEXY_FWD(init), LEXY_MOV(fn)};
}
} // namespace lexy

namespace lexy
{
/// Sink that counts all arguments.
constexpr auto count
    = fold_inplace<std::size_t>(0u, [](std::size_t& result, auto&&...) { ++result; });
} // namespace lexy

#endif // LEXY_CALLBACK_FOLD_HPP_INCLUDED

