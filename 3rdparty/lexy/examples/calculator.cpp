// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <cmath>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

namespace
{
// Naive implementation of an AST with simple evaluation function.
namespace ast
{
    using expr_ptr = std::shared_ptr<struct expr>;

    struct environment
    {
        struct function
        {
            int*     parameter_var;
            expr_ptr body;
        };

        std::map<std::string, int>      vars;
        std::map<std::string, function> fns;
    };

    struct expr
    {
        virtual ~expr()                              = default;
        virtual int evaluate(environment& env) const = 0;
    };

    struct expr_literal : expr
    {
        int value;

        explicit expr_literal(int v) : value(v) {}

        int evaluate(environment&) const override
        {
            return value;
        }
    };

    struct expr_name : expr
    {
        std::string name;

        explicit expr_name(std::string n) : name(LEXY_MOV(n)) {}

        int evaluate(environment& env) const override
        {
            auto iter = env.vars.find(name);
            // A real compiler would issue an error message at this point.
            return iter == env.vars.end() ? 0 : iter->second;
        }
    };

    struct expr_unary_arithmetic : expr
    {
        enum op_t
        {
            negate,
            complement,
        } op;
        expr_ptr rhs;

        explicit expr_unary_arithmetic(op_t op, expr_ptr e) : op(op), rhs(LEXY_MOV(e)) {}

        int evaluate(environment& env) const override
        {
            auto rhs_v = rhs->evaluate(env);
            switch (op)
            {
            case negate:
                return -rhs_v;
            case complement:
                return ~rhs_v;
            }
            return 0; // unreachable
        }
    };

    struct expr_binary_arithmetic : expr
    {
        enum op_t
        {
            plus,
            minus,
            times,
            div,
            pow,
            bit_and,
            bit_or,
            bit_xor,
        } op;
        expr_ptr lhs, rhs;

        explicit expr_binary_arithmetic(expr_ptr lhs, op_t op, expr_ptr rhs)
        : op(op), lhs(LEXY_MOV(lhs)), rhs(LEXY_MOV(rhs))
        {}

        int evaluate(environment& env) const override
        {
            auto lhs_v = lhs->evaluate(env);
            auto rhs_v = rhs->evaluate(env);
            switch (op)
            {
            case plus:
                return lhs_v + rhs_v;
            case minus:
                return lhs_v - rhs_v;
            case times:
                return lhs_v * rhs_v;
            case div:
                return lhs_v / rhs_v;
            case pow:
                return static_cast<int>(std::pow(lhs_v, rhs_v));
            case bit_and:
                return lhs_v & rhs_v;
            case bit_or:
                return lhs_v | rhs_v;
            case bit_xor:
                return lhs_v ^ rhs_v;
            }

            return 0; // unreachable
        }
    };

    struct expr_comparison : expr
    {
        enum op_t
        {
            equal,
            less,
        };
        std::vector<op_t>     ops;
        std::vector<expr_ptr> operands;

        int evaluate(environment& env) const override
        {
            auto lhs = operands[0]->evaluate(env);
            for (auto i = 0u; i != ops.size(); ++i)
            {
                auto rhs = operands[i + 1]->evaluate(env);
                switch (ops[i])
                {
                case equal:
                    if (lhs != rhs)
                        return 0;
                    break;
                case less:
                    if (lhs >= rhs)
                        return 0;
                    break;
                }
                lhs = rhs;
            }
            return 1;
        }
    };

    struct expr_if : expr
    {
        expr_ptr condition, then, else_;

        explicit expr_if(expr_ptr condition, expr_ptr then, expr_ptr else_)
        : condition(LEXY_MOV(condition)), then(LEXY_MOV(then)), else_(LEXY_MOV(else_))
        {}

        int evaluate(environment& env) const override
        {
            if (condition->evaluate(env) != 0)
                return then->evaluate(env);
            else
                return else_->evaluate(env);
        }
    };

    struct expr_call : expr
    {
        std::string function;
        expr_ptr    argument;

        explicit expr_call(std::string function, expr_ptr argument)
        : function(LEXY_MOV(function)), argument(LEXY_MOV(argument))
        {}

        int evaluate(environment& env) const override
        {
            auto iter = env.fns.find(function);
            if (iter == env.fns.end())
                return 0;
            auto& [param, body] = iter->second;

            auto old_value = std::exchange(*param, argument->evaluate(env));
            auto result    = body->evaluate(env);
            *param         = old_value;
            return result;
        }
    };

    struct expr_assignment : expr
    {
        expr_ptr lhs, rhs;

        explicit expr_assignment(expr_ptr lhs, expr_ptr rhs)
        : lhs(LEXY_MOV(lhs)), rhs(LEXY_MOV(rhs))
        {}

