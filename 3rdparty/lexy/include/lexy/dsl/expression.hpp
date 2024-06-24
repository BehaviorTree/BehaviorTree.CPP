// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_EXPRESSION_HPP_INCLUDED
#define LEXY_DSL_EXPRESSION_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/operator.hpp>

// Expression parsing algorithm uses an adapted version of Pratt parsing, as described here:
// https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
// In particular:
// * precedence specified implicitly by type type hierarchy
// * support for list and single precedence
// * support for operator groups that require additional parentheses
// * generate proper parse tree events

//=== dsl ===//
namespace lexyd
{
/// Operation that just parses the atomic rule.
struct atom : _operation_base
{
    static LEXY_CONSTEVAL auto name()
    {
        return "atom";
    }
};

/// Operation that selects between multiple ones.
template <typename... Operands>
struct groups : _operation_base
{};

struct infix_op_left : _operation_base // a ~ b ~ c == (a ~ b) ~ c
{};
struct infix_op_right : _operation_base // a ~ b ~ c == a ~ (b ~ c)
{};
struct infix_op_list : _operation_base // a ~ b ~ c kept as-is
{};
struct infix_op_single : _operation_base // a ~ b ~ c is an error
{};

struct postfix_op : _operation_base
{};

struct prefix_op : _operation_base
{};
} // namespace lexyd

//=== implementation ===//
namespace lexy::_detail
{
struct binding_power
{
    unsigned group;
    unsigned lhs, rhs;

    static constexpr auto left(unsigned group, unsigned level)
    {
        return binding_power{group, 2 * level, 2 * level + 1};
    }
    static constexpr auto right(unsigned group, unsigned level)
    {
        return binding_power{group, 2 * level + 1, 2 * level};
    }
    static constexpr auto prefix(unsigned group, unsigned level)
    {
        // prefix is sort of left-associative, so right side odd.
        return binding_power{group, 0, 2 * level + 1};
    }
    static constexpr auto postfix(unsigned group, unsigned level)
    {
        // postfix is sort of right-associative, so left side odd.
        return binding_power{group, 2 * level + 1, 0};
    }

    constexpr bool is_valid() const
    {
        return lhs > 0 || rhs > 0;
    }
    constexpr bool is_infix() const
    {
        return lhs > 0 && rhs > 0;
    }
    constexpr bool is_postfix() const
    {
        return lhs > 0 && rhs == 0;
    }
    constexpr bool is_prefix() const
    {
        return lhs == 0 && rhs > 0;
    }
};

template <typename Operation>
constexpr binding_power get_binding_power(unsigned cur_group, unsigned cur_level)
{
    if constexpr (std::is_base_of_v<lexyd::infix_op_left, Operation>
                  // We treat a list as left associative operator for simplicity here.
                  // It doesn't really matter, as it will only consider operators from the
                  // same operation anyway.
                  || std::is_base_of_v<lexyd::infix_op_list, Operation>
                  // For the purposes of error recovery, single is left associative.
                  || std::is_base_of_v<lexyd::infix_op_single, Operation>)
        return binding_power::left(cur_group, cur_level);
    else if constexpr (std::is_base_of_v<lexyd::infix_op_right, Operation>)
        return binding_power::right(cur_group, cur_level);
    else if constexpr (std::is_base_of_v<lexyd::prefix_op, Operation>)
        return binding_power::prefix(cur_group, cur_level);
    else if constexpr (std::is_base_of_v<lexyd::postfix_op, Operation>)
        return binding_power::postfix(cur_group, cur_level);
}

template <typename DestOperation>
struct _binding_power_of
{
    static constexpr auto transition(lexyd::atom, unsigned cur_group, unsigned)
    {
        // Not found, return an invalid operator, but return the current group.
        // This is the highest group encountered.
        return binding_power{cur_group, 0, 0};
    }
    template <typename CurOperation>
    static constexpr auto transition(CurOperation op, unsigned cur_group, unsigned cur_level)
    {
        // Normal operation: keep group the same, but increment level.
        return get(op, cur_group, cur_level + 1);
    }
    template <typename... Operations>
    static constexpr auto transition(lexyd::groups<Operations...>, unsigned cur_group,
                                     unsigned cur_level)
    {
        auto result = binding_power{cur_group, 0, 0};
        // Try to find the destination in each group.
        // Before we transition, we increment the group to create a new one,
        // afterwards we update group to the highest group encountered so far.
        // That way, we don't re-use group numbers.
        // Note that we don't increment the level, as that is handled by the overload above.
        (void)((result    = transition(Operations{}, cur_group + 1, cur_level),
                cur_group = result.group, result.is_valid())
               || ...);
        return result;
    }

