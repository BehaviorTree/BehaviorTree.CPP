// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_BASE_HPP_INCLUDED
#define LEXY_ACTION_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/lazy_init.hpp>
#include <lexy/_detail/type_name.hpp>
#include <lexy/callback/noop.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/grammar.hpp>

//=== parse_context ===//
namespace lexy
{
namespace _detail
{
    struct parse_context_var_base
    {
        const void*             id;
        parse_context_var_base* next;

        constexpr parse_context_var_base(const void* id) : id(id), next(nullptr) {}

        template <typename Context>
        constexpr void link(Context& context)
        {
            auto cb  = context.control_block;
            next     = cb->vars;
            cb->vars = this;
        }

        template <typename Context>
        constexpr void unlink(Context& context)
        {
            auto cb  = context.control_block;
            cb->vars = next;
        }
    };

    template <typename Id, typename T>
    struct parse_context_var : parse_context_var_base
    {
        static constexpr auto type_id = lexy::_detail::type_id<Id>();

        T value;

        explicit constexpr parse_context_var(T&& value)
        : parse_context_var_base(&type_id), value(LEXY_MOV(value))
        {}

        template <typename ControlBlock>
        static constexpr T& get(const ControlBlock* cb)
        {
            for (auto cur = cb->vars; cur; cur = cur->next)
                if (cur->id == &type_id)
                    return static_cast<parse_context_var*>(cur)->value;

            LEXY_ASSERT(false, "context variable hasn't been created");
            return static_cast<parse_context_var*>(cb->vars)->value;
        }
    };

    template <typename Handler, typename State = void>
    struct parse_context_control_block
    {
        using handler_type = Handler;
        using state_type   = State;

        LEXY_EMPTY_MEMBER Handler parse_handler;
        State*                    parse_state;

        parse_context_var_base* vars;

        int  cur_depth, max_depth;
        bool enable_whitespace_skipping;

        constexpr parse_context_control_block(Handler&& handler, State* state,
                                              std::size_t max_depth)
        : parse_handler(LEXY_MOV(handler)), parse_state(state), //
          vars(nullptr),                                        //
          cur_depth(0), max_depth(static_cast<int>(max_depth)), enable_whitespace_skipping(true)
        {}

        template <typename OtherHandler>
        constexpr parse_context_control_block(Handler&& handler,
                                              parse_context_control_block<OtherHandler, State>* cb)
        : parse_handler(LEXY_MOV(handler)), parse_state(cb->parse_state), //
          vars(cb->vars), cur_depth(cb->cur_depth), max_depth(cb->max_depth),
          enable_whitespace_skipping(cb->enable_whitespace_skipping)
        {}

        template <typename OtherHandler>
        constexpr void copy_vars_from(parse_context_control_block<OtherHandler, State>* cb)
        {
            vars                       = cb->vars;
            cur_depth                  = cb->cur_depth;
            max_depth                  = cb->max_depth;
            enable_whitespace_skipping = cb->enable_whitespace_skipping;
        }
    };
} // namespace _detail

// If a production doesn't define whitespace, we don't need to pass it and can shorten the template
// name.
template <typename Production>
using _whitespace_production_of
    = std::conditional_t<_production_defines_whitespace<Production>, Production, void>;

template <typename Handler, typename State, typename Production>
using _production_value_type =
    typename Handler::template value_callback<Production, State>::return_type;

template <typename Handler, typename State, typename Production,
          typename WhitespaceProduction = _whitespace_production_of<Production>>
struct _pc
{
    using handler_type = Handler;
    using state_type   = State;

    using production            = Production;
    using whitespace_production = WhitespaceProduction;
    using value_type            = _production_value_type<Handler, State, Production>;

    typename Handler::event_handler                       handler;
    _detail::parse_context_control_block<Handler, State>* control_block;
    _detail::lazy_init<value_type>                        value;

    constexpr explicit _pc(_detail::parse_context_control_block<Handler, State>* cb)
    : handler(Production{}), control_block(cb)
    {}