        int evaluate(environment& env) const override
        {
            if (auto name = dynamic_cast<expr_name*>(lhs.get()))
            {
                // Variable assignment.
                return env.vars[name->name] = rhs->evaluate(env);
            }
            else if (auto call = dynamic_cast<expr_call*>(lhs.get()))
            {
                // Function assignment.
                auto param = dynamic_cast<expr_name*>(call->argument.get());
                if (param == nullptr)
                    std::fputs("error: function parameter not a name\n", stderr);
                else
                    env.fns[call->function] = environment::function{&env.vars[param->name], rhs};
                return 0;
            }
            else
            {
                std::fputs("error: assignment lhs not an lvalue\n", stderr);
                return 0;
            }
        }
    };
} // namespace ast

namespace grammar
{
    namespace dsl = lexy::dsl;

    constexpr auto escaped_newline = dsl::backslash >> dsl::newline;

    // An integer literal.
    // Supports hexadecimal and decimal literals.
    struct integer : lexy::token_production
    {
        static constexpr auto rule
            = LEXY_LIT("0x") >> dsl::integer<int, dsl::hex> | dsl::integer<int>;

        static constexpr auto value = lexy::forward<int>;
    };

    // A Unicode-aware identifier.
    struct name
    {
        static constexpr auto rule
            = dsl::identifier(dsl::unicode::xid_start_underscore, dsl::unicode::xid_continue);

        static constexpr auto value = lexy::as_string<std::string>;
    };

    // An expression that is nested inside another expression.
    struct nested_expr : lexy::transparent_production
    {
        // We change the whitespace rule to allow newlines:
        // as it's nested, the REPL can properly handle continuation lines.
        static constexpr auto whitespace = dsl::ascii::space | escaped_newline;
        // The rule itself just recurses back to expression, but with the adjusted whitespace now.
        static constexpr auto rule = dsl::recurse<struct expr>;

        static constexpr auto value = lexy::forward<ast::expr_ptr>;
    };

    // An arbitrary expression.
    // It uses lexy's built-in support for operator precedence parsing to automatically generate a
    // proper rule. This is done by inheriting from expression_production.
    struct expr : lexy::expression_production
    {
        struct expected_operand
        {
            static constexpr auto name = "expected operand";
        };

        // We need to specify the atomic part of an expression.
        static constexpr auto atom = [] {
            auto paren_expr = dsl::parenthesized(dsl::p<nested_expr>);
            // Functions can only have a single argument for simplicity.
            auto var_or_call = dsl::p<name> >> dsl::if_(paren_expr);
            auto literal     = dsl::p<integer>;

            return paren_expr | var_or_call | literal | dsl::error<expected_operand>;
        }();

        // Each of the nested classes defines one operation.
        // They inherit from a tag type that specify the kind of operation (prefix, infix, postfix),
        // and associativy (left, right, single (non-associative)),
        // and specify the operator rule and operand.

        // x**2
        struct math_power : dsl::infix_op_right
        {
            static constexpr auto op = dsl::op<ast::expr_binary_arithmetic::pow>(LEXY_LIT("**"));

            // math_power has highest binding power, so it's operand is the atom rule.
            using operand = dsl::atom;
        };
        // -x
        struct math_prefix : dsl::prefix_op
        {
            static constexpr auto op = dsl::op<ast::expr_unary_arithmetic::negate>(LEXY_LIT("-"));

            using operand = math_power;
        };
        // x * x, x / x
        struct math_product : dsl::infix_op_left
        {
            static constexpr auto op = [] {
                // Since we have both ** and * as possible operators, we need to ensure that * is
                // only matched when it is not followed by *. In the particular situation where **
                // has higher binding power, it doesn't actually matter here, but it's better to be
                // more resilient.
                auto star = dsl::not_followed_by(LEXY_LIT("*"), dsl::lit_c<'*'>);
                return dsl::op<ast::expr_binary_arithmetic::times>(star)
                       / dsl::op<ast::expr_binary_arithmetic::div>(LEXY_LIT("/"));
            }();
            using operand = math_prefix;
        };
        // x + x, x - x
        struct math_sum : dsl::infix_op_left
        {
            static constexpr auto op = dsl::op<ast::expr_binary_arithmetic::plus>(LEXY_LIT("+"))
                                       / dsl::op<ast::expr_binary_arithmetic::minus>(LEXY_LIT("-"));

            using operand = math_product;
        };

        // ~x
        struct bit_prefix : dsl::prefix_op
        {
            static constexpr auto op
                = dsl::op<ast::expr_unary_arithmetic::complement>(LEXY_LIT("~"));
            using operand = dsl::atom;
        };
        // x & x
        struct bit_and : dsl::infix_op_left
        {
            static constexpr auto op = dsl::op<ast::expr_binary_arithmetic::bit_and>(LEXY_LIT("&"));
            using operand            = bit_prefix;
        };
        // x | x, x ^ x
        struct bit_or : dsl::infix_op_left
        {
            static constexpr auto op
                = dsl::op<ast::expr_binary_arithmetic::bit_or>(LEXY_LIT("|"))
                  / dsl::op<ast::expr_binary_arithmetic::bit_xor>(LEXY_LIT("^"));
            using operand = bit_and;
        };

