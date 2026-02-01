// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_VALIDATE_HPP_INCLUDED
#define LEXY_ACTION_VALIDATE_HPP_INCLUDED

#include <lexy/_detail/any_ref.hpp>
#include <lexy/_detail/lazy_init.hpp>
#include <lexy/action/base.hpp>
#include <lexy/callback/base.hpp>
#include <lexy/callback/container.hpp>
#include <lexy/callback/noop.hpp>
#include <lexy/error.hpp>

namespace lexy
{
// Convert the callback into an appropriate sink.
template <typename Callback>
constexpr auto _get_error_sink(const Callback& callback)
{
    if constexpr (std::is_same_v<Callback, lexy::_noop>)
    {
        // We collect noop instead, which counts the errors.
        return lexy::collect(callback).sink();
    }
    else if constexpr (lexy::is_sink<Callback>)
    {
        // It already is a sink.
        return callback.sink();
    }
    else
    {
        static_assert(
            std::is_void_v<typename Callback::return_type>,
            "need to use `lexy::collect()` to create an error callback that can handle multiple errors");

        // We need to collect the errors.
        return lexy::collect(callback).sink();
    }
}
template <typename Callback>
using _error_sink_t = decltype(_get_error_sink(LEXY_DECLVAL(Callback)));

template <typename ErrorCallback>
class validate_result
{
    using _sink_t = _error_sink_t<ErrorCallback>;

public:
    using error_callback = ErrorCallback;
    using error_type     = typename _sink_t::return_type;
    static_assert(!std::is_void_v<error_type>, "ErrorCallback must not be a void returning sink");

    constexpr explicit operator bool() const noexcept
    {
        return is_success();
    }

    constexpr bool is_success() const noexcept
    {
        return _status == _status_success;
    }
    constexpr bool is_error() const noexcept
    {
        return !is_success();
    }
    constexpr bool is_recovered_error() const noexcept
    {
        return _status == _status_recovered;
    }
    constexpr bool is_fatal_error() const noexcept
    {
        return _status == _status_fatal;
    }

    constexpr std::size_t error_count() const noexcept
    {
        if constexpr (std::is_same_v<error_type, std::size_t>)
            // void-returning callback yields the count only.
            return _error;
        else
            // We assume it's some sort of container otherwise.
            return _error.size();
    }

    constexpr const auto& errors() const& noexcept
    {
        return _error;
    }
    constexpr auto&& errors() && noexcept
    {
        return LEXY_MOV(_error);
    }

private:
    constexpr explicit validate_result(bool did_recover, error_type&& error)
    : _error(LEXY_MOV(error)), _status()
    {
        if (error_count() == 0u)
            _status = _status_success;
        else if (did_recover)
            _status = _status_recovered;
        else
            _status = _status_fatal;
    }

    error_type _error;
    enum
    {
        _status_success,
        _status_recovered,
        _status_fatal,
    } _status;

    template <typename Reader>
    friend class _vh;
};
} // namespace lexy

namespace lexy
{
template <typename Reader>
struct _validate_callbacks
{
    _detail::any_ref  sink;
    _detail::any_cref input;

    void (*generic)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator begin, const error<Reader, void>& error);
    void (*literal)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator begin, const error<Reader, expected_literal>& error);
    void (*keyword)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator begin, const error<Reader, expected_keyword>& error);
    void (*char_class)(_detail::any_ref sink, production_info info, _detail::any_cref input,
                       typename Reader::iterator                 begin,
                       const error<Reader, expected_char_class>& error);

    template <typename Input, typename Sink>
    constexpr _validate_callbacks(const _detail::any_holder<const Input*>& input,
                                  _detail::any_holder<Sink>&               sink)
    : sink(&sink), input(&input),
      generic([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                 typename Reader::iterator begin, const error<Reader, void>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      }),
      literal([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                 typename Reader::iterator begin, const error<Reader, expected_literal>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      }),
      keyword([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                 typename Reader::iterator begin, const error<Reader, expected_keyword>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      }),
      char_class([](_detail::any_ref sink, production_info info, _detail::any_cref input,
                    typename Reader::iterator                 begin,
                    const error<Reader, expected_char_class>& error) {
          lexy::error_context err_ctx(info, *input->template get<const Input*>(), begin);
          sink->template get<Sink>()(err_ctx, LEXY_FWD(error));
      })
    {}
};

