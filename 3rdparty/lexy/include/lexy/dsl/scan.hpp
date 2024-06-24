// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_SCAN_HPP_INCLUDED
#define LEXY_DSL_SCAN_HPP_INCLUDED

#include <lexy/_detail/lazy_init.hpp>
#include <lexy/action/base.hpp>
#include <lexy/callback/forward.hpp>
#include <lexy/callback/object.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>
#include <lexy/lexeme.hpp>

//=== rule forward declaration ===//
namespace lexyd
{
template <typename Production>
struct _prd;
template <typename Rule, typename Tag>
struct _peek;
template <typename Token>
struct _capt;
template <typename T, typename Base>
struct _int_dsl;

struct _scan;
} // namespace lexyd

namespace lexy::_detail
{
template <typename Derived, typename Reader>
class scanner;
}

//=== scan_result ===//
namespace lexy
{
constexpr struct scan_failed_t
{
} scan_failed;

template <typename T>
class scan_result
{
public:
    using value_type = T;

    constexpr scan_result() = default;
    constexpr scan_result(scan_failed_t) {}

    template <typename U = T, typename = std::enable_if_t<std::is_constructible_v<T, U>>>
    constexpr scan_result(U&& value)
    {
        _value.emplace(LEXY_MOV(value));
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }
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

    template <typename U = T>
    constexpr decltype(auto) value_or(U&& fallback) const& noexcept
    {
        return _value ? *_value : LEXY_FWD(fallback);
    }
    template <typename U = T>
    constexpr decltype(auto) value_or(U&& fallback) && noexcept
    {
        return _value ? LEXY_MOV(*_value) : LEXY_FWD(fallback);
    }

private:
    constexpr explicit scan_result(_detail::lazy_init<T>&& value) : _value(LEXY_MOV(value)) {}

    _detail::lazy_init<T> _value;

    template <typename Derived, typename Reader>
    friend class _detail::scanner;
};
template <>
class scan_result<void>
{
public:
    using value_type = void;

    constexpr scan_result() = default;
    constexpr scan_result(scan_failed_t) {}
    constexpr scan_result(bool has_value)
    {
        if (has_value)
            _value.emplace();
    }

    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }
    constexpr bool has_value() const noexcept
    {
        return static_cast<bool>(_value);
    }

private:
    constexpr explicit scan_result(_detail::lazy_init<void>&& value) : _value(LEXY_MOV(value)) {}

    _detail::lazy_init<void> _value;

    template <typename Derived, typename Reader>
    friend class _detail::scanner;
};

template <typename T>
scan_result(T&&) -> scan_result<std::decay_t<T>>;
template <typename T>
scan_result(_detail::lazy_init<T>&&) -> scan_result<T>;
} // namespace lexy

//=== scanner implementation ===//
namespace lexy::_detail
{
template <typename Context>
using _value_callback_for = lexy::production_value_callback<
    typename Context::production,
    std::remove_pointer_t<decltype(LEXY_DECLVAL(decltype(Context::control_block))->parse_state)>>;

// The context used for a child production during scanning.
// It forwards all events but overrides the value callback.
template <typename RootContext, typename Context,
          typename ValueCallback = _value_callback_for<Context>>
struct spc_child
{
    using production            = typename Context::production;
    using whitespace_production = typename Context::whitespace_production;
    using value_type            = typename ValueCallback::return_type;

    RootContext*                     root_context;
    decltype(Context::handler)       handler;
    decltype(Context::control_block) control_block;
    _detail::lazy_init<value_type>   value;

    constexpr explicit spc_child(RootContext& root, decltype(Context::control_block) cb)
    : root_context(&root), control_block(cb)
    {}
    constexpr explicit spc_child(RootContext& root, const Context& context)
    : root_context(&root), control_block(context.control_block)
    {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction child)
    {
        using sub_context_t = decltype(LEXY_DECLVAL(Context).sub_context(child));
        if constexpr (std::is_same_v<ValueCallback, void_value_callback>)
            return spc_child<RootContext, sub_context_t, void_value_callback>(*root_context,
                                                                              control_block);
        else
            return spc_child<RootContext, sub_context_t>(*root_context, control_block);
    }

