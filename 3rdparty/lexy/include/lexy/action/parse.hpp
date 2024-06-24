// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_PARSE_HPP_INCLUDED
#define LEXY_ACTION_PARSE_HPP_INCLUDED

#include <lexy/_detail/invoke.hpp>
#include <lexy/action/base.hpp>
#include <lexy/action/validate.hpp>
#include <lexy/callback/base.hpp>
#include <lexy/callback/bind.hpp>

namespace lexy
{
template <typename T, typename ErrorCallback>
class parse_result
{
    using _impl_t = lexy::validate_result<ErrorCallback>;

public:
    using value_type     = T;
    using error_callback = ErrorCallback;
    using error_type     = typename _impl_t::error_type;

    //=== status ===//
    constexpr explicit operator bool() const noexcept
    {
        return _impl.is_success();
    }

    constexpr bool is_success() const noexcept
    {
        return _impl.is_success();
    }
    constexpr bool is_error() const noexcept
    {
        return _impl.is_error();
    }
    constexpr bool is_recovered_error() const noexcept
    {
        return _impl.is_recovered_error();
    }
    constexpr bool is_fatal_error() const noexcept
    {
        return _impl.is_fatal_error();
    }

    //=== value ===//
    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(_value);
    }

    constexpr decltype(auto) value() const& noexcept
    {
        return *_value;
    }
    constexpr decltype(auto) value() && noexcept
    {
        return LEXY_MOV(*_value);
    }

    //=== error ===//
    constexpr std::size_t error_count() const noexcept
    {
        return _impl.error_count();
    }

    constexpr const auto& errors() const& noexcept
    {
        return _impl.errors();
    }
    constexpr auto&& errors() && noexcept
    {
        return LEXY_MOV(_impl).errors();
    }

private:
    constexpr explicit parse_result(_impl_t&& impl) noexcept : _impl(LEXY_MOV(impl)), _value() {}
    template <typename U>
    constexpr explicit parse_result(_impl_t&& impl, U&& v) noexcept : _impl(LEXY_MOV(impl))
    {
        LEXY_PRECONDITION(impl.is_success() || impl.is_recovered_error());
        _value.emplace(LEXY_FWD(v));
    }

    // In principle we could do a space optimization, as we can reconstruct the impl's status from
    // the state of _value and error. Feel free to implement it.
    _impl_t                     _impl;
    lexy::_detail::lazy_init<T> _value;

    template <typename Reader>
    friend class _ph;
};
} // namespace lexy

namespace lexy
{
template <typename Reader>
class _ph
{
    using iterator = typename Reader::iterator;

public:
    template <typename Input, typename Sink>
    constexpr explicit _ph(const _detail::any_holder<const Input*>& input,
                           _detail::any_holder<Sink>&               sink)
    : _validate(input, sink)
    {}

    using event_handler = typename _vh<Reader>::event_handler;

    constexpr operator _vh<Reader>&()
    {
        return _validate;
    }

    template <typename Production, typename State>
    using value_callback = production_value_callback<Production, State>;

    template <typename Result, typename T>
    constexpr auto get_result(bool rule_parse_result, T&& result) &&
    {
        using validate_result = lexy::validate_result<typename Result::error_callback>;
        return Result(LEXY_MOV(_validate).template get_result<validate_result>(rule_parse_result),
                      LEXY_MOV(result));
    }
    template <typename Result>
    constexpr auto get_result(bool rule_parse_result) &&
    {
        using validate_result = lexy::validate_result<typename Result::error_callback>;
        return Result(LEXY_MOV(_validate).template get_result<validate_result>(rule_parse_result));
    }

private:
    _vh<Reader> _validate;
};

template <typename State, typename Input, typename ErrorCallback>
struct parse_action
{
    const ErrorCallback* _callback;
    State*               _state = nullptr;

    using handler = _ph<lexy::input_reader<Input>>;
    using state   = State;
    using input   = Input;

    template <typename T>
    using result_type = parse_result<T, ErrorCallback>;

    constexpr explicit parse_action(const ErrorCallback& callback) : _callback(&callback) {}
    template <typename U = State>
    constexpr explicit parse_action(U& state, const ErrorCallback& callback)
    : _callback(&callback), _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        _detail::any_holder input_holder(&input);
        _detail::any_holder sink(_get_error_sink(*_callback));
        auto                reader = input.reader();
        return lexy::do_action<Production, result_type>(handler(input_holder, sink), _state,
                                                        reader);
    }
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename ErrorCallback>
constexpr auto parse(const Input& input, const ErrorCallback& callback)
{
    return parse_action<void, Input, ErrorCallback>(callback)(Production{}, input);
}

/// Parses the production into a value, invoking the callback on error.
/// All callbacks gain access to the specified parse state.
template <typename Production, typename Input, typename State, typename ErrorCallback>
constexpr auto parse(const Input& input, State& state, const ErrorCallback& callback)
{
    return parse_action<State, Input, ErrorCallback>(state, callback)(Production{}, input);
}
template <typename Production, typename Input, typename State, typename ErrorCallback>
constexpr auto parse(const Input& input, const State& state, const ErrorCallback& callback)
{
    return parse_action<const State, Input, ErrorCallback>(state, callback)(Production{}, input);
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_HPP_INCLUDED