template <typename Reader>
class _vh
{
public:
    template <typename Input, typename Sink>
    constexpr explicit _vh(const _detail::any_holder<const Input*>& input,
                           _detail::any_holder<Sink>&               sink)
    : _cb(input, sink)
    {}

    class event_handler
    {
        using iterator = typename Reader::iterator;

    public:
        constexpr event_handler(production_info info) : _begin(), _info(info) {}

        constexpr void on(_vh& handler, parse_events::production_start, iterator pos)
        {
            _begin = pos;

            _prev        = handler._top;
            handler._top = this;
        }
        constexpr void on(_vh& handler, parse_events::production_finish, iterator)
        {
            handler._top = _prev;
        }
        constexpr void on(_vh& handler, parse_events::production_cancel, iterator)
        {
            handler._top = _prev;
        }

        template <typename R, typename Tag>
        constexpr void on(_vh& handler, parse_events::error, const error<R, Tag>& error)
        {
            handler._cb.generic(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh& handler, parse_events::error, const error<R, void>& error)
        {
            handler._cb.generic(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh&                              handler, parse_events::error,
                          const error<R, expected_literal>& error)
        {
            handler._cb.literal(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh&                              handler, parse_events::error,
                          const error<R, expected_keyword>& error)
        {
            handler._cb.keyword(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }
        template <typename R>
        constexpr void on(_vh&                                 handler, parse_events::error,
                          const error<R, expected_char_class>& error)
        {
            handler._cb.char_class(handler._cb.sink, get_info(), handler._cb.input, _begin, error);
        }

        template <typename Event, typename... Args>
        constexpr auto on(_vh&, Event, const Args&...)
        {
            return 0; // operation_chain_start must return something
        }

        constexpr iterator production_begin() const
        {
            return _begin;
        }

        constexpr production_info get_info() const
        {
            auto cur = this;
            while (cur->_info.is_transparent && cur->_prev != nullptr)
                cur = cur->_prev;
            return cur->_info;
        }

    private:
        iterator        _begin;
        production_info _info;
        event_handler*  _prev = nullptr;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename Result>
    constexpr auto get_result(bool rule_parse_result) &&
    {
        using sink_t = _error_sink_t<typename Result::error_callback>;
        return Result(rule_parse_result, LEXY_MOV(_cb.sink->template get<sink_t>()).finish());
    }

private:
    _validate_callbacks<Reader> _cb;
    event_handler*              _top = nullptr;
};

template <typename State, typename Input, typename ErrorCallback>
struct validate_action
{
    const ErrorCallback* _callback;
    State*               _state = nullptr;

    using handler = _vh<lexy::input_reader<Input>>;
    using state   = State;
    using input   = Input;

    template <typename>
    using result_type = validate_result<ErrorCallback>;

    constexpr explicit validate_action(const ErrorCallback& callback) : _callback(&callback) {}
    template <typename U = State>
    constexpr explicit validate_action(U& state, const ErrorCallback& callback)
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

template <typename Production, typename Input, typename ErrorCallback>
constexpr auto validate(const Input& input, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    return validate_action<void, Input, ErrorCallback>(callback)(Production{}, input);
}

template <typename Production, typename Input, typename State, typename ErrorCallback>
constexpr auto validate(const Input& input, State& state, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    return validate_action<State, Input, ErrorCallback>(state, callback)(Production{}, input);
}
template <typename Production, typename Input, typename State, typename ErrorCallback>
constexpr auto validate(const Input& input, const State& state, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    return validate_action<const State, Input, ErrorCallback>(state, callback)(Production{}, input);
}
} // namespace lexy

#endif // LEXY_ACTION_VALIDATE_HPP_INCLUDED

