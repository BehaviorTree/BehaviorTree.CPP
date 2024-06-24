// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_SCAN_HPP_INCLUDED
#define LEXY_ACTION_SCAN_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/action/validate.hpp>
#include <lexy/dsl/scan.hpp>

namespace lexy
{
template <typename ControlProduction>
struct _scp;
template <>
struct _scp<void>
{
    static LEXY_CONSTEVAL auto name()
    {
        return "scanner control production";
    }

    static constexpr auto rule  = dsl::scan;
    static constexpr auto value = lexy::noop;
};
template <typename ControlProduction>
struct _scp : ControlProduction, _scp<void>
{};

template <typename ControlProduction, typename Input, typename State, typename ErrorCallback>
class scanner : public _detail::scanner<scanner<ControlProduction, Input, State, ErrorCallback>,
                                        lexy::input_reader<Input>>
{
    using _impl       = _detail::scanner<scanner<ControlProduction, Input, State, ErrorCallback>,
                                   lexy::input_reader<Input>>;
    using _handler    = lexy::validate_handler<Input, ErrorCallback>;
    using _production = _scp<ControlProduction>;

public:
    constexpr explicit scanner(const Input& input, const State* state,
                               const ErrorCallback& callback)
    : _impl(input.reader()),
      _cb(_handler(input, callback), state, max_recursion_depth<_production>()), _context(&_cb)
    {
        _context.on(parse_events::production_start{}, this->position());
    }

    constexpr const auto& parse_state() const
    {
        return *_context.control_block->parse_state;
    }

    constexpr auto finish() && -> lexy::validate_result<ErrorCallback>
    {
        auto parse_result = static_cast<bool>(*this);

        if (parse_result)
            _context.on(parse_events::production_finish{}, this->position());
        else
            _context.on(parse_events::production_cancel{}, this->position());

        return LEXY_MOV(_cb.parse_handler).get_result_void(parse_result);
    }

private:
    auto& context() noexcept
    {
        return _context;
    }

    _detail::parse_context_control_block<_handler> _cb;
    _pc<_handler, State, _production>              _context;

    friend _impl;
};

template <typename ControlProduction = void, typename Input, typename ErrorCallback>
constexpr auto scan(const Input& input, const ErrorCallback& callback)
{
    return scanner<ControlProduction, Input, void, ErrorCallback>(input, no_parse_state, callback);
}

template <typename ControlProduction = void, typename Input, typename State, typename ErrorCallback>
constexpr auto scan(const Input& input, const State& state, const ErrorCallback& callback)
{
    return scanner<ControlProduction, Input, State, ErrorCallback>(input, &state, callback);
}
} // namespace lexy

#endif // LEXY_ACTION_SCAN_HPP_INCLUDED

