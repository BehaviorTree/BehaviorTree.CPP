// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/calculator.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/action/match.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/parse_tree_doctest.hpp>

namespace
{
using test_input = lexy::string_input<lexy::utf8_encoding>;

struct test_result
{
    lexy::parse_tree_for<test_input> tree;
    int                              value;
};

test_result eval_expr(const char* str)
{
    auto        input = lexy::zstring_input<lexy::utf8_encoding>(str);
    test_result result;

    auto success = lexy::parse_as_tree<grammar::expr>(result.tree, input, lexy::noop);
    REQUIRE(success);

    ast::environment env;
    result.value = lexy::parse<grammar::expr>(input, lexy::noop).value()->evaluate(env);
    return result;
}

void fail_expr(const char* str)
{
    REQUIRE(!lexy::match<grammar::expr>(lexy::zstring_input<lexy::utf8_encoding>(str)));
}
} // namespace

TEST_CASE("expression")
{
    fail_expr("");

    auto int_decimal = eval_expr("42");
    CHECK(int_decimal.value == 42);
    CHECK(int_decimal.tree == lexy_ext::parse_tree_desc("expr").production("integer").digits("42"));
    auto int_hex = eval_expr("0x42");
    CHECK(int_hex.value == 0x42);
    CHECK(int_hex.tree
          == lexy_ext::parse_tree_desc("expr").production("integer").literal("0x").digits("42"));

    auto var = eval_expr("x");
    CHECK(var.value == 0);
    CHECK(var.tree
          == lexy_ext::parse_tree_desc("expr").production("name").token(lexy::identifier_token_kind,
                                                                        "x"));

    auto call = eval_expr("f(42)");
    CHECK(call.value == 0);
    // clang-format off
    CHECK(call.tree == lexy_ext::parse_tree_desc("expr")
             .production("name")
                 .token(lexy::identifier_token_kind, "f")
                 .finish()
             .literal("(")
             .production("expr")
                .production("integer")
                    .digits("42")
                    .finish()
                .finish()
            .literal(")"));
    // clang-format on

    auto parens = eval_expr("(1)");
    CHECK(parens.value == 1);
    // clang-format off
    CHECK(parens.tree == lexy_ext::parse_tree_desc("expr")
             .literal("(")
             .production("expr")
                 .production("integer")
                     .digits("1")
                     .finish()
                 .finish()
             .literal(")"));
    // clang-format on

    auto power = eval_expr("2**2**3");
    CHECK(power.value == 256);
    // clang-format off
    CHECK(power.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::math_power")
                .production("integer")
                    .digits("2")
                    .finish()
                .literal("**")
                .production("expr::math_power")
                    .production("integer")
                        .digits("2")
                        .finish()
                    .literal("**")
                    .production("integer")
                        .digits("3")
                        .finish());
    // clang-format on
    auto math_prefix = eval_expr("--1");
    CHECK(math_prefix.value == 1);
    // clang-format off
    CHECK(math_prefix.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::math_prefix")
                .literal("-")
                .production("expr::math_prefix")
                    .literal("-")
                    .production("integer")
                        .digits("1")
                        .finish());
    // clang-format on
    auto product = eval_expr("2*6/3");
    CHECK(product.value == 4);
    // clang-format off
    CHECK(product.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::math_product")
                .production("expr::math_product")
                    .production("integer")
                        .digits("2")
                        .finish()
                    .literal("*")
                    .production("integer")
                        .digits("6")
                        .finish()
                    .finish()
                .literal("/")
                .production("integer")
                    .digits("3")
                    .finish());
    // clang-format on
    auto sum = eval_expr("1+2-3");
    CHECK(sum.value == 0);
    // clang-format off
    CHECK(sum.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::math_sum")
                .production("expr::math_sum")
                    .production("integer")
                        .digits("1")
                        .finish()
                    .literal("+")
                    .production("integer")
                        .digits("2")
                        .finish()
                    .finish()
                .literal("-")
                .production("integer")
                    .digits("3")
                    .finish());
    // clang-format on

    auto bit_prefix = eval_expr("~~1");
    CHECK(bit_prefix.value == 1);
    // clang-format off
    CHECK(bit_prefix.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::bit_prefix")
                .literal("~")
                .production("expr::bit_prefix")
                    .literal("~")
                    .production("integer")
                        .digits("1")
                        .finish());
    auto bit_and = eval_expr("6&3&2");
    CHECK(bit_and.value == 2);
    // clang-format off
    CHECK(bit_and.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::bit_and")
                .production("expr::bit_and")
                    .production("integer")
                        .digits("6")
                        .finish()
                    .literal("&")
                    .production("integer")
                        .digits("3")
                        .finish()
                    .finish()
                .literal("&")
                .production("integer")
                    .digits("2")
                    .finish());
    // clang-format on
    auto bit_or = eval_expr("3|6^2");
    CHECK(bit_or.value == 5);
    // clang-format off
    CHECK(bit_or.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::bit_or")
                .production("expr::bit_or")
                    .production("integer")
                        .digits("3")
                        .finish()
                    .literal("|")
                    .production("integer")
                        .digits("6")
                        .finish()
                    .finish()
                .literal("^")
                .production("integer")
                    .digits("2")
                    .finish());
    // clang-format on

    fail_expr("1+2|3");
    fail_expr("-~1");

    auto comparison = eval_expr("1<2==2");
    CHECK(comparison.value == 1);
    // clang-format off
    CHECK(comparison.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::comparison")
                .production("integer")
                    .digits("1")
                    .finish()
                .literal("<")
                .production("integer")
                    .digits("2")
                    .finish()
                .literal("==")
                .production("integer")
                    .digits("2")
                    .finish());
    // clang-format on

    auto conditional = eval_expr("1?2:3");
    CHECK(conditional.value == 2);
    // clang-format off
    CHECK(conditional.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::conditional")
                .production("integer")
                    .digits("1")
                    .finish()
                .literal("?")
                .production("expr")
                    .production("integer")
                        .digits("2")
                        .finish()
                    .finish()
                .literal(":")
                .production("integer")
                    .digits("3")
                    .finish());
    // clang-format on
    fail_expr("1?2:3?4:5");

    auto assignment = eval_expr("x=1");
    CHECK(assignment.value == 1);
    // clang-format off
    CHECK(assignment.tree == lexy_ext::parse_tree_desc("expr")
            .production("expr::assignment")
                .production("name")
                    .token(lexy::identifier_token_kind, "x")
                    .finish()
                .literal("=")
                .production("integer")
                    .digits("1")
                    .finish());
    // clang-format on
    fail_expr("a=b=c");
}

namespace
{
int eval(const char* str)
{
    auto result
        = lexy::parse<grammar::stmt>(lexy::zstring_input<lexy::utf8_encoding>(str), lexy::noop);
    REQUIRE(result);
    auto exprs = result.value();
    if (exprs.empty())
        return 0;

    ast::environment env;
    for (auto i = 0u; i != exprs.size() - 1; ++i)
        exprs[i]->evaluate(env);
    return exprs.back()->evaluate(env);
}
} // namespace

TEST_CASE("stmt")
{
    // Only test complex cases, not handled above.
    CHECK(eval("1 + 2 * 3") == 7);
    CHECK(eval("2 * 2 + 3") == 7);

    CHECK(eval("1 < 2") == 1);
    CHECK(eval("1 < 2 < 3") == 1);

    CHECK(eval("x = 42; x") == 42);
    CHECK(eval("x = 11; 2 * x + 1") == 23);

    CHECK(eval("square(x) = x**2; square(3)") == 9);
    CHECK(eval("fac(n) = n == 0 ? 1 : n * fac(n-1); fac(3)") == 6);
}

