// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED
#define LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/action/validate.hpp>
#include <lexy/dsl/any.hpp>
#include <lexy/parse_tree.hpp>

namespace lexy
{
template <typename Tree, typename Input, typename ErrorCallback>
class parse_tree_handler
{
public:
    explicit parse_tree_handler(Tree& tree, const Input& input, const ErrorCallback& cb)
    : _tree(&tree), _depth(0), _validate(input, cb)
    {}

    template <typename Production>
    class event_handler
    {
        using iterator = typename lexy::input_reader<Input>::iterator;

    public:
        void on(parse_tree_handler& handler, parse_events::production_start ev, iterator pos)
        {
            if (handler._depth++ == 0)
                handler._builder.emplace(LEXY_MOV(*handler._tree), Production{});
            else
                _marker = handler._builder->start_production(Production{});

            _validate.on(handler._validate, ev, pos);
        }

        void on(parse_tree_handler& handler, parse_events::production_finish, iterator pos)
        {
            if (--handler._depth == 0)
            {
                auto reader = handler._validate.input().reader();
                reader.set_position(pos);
                lexy::try_match_token(dsl::any, reader);
                auto end = reader.position();

                *handler._tree = LEXY_MOV(*handler._builder).finish({pos, end});
            }
            else
            {
                handler._builder->finish_production(LEXY_MOV(_marker));
            }
        }

        void on(parse_tree_handler& handler, parse_events::production_cancel, iterator pos)
        {
            if (--handler._depth == 0)
            {
                handler._tree->clear();
            }
            else
            {
                // Cancelling the production removes all nodes from the tree.
                // To ensure that the parse tree remains lossless, we add everything consumed by it
                // as an error token.
                handler._builder->cancel_production(LEXY_MOV(_marker));
                handler._builder->token(lexy::error_token_kind, _validate.production_begin(), pos);
            }
        }

        auto on(parse_tree_handler& handler, lexy::parse_events::operation_chain_start, iterator)
        {
            // As we don't know the production yet (or whether it is actually an operation),
            // we create a container node to decide later.
            return handler._builder->start_container();
        }
        template <typename Operation>
        void on(parse_tree_handler& handler, lexy::parse_events::operation_chain_op, Operation op,
                iterator)
        {
            // We set the production of the current container.
            // This will do a "left rotation" on the parse tree, making a new container the parent.
            handler._builder->set_container_production(op);
        }
        template <typename Marker>
        void on(parse_tree_handler& handler, lexy::parse_events::operation_chain_finish,
                Marker&&            marker, iterator)
        {
            handler._builder->finish_container(LEXY_MOV(marker));
        }

        template <typename TokenKind>
        void on(parse_tree_handler& handler, parse_events::token, TokenKind kind, iterator begin,
                iterator end)
        {
            handler._builder->token(kind, begin, end);
        }

        template <typename Error>
        void on(parse_tree_handler& handler, parse_events::error ev, Error&& error)
        {
            _validate.on(handler._validate, ev, LEXY_FWD(error));
        }

        template <typename Event, typename... Args>
        auto on(parse_tree_handler& handler, Event ev, Args&&... args)
        {
            return _validate.on(handler._validate, ev, LEXY_FWD(args)...);
        }

    private:
        typename Tree::builder::marker _marker;
        typename validate_handler<Input, ErrorCallback>::template event_handler<Production>
            _validate;
    };

    template <typename Production, typename State>
    using value_callback = _detail::void_value_callback;

    constexpr auto get_result_void(bool rule_parse_result) &&
    {
        LEXY_PRECONDITION(_depth == 0);
        return LEXY_MOV(_validate).get_result_void(rule_parse_result);
    }

private:
    lexy::_detail::lazy_init<typename Tree::builder> _builder;
    Tree*                                            _tree;
    int                                              _depth;

    validate_handler<Input, ErrorCallback> _validate;
};

template <typename State, typename Input, typename ErrorCallback, typename TokenKind = void,
          typename MemoryResource = void>
struct parse_as_tree_action
{
    using tree_type = lexy::parse_tree_for<Input, TokenKind, MemoryResource>;

    tree_type*           _tree;
    const ErrorCallback* _callback;
    State*               _state = nullptr;

    using handler = parse_tree_handler<tree_type, Input, ErrorCallback>;
    using state   = State;
    using input   = Input;

    constexpr explicit parse_as_tree_action(tree_type& tree, const ErrorCallback& callback)
    : _tree(&tree), _callback(&callback)
    {}
    template <typename U = State>
    constexpr explicit parse_as_tree_action(U& state, tree_type& tree,
                                            const ErrorCallback& callback)
    : _tree(&tree), _callback(&callback), _state(&state)
    {}

    template <typename Production>
    constexpr auto operator()(Production, const Input& input) const
    {
        auto reader = input.reader();
        return lexy::do_action<Production>(handler(*_tree, input, *_callback), _state, reader);
    }
};

template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    return parse_as_tree_action<void, Input, ErrorCallback, TokenKind,
                                MemoryResource>(tree, callback)(Production{}, input);
}
template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename State, typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, State& state, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    return parse_as_tree_action<State, Input, ErrorCallback, TokenKind,
                                MemoryResource>(state, tree, callback)(Production{}, input);
}
template <typename Production, typename TokenKind, typename MemoryResource, typename Input,
          typename State, typename ErrorCallback>
auto parse_as_tree(parse_tree<lexy::input_reader<Input>, TokenKind, MemoryResource>& tree,
                   const Input& input, const State& state, const ErrorCallback& callback)
    -> validate_result<ErrorCallback>
{
    return parse_as_tree_action<const State, Input, ErrorCallback, TokenKind,
                                MemoryResource>(state, tree, callback)(Production{}, input);
}
} // namespace lexy

#endif // LEXY_ACTION_PARSE_AS_TREE_HPP_INCLUDED