    template <typename CurOperation>
    static constexpr auto get(CurOperation, unsigned cur_group, unsigned cur_level)
    {
        if constexpr (std::is_same_v<DestOperation, CurOperation>)
            return get_binding_power<CurOperation>(cur_group, cur_level);
        else
            return transition(typename CurOperation::operand{}, cur_group, cur_level);
    }
};

// Returns the binding power of an operator in an expression.
template <typename Expr, typename Operation>
constexpr auto binding_power_of(Operation)
{
    return _binding_power_of<Operation>::transition(typename Expr::operation{}, 0, 0);
}
} // namespace lexy::_detail

namespace lexy::_detail
{
template <typename Operation>
using op_of = LEXY_DECAY_DECLTYPE(Operation::op);

template <typename... Operations>
struct operation_list
{
    template <typename T>
    constexpr auto operator+(T) const
    {
        return operation_list<Operations..., T>{};
    }
    template <typename... T>
    constexpr auto operator+(operation_list<T...>) const
    {
        return operation_list<Operations..., T...>{};
    }

    static constexpr auto size = sizeof...(Operations);

    using ops = decltype((typename op_of<Operations>::op_literals{} + ... + op_lit_list{}));

    template <template <typename> typename Continuation, typename Context, typename Reader,
              typename... Args>
    static constexpr bool apply(Context& context, Reader& reader, parsed_operator<Reader> op,
                                Args&&... args)
    {
        auto result = false;

        auto cur_idx = std::size_t(0);
        (void)((cur_idx <= op.idx && op.idx < cur_idx + op_of<Operations>::op_literals::size
                    ? (result
                       = Continuation<Operations>::parse(context, reader,
                                                         parsed_operator<Reader>{op.cur,
                                                                                 op.idx - cur_idx},
                                                         LEXY_FWD(args)...),
                       true)
                    : (cur_idx += op_of<Operations>::op_literals::size, false))
               || ...);

        return result;
    }
};

template <bool Pre, unsigned MinBindingPower>
struct _operation_list_of
{
    template <unsigned CurLevel>
    static constexpr auto get(lexyd::atom)
    {
        return operation_list{};
    }
    template <unsigned CurLevel, typename... Operations>
    static constexpr auto get(lexyd::groups<Operations...>)
    {
        return (get<CurLevel>(Operations{}) + ...);
    }
    template <unsigned CurLevel, typename Operation>
    static constexpr auto get(Operation)
    {
        constexpr auto bp = get_binding_power<Operation>(0, CurLevel);

        auto           tail      = get<CurLevel + 1>(typename Operation::operand{});
        constexpr auto is_prefix = std::is_base_of_v<lexyd::prefix_op, Operation>;
        if constexpr (is_prefix == Pre
                      && ((is_prefix && bp.rhs >= MinBindingPower)
                          || (!is_prefix && bp.lhs >= MinBindingPower)))
            return tail + Operation{};
        else
            return tail;
    }
};

// prefix operations
template <typename Expr, unsigned MinBindingPower>
using pre_operation_list_of = decltype(_operation_list_of<true, MinBindingPower>::template get<1>(
    typename Expr::operation{}));

// infix and postfix operations
template <typename Expr, unsigned MinBindingPower>
using post_operation_list_of = decltype(_operation_list_of<false, MinBindingPower>::template get<1>(
    typename Expr::operation{}));
} // namespace lexy::_detail

//=== expression rule ===//
namespace lexyd
{
template <typename RootOperation>
struct _expr : rule_base
{
    struct _state
    {
        unsigned cur_group         = 0;
        unsigned cur_nesting_level = 0;
    };