    constexpr auto value_callback()
    {
        return ValueCallback(control_block->parse_state);
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args)
    {
        return handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};

// The context used for a top-level rule parsing during scanning.
// It forwards all events but overrids the value callback to construct a T.
template <typename T, typename Context>
struct spc
{
    using production            = typename Context::production;
    using whitespace_production = typename Context::whitespace_production;
    using value_type            = T;

    Context*                         root_context;
    decltype(Context::handler)&      handler;
    decltype(Context::control_block) control_block;
    _detail::lazy_init<T>&           value;

    constexpr explicit spc(_detail::lazy_init<T>& value, Context& context)
    : root_context(&context), handler(context.handler), control_block(context.control_block),
      value(value)
    {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction child)
    {
        using sub_context_t = decltype(LEXY_DECLVAL(Context).sub_context(child));
        if constexpr (std::is_void_v<T>)
            return spc_child<Context, sub_context_t, void_value_callback>(*root_context,
                                                                          control_block);
        else
            return spc_child<Context, sub_context_t>(*root_context, control_block);
    }

    constexpr auto value_callback()
    {
        return lexy::construct<T>;
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args)
    {
        return handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};

template <typename Reader>
struct scanner_input
{
    Reader _impl;

    constexpr auto reader() const&
    {
        return _impl;
    }
};

// The common interface of all scanner types.
template <typename Derived, typename Reader>
class scanner
{
public:
    using encoding = typename Reader::encoding;

    constexpr scanner(const scanner&) noexcept = delete;
    constexpr scanner& operator=(const scanner&) noexcept = delete;

    //=== status ===//
    constexpr explicit operator bool() const noexcept
    {
        return _state == _state_normal;
    }

    constexpr bool is_at_eof() const
    {
        return _reader.peek() == Reader::encoding::eof();
    }

    constexpr auto position() const noexcept -> typename Reader::iterator
    {
        return _reader.position();
    }

    constexpr auto remaining_input() const noexcept
    {
        return scanner_input<Reader>{_reader};
    }

    //=== parsing ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr void parse(scan_result<T>& result, Rule)
    {
        if (_state == _state_failed)
            return;

        _detail::spc context(result._value, static_cast<Derived&>(*this).context());

        using parser = lexy::parser_for<Rule, lexy::_detail::final_parser>;
        auto success = parser::parse(context, _reader);
        if (!success)
            _state = _state_failed;
    }

    template <typename Production, typename Rule = lexy::production_rule<Production>>
    constexpr auto parse(Production production = {})
    {
        // Directly create a child context from the production context.
        auto& root_context = static_cast<Derived&>(*this).context();
        auto  context      = _detail::spc_child(root_context, root_context.sub_context(production));
        if (_state == _state_failed)
            return scan_result(LEXY_MOV(context.value));

        // We manually parse the rule of the production, so need to raise events.
        context.on(lexy::parse_events::production_start{}, _reader.position());

        if constexpr (lexy::_production_defines_whitespace<Production>)
        {
            // Skip initial whitespace of the production.
            using whitespace_parser
                = lexy::whitespace_parser<decltype(context), lexy::pattern_parser<>>;
            if (!whitespace_parser::parse(context, _reader))
            {
                context.on(lexy::parse_events::production_cancel{}, _reader.position());
                _state = _state_failed;
                return scan_result(LEXY_MOV(context.value));
            }
        }

        using parser = lexy::parser_for<Rule, lexy::_detail::final_parser>;
        auto success = parser::parse(context, _reader);
        if (!success)
        {
            context.on(lexy::parse_events::production_cancel{}, _reader.position());
            _state = _state_failed;
            return scan_result(LEXY_MOV(context.value));
        }

        context.on(lexy::parse_events::production_finish{}, _reader.position());

        if constexpr (lexy::is_token_production<Production>)
        {
            // Skip trailing whitespace of the parent.
            using whitespace_parser = lexy::whitespace_parser<LEXY_DECAY_DECLTYPE(root_context),
                                                              lexy::pattern_parser<>>;
            if (!whitespace_parser::parse(root_context, _reader))
            {
                _state = _state_failed;
                return scan_result(LEXY_MOV(context.value));
            }
        }

        return scan_result(LEXY_MOV(context.value));
    }

