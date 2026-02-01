// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_TRACE_HPP_INCLUDED
#define LEXY_ACTION_TRACE_HPP_INCLUDED

#include <lexy/_detail/nttp_string.hpp>
#include <lexy/action/base.hpp>
#include <lexy/input_location.hpp>
#include <lexy/token.hpp>
#include <lexy/visualize.hpp>

//=== debug event ===//
namespace lexy::parse_events
{
/// Debug event was triggered.
/// Arguments: pos, str
struct debug
{};
} // namespace lexy::parse_events

namespace lexyd
{
template <typename CharT, CharT... C>
struct _debug : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<>;
            context.on(_ev::debug{}, reader.position(), str);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

#if LEXY_HAS_NTTP
template <lexy::_detail::string_literal Str>
constexpr auto debug = lexy::_detail::to_type_string<_debug, Str>{};
#endif

#define LEXY_DEBUG(Str)                                                                            \
    LEXY_NTTP_STRING(::lexyd::_debug, Str) {}
} // namespace lexyd

//=== trace ====//
namespace lexy::_detail
{
template <typename OutputIt, typename TokenKind>
class trace_writer
{
public:
    explicit trace_writer(OutputIt out, visualization_options opts)
    : _out(out), _opts(opts), _cur_depth(0)
    {}

    template <typename Location>
    void write_production_start(const Location& loc, const char* name)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            write_prefix(loc, prefix::event);

            _out = _detail::write_color<_detail::color::bold>(_out, _opts);
            _out = _detail::write_str(_out, name);
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);

            if (_cur_depth == _opts.max_tree_depth)
            {
                // Print an ellipsis instead of children.
                _out = _detail::write_str(_out, ": ");
                _out = _detail::write_ellipsis(_out, _opts);
            }
            else
            {
                // Prepare for children.
                _out = _detail::write_str(_out, ":");
            }
        }

        ++_cur_depth;
    }

    template <typename Location, typename Reader>
    void write_token(const Location& loc, lexy::token_kind<TokenKind> kind,
                     lexy::lexeme<Reader> lexeme)
    {
        if (_cur_depth > _opts.max_tree_depth || (kind.ignore_if_empty() && lexeme.empty()))
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, kind.name());
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (!lexeme.empty())
        {
            _out = _detail::write_str(_out, ": ");
            _out = visualize_to(_out, lexeme, _opts | visualize_space);
        }
    }

    template <typename Location, typename Reader>
    void write_backtrack(const Location& loc, lexy::lexeme<Reader> lexeme)
    {
        if (_cur_depth > _opts.max_tree_depth || lexeme.empty())
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::yellow, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "backtracked");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_str(_out, ": ");

        _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
        _out = visualize_to(_out, lexeme, _opts.reset(visualize_use_color) | visualize_space);
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);
    }

    template <typename Location, typename Reader, typename Tag>
    void write_error(const Location& loc, const lexy::error<Reader, Tag>& error)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::red, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "error");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::red>(_out, _opts);
        _out = _detail::write_str(_out, ": ");

        if constexpr (std::is_same_v<Tag, lexy::expected_literal>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                  error.length());

            _out = _detail::write_str(_out, "expected '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_keyword>)
        {
            auto string = _detail::make_literal_lexeme<typename Reader::encoding>(error.string(),
                                                                                  error.length());

            _out = _detail::write_str(_out, "expected keyword '");
            _out = visualize_to(_out, string, _opts);
            _out = _detail::write_str(_out, "'");
        }
        else if constexpr (std::is_same_v<Tag, lexy::expected_char_class>)
        {
            _out = _detail::write_str(_out, "expected ");
            _out = _detail::write_str(_out, error.name());
        }
        else
        {
            _out = _detail::write_str(_out, error.message());
        }

        _out = _detail::write_color<_detail::color::reset>(_out, _opts);
    }

    template <typename Location>
    void write_recovery_start(const Location& loc)
    {
        if (_cur_depth <= _opts.max_tree_depth)
        {
            write_prefix(loc, prefix::event);

            _out = _detail::write_color<_detail::color::yellow, _detail::color::bold>(_out, _opts);
            _out = _detail::write_str(_out, "error recovery");
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);

            _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
            _out = _detail::write_str(_out, ":");
            _out = _detail::write_color<_detail::color::reset>(_out, _opts);

            if (_cur_depth == _opts.max_tree_depth)
            {
                // Print an ellipsis instead of children.
                _out = _detail::write_str(_out, " ");
                _out = _detail::write_ellipsis(_out, _opts);
            }
        }
        ++_cur_depth;
    }

    template <typename Location>
    void write_operation(const Location& loc, const char* name)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "operation");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_str(_out, name);
    }

    template <typename Location>
    void write_debug(const Location& loc, const char* str)
    {
        if (_cur_depth > _opts.max_tree_depth)
            return;

        write_prefix(loc, prefix::event);

        _out = _detail::write_color<_detail::color::blue, _detail::color::bold>(_out, _opts);
        _out = _detail::write_str(_out, "debug");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        _out = _detail::write_color<_detail::color::blue>(_out, _opts);
        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_str(_out, str);
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);
    }

    template <typename Location>
    void write_finish(const Location& loc)
    {
        if (_cur_depth <= _opts.max_tree_depth)
            write_prefix(loc, prefix::finish);
        --_cur_depth;
    }
    template <typename Location>
    void write_cancel(const Location& loc)
    {
        if (_cur_depth <= _opts.max_tree_depth)
            write_prefix(loc, prefix::cancel);
        --_cur_depth;
    }

    OutputIt finish() &&
    {
        *_out++ = '\n';
        return _out;
    }