    template <typename Operation>
    struct _continuation
    {
        struct _op_cont
        {
            template <typename Context, typename Reader, typename... Args>
            LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, _state& state,
                                               Args&&... op_args)
            {
                using namespace lexy::_detail;

                constexpr auto value_type_void = std::is_void_v<typename Context::value_type>;
                constexpr auto binding_power
                    = binding_power_of<typename Context::production>(Operation{});

                if constexpr (std::is_base_of_v<infix_op_list, Operation>)
                {
                    // We need to handle a list infix operator specially,
                    // and parse an arbitrary amount of lhs.
                    // For that, we use a loop and a sink.
                    auto sink = context.value_callback().sink();

                    // We need to pass the initial lhs to the sink.
                    if constexpr (!value_type_void)
                        sink(*LEXY_MOV(context.value));
                    context.value = {};

                    // As well as the operator we've already got.
                    sink(LEXY_FWD(op_args)...);

                    auto result = true;
                    while (true)
                    {
                        // Parse (another) value.
                        if (!_parse<binding_power.rhs>(context, reader, state))
                        {
                            result = false;
                            break;
                        }

                        if constexpr (!value_type_void)
                            sink(*LEXY_MOV(context.value));
                        context.value = {};

                        using op_rule = op_of<Operation>;
                        auto op       = parse_operator<typename op_rule::op_literals>(reader);
                        if (op.idx >= op_rule::op_literals::size)
                        {
                            // The list ends at this point.
                            reader.reset(op.cur);
                            break;
                        }

                        // Need to finish the operator properly, by passing it to the sink.
                        if (!op_rule::template op_finish<lexy::sink_parser>(context, reader, op,
                                                                            sink))
                        {
                            result = false;
                            break;
                        }
                    }

                    // We store the final value of the sink no matter the parse result.
                    if constexpr (value_type_void)
                    {
                        LEXY_MOV(sink).finish();
                        context.value.emplace();
                    }
                    else
                    {
                        context.value.emplace(LEXY_MOV(sink).finish());
                    }

                    // If we've failed at any point, propagate failure now.
                    if (!result)
                        return false;
                }
                else if constexpr (binding_power.is_prefix())
                {
                    if (!_parse<binding_power.rhs>(context, reader, state))
                        return false;

                    auto value    = LEXY_MOV(context.value);
                    context.value = {};

                    if constexpr (value_type_void)
                        context.value.emplace_result(context.value_callback(),
                                                     LEXY_FWD(op_args)...);
                    else
                        context.value.emplace_result(context.value_callback(), LEXY_FWD(op_args)...,
                                                     *LEXY_MOV(value));
                }
                else if constexpr (binding_power.is_infix())
                {
                    auto lhs      = LEXY_MOV(context.value);
                    context.value = {};

                    if (!_parse<binding_power.rhs>(context, reader, state))
                    {
                        // Put it back, so we can properly recover.
                        context.value = LEXY_MOV(lhs);
                        return false;
                    }

                    auto rhs      = LEXY_MOV(context.value);
                    context.value = {};

                    if constexpr (value_type_void)
                        context.value.emplace_result(context.value_callback(),
                                                     LEXY_FWD(op_args)...);
                    else
                        context.value.emplace_result(context.value_callback(), *LEXY_MOV(lhs),
                                                     LEXY_FWD(op_args)..., *LEXY_MOV(rhs));

                    if constexpr (std::is_base_of_v<infix_op_single, Operation>)
                    {
                        using op_rule = op_of<Operation>;
                        auto op       = parse_operator<typename op_rule::op_literals>(reader);
                        if (op.idx < op_rule::op_literals::size)
                        {
                            using tag = typename Context::production::operator_chain_error;
                            auto err
                                = lexy::error<Reader, tag>(op.cur.position(), reader.position());
                            context.on(_ev::error{}, err);
                        }
                        reader.reset(op.cur);
                    }
                }
                else if constexpr (binding_power.is_postfix())
                {
                    auto value    = LEXY_MOV(context.value);
                    context.value = {};

                    if constexpr (value_type_void)
                        context.value.emplace_result(context.value_callback(),
                                                     LEXY_FWD(op_args)...);
                    else
                        context.value.emplace_result(context.value_callback(), *LEXY_MOV(value),
                                                     LEXY_FWD(op_args)...);
                }

                context.on(_ev::operation_chain_op{}, Operation{}, reader.position());
                return true;
            }
        };

        template <typename Context, typename Reader>
        static constexpr bool parse(Context& context, Reader& reader,
                                    lexy::_detail::parsed_operator<Reader> op, _state& state)
        {
            using namespace lexy::_detail;
            using production = typename Context::production;

            // Check whether we might have nested to far.
            if (state.cur_nesting_level++ >= production::max_operator_nesting)
            {
                using tag = typename production::operator_nesting_error;
                auto err  = lexy::error<Reader, tag>(op.cur.position(), reader.position());
                context.on(_ev::error{}, err);

                // We do not recover, to prevent stack overflow.
                reader.reset(op.cur);
                return false;
            }

            // If the operator is part of a group, check whether it matches.
            constexpr auto binding_power = binding_power_of<production>(Operation{});
            if constexpr (binding_power.group != 0)
            {
                if (state.cur_group == 0)
                {
                    // We didn't have any operator group yet, set it.
                    state.cur_group = binding_power.group;
                }
                else if (state.cur_group != binding_power.group)
                {
                    // Operators can't be grouped.
                    using tag = typename production::operator_group_error;
                    auto err  = lexy::error<Reader, tag>(op.cur.position(), reader.position());
                    context.on(_ev::error{}, err);
                    // Trivially recover, but don't update group:
                    // let the first one stick.
                }
            }

            // Finish the operator and parse a RHS, if necessary.
            return op_of<Operation>::template op_finish<_op_cont>(context, reader, op, state);
        }
    };

