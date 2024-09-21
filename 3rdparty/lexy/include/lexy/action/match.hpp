// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_MATCH_HPP_INCLUDED
#define LEXY_ACTION_MATCH_HPP_INCLUDED

#include <lexy/action/base.hpp>

namespace lexy
{
class _mh
{
public:
    constexpr _mh() : _failed(false) {}

    class event_handler
    {
    public:
        constexpr event_handler(production_info) {}

        template <typename Error>
        constexpr void on(_mh& handler, parse_events::error, Error&&)
        {
            handler._failed = true;
        }

        template <typename Event, typename... Args>
        constexpr int on(_mh&, Event, const Args&...)
        {
            return 0; // operation_chain_start needs to return something
        }
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename>
    constexpr bool get_result(bool rule_parse_result) &&
    {
        return rule_parse_result && !_failed;
    }

private:
    bool _failed;
};

template <typename State, typename Input>
struct match_action
{
    State* _state = nullptr;

    using handler = _mh;
    using state   = State;
    using input   = Input;

    template <typename>
    using result_type = bool;

    constexpr match_action() = default;
    template <typename U = State>
    constexpr explicit match_action(U& state) : _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        auto reader = input.reader();
        return lexy::do_action<Production, result_type>(handler(), _state, reader);
    }
};

template <typename Production, typename Input>
constexpr bool match(const Input& input)
{
    return match_action<void, Input>()(Production{}, input);
}
template <typename Production, typename Input, typename State>
constexpr bool match(const Input& input, State& state)
{
    return match_action<State, Input>(state)(Production{}, input);
}
template <typename Production, typename Input, typename State>
constexpr bool match(const Input& input, const State& state)
{
    return match_action<const State, Input>(state)(Production{}, input);
}
} // namespace lexy

#endif // LEXY_ACTION_MATCH_HPP_INCLUDED