private:
    enum class prefix
    {
        event,
        cancel,
        finish,
    };

    template <typename Location>
    void write_prefix(const Location& loc, prefix p)
    {
        const auto use_unicode = _opts.is_set(visualize_use_unicode);

        if (_cur_depth > 0)
            *_out++ = '\n';

        _out = _detail::write_color<_detail::color::faint>(_out, _opts);
        _out = _detail::write_format(_out, "%2u:%3u", loc.line_nr(), loc.column_nr());
        _out = _detail::write_str(_out, ": ");
        _out = _detail::write_color<_detail::color::reset>(_out, _opts);

        if (_cur_depth > 0)
        {
            for (auto i = 0u; i != _cur_depth - 1; ++i)
                _out = _detail::write_str(_out, use_unicode ? u8"│  " : u8"  ");

            switch (p)
            {
            case prefix::event:
                _out = _detail::write_str(_out, use_unicode ? u8"├──" : u8"- ");
                break;
            case prefix::cancel:
                _out = _detail::write_str(_out, use_unicode ? u8"└" : u8"-");
                _out = _detail::write_color<_detail::color::yellow>(_out, _opts);
                _out = _detail::write_str(_out, use_unicode ? u8"╳" : u8"x");
                _out = _detail::write_color<_detail::color::reset>(_out, _opts);
                break;
            case prefix::finish:
                _out = _detail::write_str(_out, use_unicode ? u8"┴" : u8"- finish");
                break;
            }
        }
    }

    OutputIt              _out;
    visualization_options _opts;

    std::size_t _cur_depth;
};
} // namespace lexy::_detail

namespace lexy
{
template <typename OutputIt, typename Input, typename TokenKind = void>
class _th
{
public:
    explicit _th(OutputIt out, const Input& input, visualization_options opts = {}) noexcept
    : _writer(out, opts), _input(&input), _anchor(input)
    {
        LEXY_PRECONDITION(opts.max_tree_depth <= visualization_options::max_tree_depth_limit);
    }

    class event_handler
    {
        using iterator = typename lexy::input_reader<Input>::iterator;

    public:
        constexpr event_handler(production_info info) : _info(info) {}

        void on(_th&, parse_events::grammar_start, iterator) {}
        void on(_th&, parse_events::grammar_finish, lexy::input_reader<Input>&) {}
        void on(_th&, parse_events::grammar_cancel, lexy::input_reader<Input>&) {}

        void on(_th& handler, parse_events::production_start, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_production_start(loc, _info.name);

            // All events for the production are after the initial event.
            _previous_anchor.emplace(handler._anchor);
            handler._anchor = loc.anchor();
        }
        void on(_th& handler, parse_events::production_finish, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_finish(loc);
        }
        void on(_th& handler, parse_events::production_cancel, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_cancel(loc);

            // We've backtracked, so we need to restore the anchor.
            handler._anchor = *_previous_anchor;
        }

