// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_BASE_HPP_INCLUDED
#define LEXY_CALLBACK_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/invoke.hpp>

namespace lexy
{
template <typename T>
using _detect_callback = typename T::return_type;
template <typename T>
constexpr bool is_callback = _detail::is_detected<_detect_callback, T>;

template <typename T, typename... Args>
using _detect_callback_for = decltype(LEXY_DECLVAL(const T)(LEXY_DECLVAL(Args)...));
template <typename T, typename... Args>
constexpr bool is_callback_for
    = _detail::is_detected<_detect_callback_for, std::decay_t<T>, Args...>;

template <typename T, typename State>
using _detect_callback_state = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(State&)]);
template <typename T, typename State>
constexpr bool is_callback_state
    = _detail::is_detected<_detect_callback_state, T, std::decay_t<State>>;

template <typename T, typename State, typename... Args>
using _detect_callback_with_state_for
    = decltype(LEXY_DECLVAL(const T)[LEXY_DECLVAL(State&)](LEXY_DECLVAL(Args)...));
template <typename T, typename State, typename... Args>
constexpr bool is_callback_with_state_for
    = _detail::is_detected<_detect_callback_with_state_for, std::decay_t<T>, State, Args...>;

/// Returns the type of the `.sink()` function.
template <typename Sink, typename... Args>
using sink_callback = decltype(LEXY_DECLVAL(Sink).sink(LEXY_DECLVAL(Args)...));

template <typename T, typename... Args>
using _detect_sink_callback_for = decltype(LEXY_DECLVAL(T&)(LEXY_DECLVAL(Args)...));
template <typename T, typename... Args>
constexpr bool is_sink_callback_for
    = _detail::is_detected<_detect_sink_callback_for, std::decay_t<T>, Args...>;

template <typename T, typename... Args>
using _detect_sink = decltype(LEXY_DECLVAL(const T).sink(LEXY_DECLVAL(Args)...).finish());
template <typename T, typename... Args>
constexpr bool is_sink = _detail::is_detected<_detect_sink, T, Args...>;
} // namespace lexy

namespace lexy
{
template <typename Fn>
struct _fn_holder
{
    Fn fn;

    constexpr explicit _fn_holder(Fn fn) : fn(fn) {}

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> decltype(_detail::invoke(fn, LEXY_FWD(args)...))
    {
        return _detail::invoke(fn, LEXY_FWD(args)...);
    }
};

template <typename Fn>
using _fn_as_base = std::conditional_t<std::is_class_v<Fn>, Fn, _fn_holder<Fn>>;

template <typename... Fns>
struct _overloaded : _fn_as_base<Fns>...
{
    constexpr explicit _overloaded(Fns... fns) : _fn_as_base<Fns>(LEXY_MOV(fns))... {}

    using _fn_as_base<Fns>::operator()...;
};

template <typename... Op>
constexpr auto _make_overloaded(Op&&... op)
{
    if constexpr (sizeof...(Op) == 1)
        return (LEXY_FWD(op), ...);
    else
        return _overloaded(LEXY_FWD(op)...);
}
} // namespace lexy

#endif // LEXY_CALLBACK_BASE_HPP_INCLUDED