        // Comparisons are list operators, which allows implementation of chaining.
        // x == y < z
        struct comparison : dsl::infix_op_list
        {
            // Other comparison operators omitted for simplicity.
            static constexpr auto op = dsl::op<ast::expr_comparison::equal>(LEXY_LIT("=="))
                                       / dsl::op<ast::expr_comparison::less>(LEXY_LIT("<"));

            // The use of dsl::groups ensures that an expression can either contain math or bit
            // operators. Mixing requires parenthesis.
            using operand = dsl::groups<math_sum, bit_or>;
        };

        // x ? y : z
        struct conditional : dsl::infix_op_single
        {
            // We treat a conditional operator, which has three operands,
            // as a binary operator where the operator consists of ?, the inner operator, and :.
            // The <void> ensures that `dsl::op` does not produce a value.
            static constexpr auto op
                = dsl::op<void>(LEXY_LIT("?") >> dsl::p<nested_expr> + dsl::lit_c<':'>);
            using operand = comparison;
        };

        struct assignment : dsl::infix_op_single
        {
            static constexpr auto op
                // Similar to * above, we need to prevent `=` from matching `==`.
                = dsl::op<void>(dsl::not_followed_by(LEXY_LIT("="), dsl::lit_c<'='>));
            using operand = conditional;
        };

        // An expression also needs to specify the operation with the lowest binding power.
        // The operation of everything else is determined by following the `::operand` member.
        using operation = assignment;

        static constexpr auto value =
            // We need a sink as the comparison expression generates a list.
            lexy::fold_inplace<std::unique_ptr<ast::expr_comparison>>(
                [] { return std::make_unique<ast::expr_comparison>(); },
                [](auto& node, ast::expr_ptr opr) { node->operands.push_back(LEXY_MOV(opr)); },
                [](auto& node, ast::expr_comparison::op_t op) { node->ops.push_back(op); })
            // The result of the list feeds into a callback that handles all other cases.
            >> lexy::callback(
                // atoms
                lexy::forward<ast::expr_ptr>, lexy::new_<ast::expr_literal, ast::expr_ptr>,
                lexy::new_<ast::expr_name, ast::expr_ptr>,
                lexy::new_<ast::expr_call, ast::expr_ptr>,
                // unary/binary operators
                lexy::new_<ast::expr_unary_arithmetic, ast::expr_ptr>,
                lexy::new_<ast::expr_binary_arithmetic, ast::expr_ptr>,
                // conditional and assignment
                lexy::new_<ast::expr_if, ast::expr_ptr>,
                lexy::new_<ast::expr_assignment, ast::expr_ptr>);
    };

    // A statement, which is a list of expressions separated by semicolons.
    struct stmt
    {
        // We don't allow newlines as whitespace at the top-level.
        // This is because we can't easily know whether we need to request more input when seeing a
        // newline or not. Once we're having a e.g. parenthesized expression, we know that we need
        // more input until we've reached ), so then change the whitespace rule.
        static constexpr auto whitespace = dsl::ascii::blank | escaped_newline;

        static constexpr auto rule = [] {
            // We can't use `dsl::eol` as our terminator directly,
            // since that would try and skip whitespace, which requests more input on the REPL.
            auto at_eol = dsl::peek(dsl::eol);
            return dsl::terminator(at_eol).opt_list(dsl::p<expr>, dsl::sep(dsl::semicolon));
        }();

        static constexpr auto value = lexy::as_list<std::vector<ast::expr_ptr>>;
    };
} // namespace grammar
} // namespace

#ifndef LEXY_TEST
#    include <lexy/action/parse.hpp>
#    include <lexy_ext/report_error.hpp>
#    include <lexy_ext/shell.hpp>

int main()
{
    ast::environment environment;
    // We create an interactive REPL and use it for our input.
    for (lexy_ext::shell<lexy_ext::default_prompt<lexy::utf8_encoding>> shell; shell.is_open();)
    {
        auto input = shell.prompt_for_input();

        auto result = lexy::parse<grammar::stmt>(input, lexy_ext::report_error);
        if (result.has_value() && !result.value().empty())
        {
            auto exprs = LEXY_MOV(result).value();
            for (auto i = 0u; i < exprs.size() - 1; ++i)
                exprs[i]->evaluate(environment);

            auto value = exprs.back()->evaluate(environment);
            std::printf("= %d\n", value);
        }
    }
}
#endif