    template <unsigned MinBindingPower, typename Context, typename Reader>
    static constexpr bool _parse_lhs(Context& context, Reader& reader, _state& state)
    {
        using namespace lexy::_detail;

        using op_list = pre_operation_list_of<typename Context::production, MinBindingPower>;
        using atom_parser
            = lexy::parser_for<LEXY_DECAY_DECLTYPE(Context::production::atom), final_parser>;

        if constexpr (op_list::size == 0)
        {
            // We don't have any prefix operators, so parse an atom directly.
            (void)state;
            return atom_parser::parse(context, reader);
        }
        else
        {
            auto op = lexy::_detail::parse_operator<typename op_list::ops>(reader);
            if (op.idx >= op_list::ops::size)
            {
                // We don't have a prefix operator, so it must be an atom.
                reader.reset(op.cur);
                return atom_parser::parse(context, reader);
            }

            auto start_event = context.on(_ev::operation_chain_start{}, op.cur.position());
            auto result      = op_list::template apply<_continuation>(context, reader, op, state);
            context.on(_ev::operation_chain_finish{}, LEXY_MOV(start_event), reader.position());
            return result;
        }
    }

    template <unsigned MinBindingPower, typename Context, typename Reader>
    static constexpr bool _parse(Context& context, Reader& reader, _state& state)
    {
        using namespace lexy::_detail;
        using op_list = post_operation_list_of<typename Context::production, MinBindingPower>;

        if constexpr (op_list::size == 0)
        {
            // We don't have any post operators, so we only parse the left-hand-side.
            return _parse_lhs<MinBindingPower>(context, reader, state);
        }
        else
        {
            auto start_event = context.on(_ev::operation_chain_start{}, reader.position());
            if (!_parse_lhs<MinBindingPower>(context, reader, state))
            {
                context.on(_ev::operation_chain_finish{}, LEXY_MOV(start_event), reader.position());
                return false;
            }

            auto result = true;
            while (true)
            {
                auto op = parse_operator<typename op_list::ops>(reader);
                if (op.idx >= op_list::ops::size)
                {
                    reader.reset(op.cur);
                    break;
                }

                result = op_list::template apply<_continuation>(context, reader, op, state);
                if (!result)
                    break;
            }

            context.on(_ev::operation_chain_finish{}, LEXY_MOV(start_event), reader.position());
            return result;
        }

        return false; // unreachable
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader)
        {
            static_assert(std::is_same_v<NextParser, lexy::_detail::final_parser>);

            using production             = typename Context::production;
            constexpr auto binding_power = lexy::_detail::binding_power_of<production>(
                lexy::_detail::type_or<RootOperation, typename production::operation>{});
            // The MinBindingPower is determined by the root operation.
            // The initial operand is always on the left, so we use the left binding power.
            // However, for a prefix operator it is zero, but then it's a right operand so we use
            // that.
            constexpr auto min_binding_power
                = binding_power.is_prefix() ? binding_power.rhs : binding_power.lhs;

            _state state;
            _parse<min_binding_power>(context, reader, state);

            // Regardless of parse errors, we can recover if we already had a value at some point.
            return !!context.value;
        }
    };
};
} // namespace lexyd

//=== expression_production ===//
namespace lexy
{
struct max_operator_nesting_exceeded
{
    static LEXY_CONSTEVAL auto name()
    {
        return "maximum operator nesting level exceeded";
    }
};

struct operator_chain_error
{
    static LEXY_CONSTEVAL auto name()
    {
        return "operator cannot be chained";
    }
};

struct operator_group_error
{
    static LEXY_CONSTEVAL auto name()
    {
        return "operator cannot be mixed with previous operators";
    }
};

struct expression_production
{
    using operator_nesting_error               = lexy::max_operator_nesting_exceeded;
    static constexpr auto max_operator_nesting = 256; // arbitrary power of two

    using operator_chain_error = lexy::operator_chain_error;
    using operator_group_error = lexy::operator_group_error;

    static constexpr auto rule = lexyd::_expr<void>{};
};

template <typename Expr, typename RootOperation>
struct subexpression_production : Expr
{
    static constexpr auto rule = lexyd::_expr<RootOperation>{};
};
} // namespace lexy

#endif // LEXY_DSL_EXPRESSION_HPP_INCLUDED

