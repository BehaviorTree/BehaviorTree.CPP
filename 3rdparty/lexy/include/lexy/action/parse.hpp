// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
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

    template <typename Input, typename Callback>
    friend class parse_handler;
};
} // namespace lexy

namespace lexy
{
template <typename Input, typename ErrorCallback>
class parse_handler
{
    using iterator = typename lexy::input_reader<Input>::iterator;

public:
    constexpr explicit parse_handler(const Input& input, const ErrorCallback& callback)
    : _validate(input, callback)
    {}

    template <typename Production>
    using event_handler =
        typename validate_handler<Input, ErrorCallback>::template event_handler<Production>;

    constexpr operator validate_handler<Input, ErrorCallback>&()
    {
        return _validate;
    }

    template <typename Production, typename State>
    using value_callback = production_value_callback<Production, State>;

    constexpr auto get_result_void(bool rule_parse_result) &&
    {
        return parse_result<void, ErrorCallback>(
            LEXY_MOV(_validate).get_result_void(rule_parse_result));
    }

    template <typename T>
    constexpr auto get_result(bool rule_parse_result, T&& result) &&
    {
        return parse_result<T, ErrorCallback>(LEXY_MOV(_validate).get_result_void(
                                                  rule_parse_result),
                                              LEXY_MOV(result));
    }
    template <typename T>
    constexpr auto get_result(bool rule_parse_result) &&
    {
        return parse_result<T, ErrorCallback>(
            LEXY_MOV(_validate).get_result_void(rule_parse_result));
    }

private:
    validate_handler<Input, ErrorCallback> _validate;
};

/// Parses the production into a value, invoking the callback on error.
template <typename Production, typename Input, typename Callback>
constexpr auto parse(const Input& input, Callback callback)
{
    auto handler = lexy::parse_handler(input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), no_parse_state, reader);
}

/// Parses the production into a value, invoking the callback on error.
/// All callbacks gain access to the specified parse state.
template <typename Production, typename Input, typename State, typename Callback>
constexpr auto parse(const Input& input, const State& state, Callback callback)
{
    auto handler = lexy::parse_handler(input, LEXY_MOV(callback));
    auto reader  = input.reader();
    return lexy::do_action<Production>(LEXY_MOV(handler), &state, reader);
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_HPP_INCLUDED