    template <typename ChildProduction>
    constexpr auto sub_context(ChildProduction)
    {
        // Update the whitespace production if necessary.
        // If the current production is a token or defines whitespace,
        // we change it to the current production (or void), otherwise keep it.
        using new_whitespace_production
            = std::conditional_t<is_token_production<ChildProduction> //
                                     || _production_defines_whitespace<ChildProduction>,
                                 _whitespace_production_of<ChildProduction>, WhitespaceProduction>;
        return _pc<Handler, State, ChildProduction, new_whitespace_production>(control_block);
    }

    constexpr auto value_callback()
    {
        using callback = typename Handler::template value_callback<Production, State>;
        return callback(control_block->parse_state);
    }

    template <typename Event, typename... Args>
    constexpr auto on(Event ev, Args&&... args)
    {
        return handler.on(control_block->parse_handler, ev, LEXY_FWD(args)...);
    }
};
} // namespace lexy

//=== do_action ===//
namespace lexy::_detail
{
struct final_parser
{
    template <typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader&, Args&&... args)
    {
        context.value.emplace_result(context.value_callback(), LEXY_FWD(args)...);
        return true;
    }
};

template <typename NextParser>
struct context_finish_parser
{
    template <typename Context, typename Reader, typename SubContext, typename... Args>
    LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, SubContext& sub_context,
                                       Args&&... args)
    {
        // Might need to skip whitespace, according to the original context.
        using continuation
            = std::conditional_t<lexy::is_token_production<typename SubContext::production>,
                                 lexy::whitespace_parser<Context, NextParser>, NextParser>;

        // Pass the produced value to the next parser.
        if constexpr (std::is_void_v<typename SubContext::value_type>)
            return continuation::parse(context, reader, LEXY_FWD(args)...);
        else
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       LEXY_MOV(*sub_context.value));
    }
};
} // namespace lexy::_detail

namespace lexy
{
constexpr void* no_parse_state = nullptr;

template <typename Handler, typename State, typename Production, typename Reader>
constexpr auto _do_action(_pc<Handler, State, Production>& context, Reader& reader)
{
    context.on(parse_events::grammar_start{}, reader.position());
    context.on(parse_events::production_start{}, reader.position());

    // We parse whitespace, theen the rule, then finish.
    using parser = lexy::whitespace_parser<
        LEXY_DECAY_DECLTYPE(context),
        lexy::parser_for<lexy::production_rule<Production>, _detail::final_parser>>;
    auto rule_result = parser::parse(context, reader);

    if (rule_result)
    {
        context.on(parse_events::production_finish{}, reader.position());
        context.on(parse_events::grammar_finish{}, reader);
    }
    else
    {
        context.on(parse_events::production_cancel{}, reader.position());
        context.on(parse_events::grammar_cancel{}, reader);
    }

    return rule_result;
}

template <typename Production, template <typename> typename Result, typename Handler,
          typename State, typename Reader>
constexpr auto do_action(Handler&& handler, State* state, Reader& reader)
{
    static_assert(!std::is_reference_v<Handler>, "need to move handler in");

    _detail::parse_context_control_block control_block(LEXY_MOV(handler), state,
                                                       max_recursion_depth<Production>());
    _pc<Handler, State, Production>      context(&control_block);

    auto rule_result = _do_action(context, reader);

    using value_type = typename decltype(context)::value_type;
    if constexpr (std::is_void_v<value_type>)
        return LEXY_MOV(control_block.parse_handler).template get_result<Result<void>>(rule_result);
    else if (context.value)
        return LEXY_MOV(control_block.parse_handler)
            .template get_result<Result<value_type>>(rule_result, LEXY_MOV(*context.value));
    else
        return LEXY_MOV(control_block.parse_handler)
            .template get_result<Result<value_type>>(rule_result);
}
} // namespace lexy

//=== value callback ===//
namespace lexy::_detail
{
struct void_value_callback
{
    constexpr void_value_callback() = default;
    template <typename State>
    constexpr explicit void_value_callback(State*)
    {}

    using return_type = void;

    constexpr auto sink() const
    {
        return lexy::noop.sink();
    }

    template <typename... Args>
    constexpr void operator()(Args&&...) const
    {}
};
} // namespace lexy::_detail

#endif // LEXY_ACTION_BASE_HPP_INCLUDED