    template <typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr void parse(Rule rule)
    {
        scan_result<void> result;
        parse(result, rule);
    }

    //=== branch parsing ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr bool branch(scan_result<T>& result, Rule)
    {
        static_assert(lexy::is_branch_rule<Rule>);
        if (_state == _state_failed)
            return false;

        _detail::spc context(result._value, static_cast<Derived&>(*this).context());

        lexy::branch_parser_for<Rule, Reader> parser{};
        if (!parser.try_parse(context.control_block, _reader))
        {
            parser.cancel(context);
            return false; // branch wasn't token
        }

        auto success = parser.template finish<lexy::_detail::final_parser>(context, _reader);
        if (!success)
            _state = _state_failed;
        return true; // branch was taken
    }

    template <typename Production, typename T, typename = lexy::production_rule<Production>>
    constexpr bool branch(scan_result<T>& result, Production = {})
    {
        return branch(result, lexyd::_prd<Production>{});
    }

    template <typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr bool branch(Rule rule)
    {
        scan_result<void> result;
        return branch(result, rule);
    }

    //=== error handling ===//
    class error_recovery_guard
    {
    public:
        error_recovery_guard(const error_recovery_guard&) = delete;
        error_recovery_guard& operator=(const error_recovery_guard&) = delete;

        constexpr void cancel() &&
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_cancel{}, _self->_reader.position());
            _self->_state = _state_failed;
        }

        constexpr void finish() &&
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_finish{}, _self->_reader.position());
            _self->_state = _state_normal;
        }

    private:
        constexpr explicit error_recovery_guard(scanner& self) noexcept : _self(&self)
        {
            auto& context = static_cast<Derived&>(*_self).context();
            context.on(parse_events::recovery_start{}, _self->_reader.position());
            _self->_state = _state_recovery;
        }

        scanner* _self;
        friend scanner;
    };

    constexpr auto error_recovery()
    {
        LEXY_PRECONDITION(_state == _state_failed);
        return error_recovery_guard(*this);
    }

    template <typename TokenRule>
    constexpr bool discard(TokenRule rule)
    {
        static_assert(lexy::is_token_rule<TokenRule>);
        if (_state == _state_failed)
            return false;

        auto begin  = _reader.position();
        auto result = lexy::try_match_token(rule, _reader);
        auto end    = _reader.position();

        auto& context = static_cast<Derived&>(*this).context();
        context.on(parse_events::token{}, lexy::error_token_kind, begin, end);

        return result;
    }

    template <typename Tag, typename... Args>
    constexpr void error(Tag, Args&&... args)
    {
        auto& context = static_cast<Derived&>(*this).context();
        context.on(parse_events::error{}, lexy::error<Reader, Tag>(LEXY_FWD(args)...));
    }

    template <typename Tag, typename... Args>
    constexpr void fatal_error(Tag tag, Args&&... args)
    {
        error(tag, LEXY_FWD(args)...);
        _state = _state_failed;
    }

    //=== convenience ===//
    template <typename T, typename Rule, typename = std::enable_if_t<lexy::is_rule<Rule>>>
    constexpr auto parse(Rule rule)
    {
        scan_result<T> result;
        parse(result, rule);
        return result;
    }

    template <typename Rule>
    constexpr bool peek(Rule)
    {
        static_assert(lexy::is_rule<Rule>);
        return branch(dsl::_peek<Rule, void>{});
    }

    template <typename T, typename Base, typename Digits>
    constexpr auto integer(Digits digits)
    {
        scan_result<T> result;
        parse(result, lexyd::_int_dsl<T, Base>{}(digits));
        return result;
    }
    template <typename T, typename Digits>
    constexpr auto integer(Digits digits)
    {
        scan_result<T> result;
        parse(result, lexyd::_int_dsl<T, void>{}(digits));
        return result;
    }

    template <typename Rule>
    constexpr auto capture(Rule rule) -> scan_result<lexeme<Reader>>
    {
        static_assert(lexy::is_rule<Rule>);

        auto begin = _reader.position();
        parse(rule);
        auto end = _reader.position();

        if (*this)
            return lexeme<Reader>(begin, end);
        else
            return scan_failed;
    }
    template <typename Token>
    constexpr auto capture_token(Token)
    {
        scan_result<lexeme<Reader>> result;
        parse(result, lexyd::_capt<Token>{});
        return result;
    }

