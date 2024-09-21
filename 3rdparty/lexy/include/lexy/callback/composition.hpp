// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED
#define LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED

#include <lexy/callback/base.hpp>

namespace lexy
{
template <typename Cb, typename State, typename = void>
struct _compose_state
{
    const Cb& _cb;
    State&    _state;

    using return_type = typename Cb::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_cb(LEXY_FWD(args)...))
    {
        return _cb(LEXY_FWD(args)...);
    }
};
template <typename Cb, typename State>
struct _compose_state<Cb, State, std::enable_if_t<lexy::is_callback_state<Cb, State>>>
{
    const Cb& _cb;
    State&    _state;

    using return_type = typename Cb::return_type;

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_cb[_state](LEXY_FWD(args)...))
    {
        return _cb[_state](LEXY_FWD(args)...);
    }
};

template <typename First, typename Second>
struct _compose_cb
{
    LEXY_EMPTY_MEMBER First  _first;
    LEXY_EMPTY_MEMBER Second _second;

    constexpr explicit _compose_cb(First&& first, Second&& second)
    : _first(LEXY_MOV(first)), _second(LEXY_MOV(second))
    {}

    using return_type = typename Second::return_type;

    template <typename State,
              typename = std::enable_if_t<lexy::is_callback_state<First, State> //
                                          || lexy::is_callback_state<Second, State>>>
    constexpr auto operator[](State& state) const
    {
        auto first  = _compose_state<First, State>{_first, state};
        auto second = _compose_state<Second, State>{_second, state};
        return lexy::_compose_cb(LEXY_MOV(first), LEXY_MOV(second));
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const
        -> LEXY_DECAY_DECLTYPE(_first(LEXY_FWD(args)...), LEXY_DECLVAL(return_type))
    {
        return _second(_first(LEXY_FWD(args)...));
    }
};

template <typename Sink, typename Callback>
struct _compose_s
{
    LEXY_EMPTY_MEMBER Sink     _sink;
    LEXY_EMPTY_MEMBER Callback _callback;

    using return_type = typename Callback::return_type;

    template <typename... Args>
    constexpr auto sink(Args&&... args) const -> decltype(_sink.sink(LEXY_FWD(args)...))
    {
        return _sink.sink(LEXY_FWD(args)...);
    }

    template <typename State, typename = std::enable_if_t<lexy::is_callback_state<Callback, State>>>
    constexpr auto operator[](State& state) const
    {
        return _compose_state<Callback, State>{_callback, state};
    }

    template <typename... Args>
    constexpr auto operator()(Args&&... args) const -> decltype(_callback(LEXY_FWD(args)...))
    {
        return _callback(LEXY_FWD(args)...);
    }
};

/// Composes two callbacks.
template <typename First, typename Second, typename = _detect_callback<First>,
          typename = _detect_callback<Second>>
constexpr auto operator|(First first, Second second)
{
    return _compose_cb(LEXY_MOV(first), LEXY_MOV(second));
}
template <typename S, typename Cb, typename Second>
constexpr auto operator|(_compose_s<S, Cb> composed, Second second)
{
    auto cb = LEXY_MOV(composed._callback) | LEXY_MOV(second);
    return _compose_s<S, decltype(cb)>{LEXY_MOV(composed._sink), LEXY_MOV(cb)};
}

/// Composes a sink with a callback.
template <typename Sink, typename Callback, typename = _detect_callback<Callback>>
constexpr auto operator>>(Sink sink, Callback cb)
{
    return _compose_s<Sink, Callback>{LEXY_MOV(sink), LEXY_MOV(cb)};
}
} // namespace lexy

#endif // LEXY_CALLBACK_COMPOSITION_HPP_INCLUDED