        int on(_th& handler, parse_events::operation_chain_start, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_production_start(loc, "operation chain");
            return 0; // need to return something
        }
        template <typename Operation>
        void on(_th& handler, parse_events::operation_chain_op, Operation, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_operation(loc, lexy::production_name<Operation>());
        }
        void on(_th& handler, parse_events::operation_chain_finish, int, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_finish(loc);
        }

        template <typename TK>
        void on(_th& handler, parse_events::token, TK kind, iterator begin, iterator end)
        {
            auto loc = handler.get_location(begin);
            handler._writer.write_token(loc, token_kind<TokenKind>(kind),
                                        lexeme_for<Input>(begin, end));
        }
        void on(_th& handler, parse_events::backtracked, iterator begin, iterator end)
        {
            auto loc = handler.get_location(begin);
            handler._writer.write_backtrack(loc, lexeme_for<Input>(begin, end));
        }

        template <typename Error>
        void on(_th& handler, parse_events::error, const Error& error)
        {
            auto loc = handler.get_location(error.position());
            handler._writer.write_error(loc, error);
        }

        void on(_th& handler, parse_events::recovery_start, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_recovery_start(loc);
        }
        void on(_th& handler, parse_events::recovery_finish, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_finish(loc);
        }
        void on(_th& handler, parse_events::recovery_cancel, iterator pos)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_cancel(loc);
        }

        void on(_th& handler, parse_events::debug, iterator pos, const char* str)
        {
            auto loc = handler.get_location(pos);
            handler._writer.write_debug(loc, str);
        }

    private:
        production_info _info;
        // The beginning of the previous production.
        // If the current production gets canceled, it needs to be restored.
        _detail::lazy_init<input_location_anchor<Input>> _previous_anchor;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    template <typename>
    constexpr OutputIt get_result(bool) &&
    {
        return LEXY_MOV(_writer).finish();
    }

private:
    input_location<Input> get_location(typename lexy::input_reader<Input>::iterator pos)
    {
        return get_input_location(*_input, pos, _anchor);
    }

    _detail::trace_writer<OutputIt, TokenKind> _writer;

    const Input*                 _input;
    input_location_anchor<Input> _anchor;
};

template <typename State, typename Input, typename OutputIt, typename TokenKind = void>
struct trace_action
{
    OutputIt              _out;
    visualization_options _opts;
    State*                _state = nullptr;

    using handler = _th<OutputIt, Input>;
    using state   = State;
    using input   = Input;

    template <typename>
    using result_type = OutputIt;

    constexpr explicit trace_action(OutputIt out, visualization_options opts = {})
    : _out(out), _opts(opts)
    {}
    template <typename U = State>
    constexpr explicit trace_action(U& state, OutputIt out, visualization_options opts = {})
    : _out(out), _opts(opts), _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        auto reader = input.reader();
        return lexy::do_action<Production, result_type>(handler(_out, input, _opts), _state,
                                                        reader);
    }
};

template <typename Production, typename TokenKind = void, typename OutputIt, typename Input>
OutputIt trace_to(OutputIt out, const Input& input, visualization_options opts = {})
{
    return trace_action<void, Input, OutputIt, TokenKind>(out, opts)(Production{}, input);
}
template <typename Production, typename TokenKind = void, typename OutputIt, typename Input,
          typename State>
OutputIt trace_to(OutputIt out, const Input& input, State& state, visualization_options opts = {})
{
    return trace_action<State, Input, OutputIt, TokenKind>(state, out, opts)(Production{}, input);
}
template <typename Production, typename TokenKind = void, typename OutputIt, typename Input,
          typename State>
OutputIt trace_to(OutputIt out, const Input& input, const State& state,
                  visualization_options opts = {})
{
    return trace_action<const State, Input, OutputIt, TokenKind>(state, out, opts)(Production{},
                                                                                   input);
}

template <typename Production, typename TokenKind = void, typename Input>
void trace(std::FILE* file, const Input& input, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, opts);
}
template <typename Production, typename TokenKind = void, typename Input, typename State>
void trace(std::FILE* file, const Input& input, State& state, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, state, opts);
}
template <typename Production, typename TokenKind = void, typename Input, typename State>
void trace(std::FILE* file, const Input& input, const State& state, visualization_options opts = {})
{
    trace_to<Production, TokenKind>(cfile_output_iterator{file}, input, state, opts);
}
} // namespace lexy

#endif // LEXY_ACTION_TRACE_HPP_INCLUDED