protected:
    constexpr explicit scanner(const Reader& reader) noexcept
    : _reader(reader), _state(_state_normal)
    {}

    constexpr Reader& reader() noexcept
    {
        return _reader;
    }

private:
    Reader _reader;

    enum
    {
        _state_normal,
        _state_failed,   // after a failure
        _state_recovery, // recovery guard active
    } _state;
};
} // namespace lexy::_detail

//=== dsl::scan ===//
namespace lexy
{
template <typename Context, typename Reader>
class rule_scanner : public _detail::scanner<rule_scanner<Context, Reader>, Reader>
{
public:
    using production = typename Context::production;

    constexpr std::size_t recursion_depth() const noexcept
    {
        auto& cb = _context->control_block();
        return static_cast<std::size_t>(cb.cur_depth);
    }

    constexpr auto begin() const noexcept -> typename Reader::iterator
    {
        return _begin;
    }

private:
    constexpr explicit rule_scanner(Context& context, Reader reader)
    : _detail::scanner<rule_scanner<Context, Reader>, Reader>(reader), _context(&context),
      _begin(reader.position())
    {}

    constexpr Context& context() noexcept
    {
        return *_context;
    }

    Context*                  _context;
    typename Reader::iterator _begin;

    friend _detail::scanner<rule_scanner<Context, Reader>, Reader>;
    friend lexyd::_scan;
};
} // namespace lexy

namespace lexyd
{
template <typename Context, typename Scanner, typename StatePtr>
using _detect_scan_state = decltype(Context::production::scan(LEXY_DECLVAL(Scanner&), *StatePtr()));

struct _scan : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Scanner, typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool _parse(Scanner& scanner, Context& context, Reader& reader,
                                            Args&&... args)
        {
            lexy::scan_result result = [&] {
                if constexpr (lexy::_detail::is_detected<
                                  _detect_scan_state, Context, decltype(scanner),
                                  decltype(context.control_block->parse_state)>)
                    return Context::production::scan(scanner, *context.control_block->parse_state,
                                                     LEXY_FWD(args)...);
                else
                    return Context::production::scan(scanner, LEXY_FWD(args)...);
            }();
            reader.set_position(scanner.position());
            if (!result)
                return false;

            if constexpr (std::is_void_v<typename decltype(result)::value_type>)
                return NextParser::parse(context, reader);
            else
                return NextParser::parse(context, reader, LEXY_MOV(result).value());
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::rule_scanner scanner(context, reader);
            return _parse(scanner, context, reader, LEXY_FWD(args)...);
        }
        template <typename RootContext, typename Context, typename ValueCallback, typename Reader,
                  typename... Args>
        LEXY_PARSER_FUNC static bool parse(
            lexy::_detail::spc_child<RootContext, Context, ValueCallback>& context, Reader& reader,
            Args&&... args)
        {
            lexy::rule_scanner scanner(*context.root_context, reader);
            return _parse(scanner, context, reader, LEXY_FWD(args)...);
        }
    };
};

inline constexpr auto scan = _scan{};
} // namespace lexyd

namespace lexy
{
template <typename T>
struct scan_production
{
    using scan_result = lexy::scan_result<T>;

    static constexpr auto rule  = dsl::scan;
    static constexpr auto value = lexy::forward<T>;
};
} // namespace lexy

#endif // LEXY_DSL_SCAN_HPP_INCLUDED

