// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/expression.hpp>

#include "verify.hpp"
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/callback/composition.hpp>
#include <lexy/callback/fold.hpp>
#include <lexy/dsl/integer.hpp>
#include <lexy_ext/parse_tree_doctest.hpp>

namespace
{
template <typename Tree>
struct op_result : test_result
{
    Tree tree;
};

constexpr auto error_callback = lexy::callback([](auto&&...) {});
using test_tree               = lexy_ext::parse_tree_desc<>;
} // namespace

#define LEXY_OP_VERIFY(...)                                                                        \
    [&] {                                                                                          \
        constexpr auto                        input = lexy_test::_get_input(__VA_ARGS__);          \
        lexy::parse_tree_for<decltype(input)> tree;                                                \
        lexy::parse_as_tree<prod>(tree, input, error_callback);                                    \
        return op_result<decltype(tree)>{LEXY_VERIFY_P(prod, __VA_ARGS__), LEXY_MOV(tree)};        \
    }()

namespace
{
constexpr auto integer = dsl::integer<int>;
}

namespace single_operation
{
constexpr auto op_minus = dsl::op(dsl::lit_c<'-'>);

struct infix_left : lexy::expression_production, test_production
{
    static constexpr auto max_operator_nesting = 2;
    static constexpr auto atom                 = integer;

    struct operation : dsl::infix_op_left
    {
        static constexpr auto name = "op";
        static constexpr auto op   = op_minus;
        using operand              = dsl::atom;
    };
};
struct infix_right : lexy::expression_production, test_production
{
    static constexpr auto max_operator_nesting = 2;
    static constexpr auto atom                 = integer;

    struct operation : dsl::infix_op_right
    {
        static constexpr auto name = "op";
        static constexpr auto op   = op_minus;
        using operand              = dsl::atom;
    };
};
struct infix_list : lexy::expression_production, test_production
{
    static constexpr auto max_operator_nesting = 2;
    static constexpr auto atom                 = integer;

    struct operation : dsl::infix_op_list
    {
        static constexpr auto name = "op";
        static constexpr auto op   = op_minus;
        using operand              = dsl::atom;
    };
};
struct infix_single : lexy::expression_production, test_production
{
    static constexpr auto max_operator_nesting = 2;
    static constexpr auto atom                 = integer;

    struct operation : dsl::infix_op_single
    {
        static constexpr auto name = "op";
        static constexpr auto op   = op_minus;
        using operand              = dsl::atom;
    };
};

struct postfix : lexy::expression_production, test_production
{
    static constexpr auto max_operator_nesting = 2;
    static constexpr auto atom                 = integer;

    struct operation : dsl::postfix_op
    {
        static constexpr auto name = "op";
        static constexpr auto op   = op_minus;
        using operand              = dsl::atom;
    };
};

struct prefix : lexy::expression_production, test_production
{
    static constexpr auto max_operator_nesting = 2;
    static constexpr auto atom                 = integer;

    struct operation : dsl::prefix_op
    {
        static constexpr auto name = "op";
        static constexpr auto op   = op_minus;
        using operand              = dsl::atom;
    };
};
} // namespace single_operation

TEST_CASE("expression - single operation")
{
    using namespace single_operation;

    SUBCASE("infix_left")
    {
        using prod = infix_left;

        auto callback = lexy::callback<int>([](const char*, int value) { return value; },
                                            [](const char*, int lhs, lexy::op<op_minus>, int rhs) {
                                                return lhs - rhs;
                                            });

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
        // clang-format on

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.value == 1);
        CHECK(atom.trace == test_trace().operation_chain().digits("1"));
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto one = LEXY_OP_VERIFY("2-1");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        // clang-format off
        CHECK(one.trace == test_trace()
                .operation_chain()
                    .digits("2")
                    .literal("-")
                    .digits("1")
                    .operation("op"));
        CHECK(one.tree == test_tree(prod{})
                .production("op")
                    .digits("2")
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto two = LEXY_OP_VERIFY("3-2-1");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 0);
        // clang-format off
        CHECK(two.trace == test_trace()
                .operation_chain()
                    .digits("3")
                    .literal("-")
                    .digits("2")
                    .operation("op")
                    .literal("-")
                    .digits("1")
                    .operation("op"));
        CHECK(two.tree == test_tree(prod{})
                .production("op")
                    .production("op")
                        .digits("3")
                        .literal("-")
                        .digits("2")
                        .finish()
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto three = LEXY_OP_VERIFY("4-3-2-1");
        CHECK(three.status == test_result::recovered_error);
        CHECK(three.value == -1);
        // clang-format off
        CHECK(three.trace == test_trace()
                .operation_chain()
                    .digits("4")
                    .literal("-")
                    .digits("3")
                    .operation("op")
                    .literal("-")
                    .digits("2")
                    .operation("op")
                    .error(5, 6, "maximum operator nesting level exceeded"));
        CHECK(three.tree == test_tree(prod{})
                .production("op")
                    .production("op")
                        .digits("4")
                        .literal("-")
                        .digits("3")
                        .finish()
                    .literal("-")
                    .digits("2"));
        // clang-format on

        auto prefix = LEXY_OP_VERIFY("-1");
        CHECK(prefix.status == test_result::fatal_error);
        CHECK(prefix.value == -1);
        // clang-format off
        CHECK(prefix.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(prefix.tree == test_tree());
        // clang-format on

        auto postfix = LEXY_OP_VERIFY("1-");
        CHECK(postfix.status == test_result::recovered_error);
        CHECK(postfix.value == 1);
        // clang-format off
        CHECK(postfix.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("-")
                    .expected_char_class(2, "digit.decimal"));
        CHECK(postfix.tree == test_tree(prod{})
                .digits("1")
                .literal("-"));
        // clang-format on
    }
    SUBCASE("infix_right")
    {
        using prod = infix_right;

        auto callback = lexy::callback<int>([](const char*, int value) { return value; },
                                            [](const char*, int lhs, lexy::op<op_minus>, int rhs) {
                                                return lhs - rhs;
                                            });

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
        // clang-format on

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.value == 1);
        CHECK(atom.trace == test_trace().operation_chain().digits("1"));
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto one = LEXY_OP_VERIFY("2-1");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        // clang-format off
        CHECK(one.trace == test_trace()
                .operation_chain()
                    .digits("2")
                    .literal("-")
                    .operation_chain()
                        .digits("1")
                        .finish()
                    .operation("op"));
        CHECK(one.tree == test_tree(prod{})
                .production("op")
                    .digits("2")
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto two = LEXY_OP_VERIFY("3-2-1");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        // clang-format off
        CHECK(two.trace == test_trace()
                .operation_chain()
                    .digits("3")
                    .literal("-")
                    .operation_chain()
                        .digits("2")
                        .literal("-")
                        .operation_chain()
                            .digits("1")
                            .finish()
                        .operation("op")
                        .finish()
                    .operation("op"));
        CHECK(two.tree == test_tree(prod{})
                .production("op")
                    .digits("3")
                    .literal("-")
                    .production("op")
                        .digits("2")
                        .literal("-")
                        .digits("1"));
        // clang-format on

        auto three = LEXY_OP_VERIFY("4-3-2-1");
        CHECK(three.status == test_result::recovered_error);
        CHECK(three.value == 4);
        // clang-format off
        CHECK(three.trace == test_trace()
                .operation_chain()
                    .digits("4")
                    .literal("-")
                    .operation_chain()
                        .digits("3")
                        .literal("-")
                        .operation_chain()
                            .digits("2")
                            .error(5, 6, "maximum operator nesting level exceeded"));
        CHECK(three.tree == test_tree(prod{})
                .digits("4")
                .literal("-")
                .digits("3")
                .literal("-")
                .digits("2"));
        // clang-format on

        auto prefix = LEXY_OP_VERIFY("-1");
        CHECK(prefix.status == test_result::fatal_error);
        CHECK(prefix.value == -1);
        // clang-format off
        CHECK(prefix.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(prefix.tree == test_tree());
        // clang-format on

        auto postfix = LEXY_OP_VERIFY("1-");
        CHECK(postfix.status == test_result::recovered_error);
        CHECK(postfix.value == 1);
        // clang-format off
        CHECK(postfix.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("-")
                    .operation_chain()
                        .expected_char_class(2, "digit.decimal")
                        .finish());
        CHECK(postfix.tree == test_tree(prod{})
                .digits("1")
                .literal("-"));
        // clang-format on
    }
    SUBCASE("infix_list")
    {
        using prod = infix_list;

        auto sink = lexy::fold_inplace<int>(0, [first = true](int& result, auto arg) mutable {
            if constexpr (std::is_same_v<decltype(arg), int>)
            {
                if (first)
                    result = arg;
                else
                    result -= arg;

                first = false;
            }
            else
            {
                CHECK(!first);
                CHECK(std::is_same_v<decltype(arg), lexy::op<op_minus>>);
            }
        });

        auto callback = sink >> lexy::callback<int>([](const char*, int value) { return value; });

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
        // clang-format on

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.value == 1);
        CHECK(atom.trace == test_trace().operation_chain().digits("1"));
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto one = LEXY_OP_VERIFY("2-1");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        // clang-format off
        CHECK(one.trace == test_trace()
                .operation_chain()
                    .digits("2")
                    .literal("-")
                    .digits("1")
                    .operation("op"));
        CHECK(one.tree == test_tree(prod{})
                .production("op")
                    .digits("2")
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto two = LEXY_OP_VERIFY("3-2-1");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 0);
        // clang-format off
        CHECK(two.trace == test_trace()
                .operation_chain()
                    .digits("3")
                    .literal("-")
                    .digits("2")
                    .literal("-")
                    .digits("1")
                    .operation("op"));
        CHECK(two.tree == test_tree(prod{})
                .production("op")
                    .digits("3")
                    .literal("-")
                    .digits("2")
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto three = LEXY_OP_VERIFY("4-3-2-1");
        CHECK(three.status == test_result::success);
        CHECK(three.value == -2);
        // clang-format off
        CHECK(three.trace == test_trace()
                .operation_chain()
                    .digits("4")
                    .literal("-")
                    .digits("3")
                    .literal("-")
                    .digits("2")
                    .literal("-")
                    .digits("1")
                    .operation("op"));
        CHECK(three.tree == test_tree(prod{})
                .production("op")
                    .digits("4")
                    .literal("-")
                    .digits("3")
                    .literal("-")
                    .digits("2")
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto prefix = LEXY_OP_VERIFY("-1");
        CHECK(prefix.status == test_result::fatal_error);
        CHECK(prefix.value == -1);
        // clang-format off
        CHECK(prefix.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(prefix.tree == test_tree());
        // clang-format on

        auto postfix = LEXY_OP_VERIFY("1-");
        CHECK(postfix.status == test_result::recovered_error);
        CHECK(postfix.value == 1);
        // clang-format off
        CHECK(postfix.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("-")
                    .expected_char_class(2, "digit.decimal"));
        CHECK(postfix.tree == test_tree(prod{})
                .digits("1")
                .literal("-"));
        // clang-format on
    }
    SUBCASE("infix_single")
    {
        using prod = infix_single;

        auto callback = lexy::callback<int>([](const char*, int value) { return value; },
                                            [](const char*, int lhs, lexy::op<op_minus>, int rhs) {
                                                return lhs - rhs;
                                            });

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
        // clang-format on

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.value == 1);
        CHECK(atom.trace == test_trace().operation_chain().digits("1"));
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto one = LEXY_OP_VERIFY("2-1");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        // clang-format off
        CHECK(one.trace == test_trace()
                .operation_chain()
                    .digits("2")
                    .literal("-")
                    .digits("1")
                    .operation("op"));
        CHECK(one.tree == test_tree(prod{})
                .production("op")
                    .digits("2")
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto two = LEXY_OP_VERIFY("3-2-1");
        CHECK(two.status == test_result::recovered_error);
        CHECK(two.value == 0);
        // clang-format off
        CHECK(two.trace == test_trace()
                .operation_chain()
                    .digits("3")
                    .literal("-")
                    .digits("2")
                    .error(3, 4, "operator cannot be chained")
                    .operation("op")
                    .literal("-")
                    .digits("1")
                    .operation("op"));
        CHECK(two.tree == test_tree(prod{})
                .production("op")
                    .production("op")
                        .digits("3")
                        .literal("-")
                        .digits("2")
                        .finish()
                    .literal("-")
                    .digits("1"));
        // clang-format on

        auto three = LEXY_OP_VERIFY("4-3-2-1");
        CHECK(three.status == test_result::recovered_error);
        CHECK(three.value == -1);
        // clang-format off
        CHECK(three.trace == test_trace()
                .operation_chain()
                    .digits("4")
                    .literal("-")
                    .digits("3")
                    .error(3, 4, "operator cannot be chained")
                    .operation("op")
                    .literal("-")
                    .digits("2")
                    .error(5, 6, "operator cannot be chained")
                    .operation("op")
                    .error(5, 6, "maximum operator nesting level exceeded"));
        CHECK(three.tree == test_tree(prod{})
                .production("op")
                    .production("op")
                        .digits("4")
                        .literal("-")
                        .digits("3")
                        .finish()
                    .literal("-")
                    .digits("2"));
        // clang-format on

        auto prefix = LEXY_OP_VERIFY("-1");
        CHECK(prefix.status == test_result::fatal_error);
        CHECK(prefix.value == -1);
        // clang-format off
        CHECK(prefix.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(prefix.tree == test_tree());
        // clang-format on

        auto postfix = LEXY_OP_VERIFY("1-");
        CHECK(postfix.status == test_result::recovered_error);
        CHECK(postfix.value == 1);
        // clang-format off
        CHECK(postfix.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("-")
                    .expected_char_class(2, "digit.decimal"));
        CHECK(postfix.tree == test_tree(prod{})
                .digits("1")
                .literal("-"));
        // clang-format on
    }

    SUBCASE("postfix")
    {
        using prod = postfix;

        auto callback
            = lexy::callback<int>([](const char*, int value) { return value; },
                                  [](const char*, int lhs, lexy::op<op_minus>) { return -lhs; });

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
        // clang-format on

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.value == 1);
        CHECK(atom.trace == test_trace().operation_chain().digits("1"));
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto one = LEXY_OP_VERIFY("1-");
        CHECK(one.status == test_result::success);
        CHECK(one.value == -1);
        CHECK(one.trace == test_trace().operation_chain().digits("1").literal("-").operation("op"));
        CHECK(one.tree == test_tree(prod{}).production("op").digits("1").literal("-"));

        auto two = LEXY_OP_VERIFY("1--");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 1);
        // clang-format off
        CHECK(two.trace == test_trace()
                 .operation_chain()
                     .digits("1")
                     .literal("-")
                     .operation("op")
                     .literal("-")
                     .operation("op"));
        CHECK(two.tree == test_tree(prod{})
                 .production("op")
                     .production("op")
                         .digits("1")
                         .literal("-")
                         .finish()
                     .literal("-"));
        // clang-format on

        auto three = LEXY_OP_VERIFY("1---");
        CHECK(three.status == test_result::recovered_error);
        CHECK(three.value == 1);
        // clang-format off
        CHECK(three.trace == test_trace()
                 .operation_chain()
                     .digits("1")
                     .literal("-")
                     .operation("op")
                     .literal("-")
                     .operation("op")
                     .error(3, 4, "maximum operator nesting level exceeded"));
        CHECK(three.tree == test_tree(prod{})
                 .production("op")
                     .production("op")
                         .digits("1")
                         .literal("-")
                         .finish()
                     .literal("-"));
        // clang-format on

        auto prefix = LEXY_OP_VERIFY("-1");
        CHECK(prefix.status == test_result::fatal_error);
        CHECK(prefix.value == -1);
        // clang-format off
        CHECK(prefix.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(prefix.tree == test_tree());
        // clang-format on

        auto infix = LEXY_OP_VERIFY("1-2");
        CHECK(infix.status == test_result::success);
        CHECK(infix.value == -1);
        CHECK(infix.trace
              == test_trace().operation_chain().digits("1").literal("-").operation("op"));
        CHECK(infix.tree == test_tree(prod{}).production("op").digits("1").literal("-"));
    }

    SUBCASE("prefix")
    {
        using prod = prefix;

        auto callback
            = lexy::callback<int>([](const char*, int value) { return value; },
                                  [](const char*, lexy::op<op_minus>, int rhs) { return -rhs; });

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());
        CHECK(empty.tree == test_tree());

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.value == 1);
        CHECK(atom.trace == test_trace().digits("1"));
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto one = LEXY_OP_VERIFY("-1");
        CHECK(one.status == test_result::success);
        CHECK(one.value == -1);
        CHECK(one.trace == test_trace().operation_chain().literal("-").digits("1").operation("op"));
        CHECK(one.tree == test_tree(prod{}).production("op").literal("-").digits("1"));

        auto two = LEXY_OP_VERIFY("--1");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 1);
        // clang-format off
        CHECK(two.trace == test_trace()
                 .operation_chain()
                     .literal("-")
                     .operation_chain()
                         .literal("-")
                         .digits("1")
                         .operation("op")
                         .finish()
                     .operation("op"));
        CHECK(two.tree == test_tree(prod{})
                 .production("op")
                    .literal("-")
                     .production("op")
                         .literal("-")
                         .digits("1")
                         .finish());
        // clang-format on

        auto three = LEXY_OP_VERIFY("---1");
        CHECK(three.status == test_result::fatal_error);
        // clang-format off
        CHECK(three.trace == test_trace()
                 .operation_chain()
                     .literal("-")
                     .operation_chain()
                         .literal("-")
                         .operation_chain()
                            .error(2, 3, "maximum operator nesting level exceeded")
                            .finish()
                         .finish()
                    .finish()
                .cancel());
        CHECK(three.tree == test_tree());
        // clang-format on

        auto infix = LEXY_OP_VERIFY("1-2");
        CHECK(infix.status == test_result::success);
        CHECK(infix.value == 1);
        CHECK(infix.trace == test_trace().digits("1"));
        CHECK(infix.tree == test_tree(prod{}).digits("1"));

        auto postfix = LEXY_OP_VERIFY("1-");
        CHECK(postfix.status == test_result::success);
        CHECK(postfix.value == 1);
        CHECK(postfix.trace == test_trace().digits("1"));
        CHECK(postfix.tree == test_tree(prod{}).digits("1"));
    }
}

// We're only testing the trace in the error case from now on,
// and using the parse tree otherwise as it has less clutter.

namespace nested_operations
{
constexpr auto op_plus  = dsl::op(dsl::lit_c<'+'>);
constexpr auto op_minus = dsl::op(dsl::lit_c<'-'>);
constexpr auto op_times = dsl::op(dsl::lit_c<'*'>);
constexpr auto op_div   = dsl::op(dsl::lit_c<'/'>);

struct sum_product : lexy::expression_production, test_production
{
    static constexpr auto atom = integer;

    struct prefix : dsl::prefix_op
    {
        static constexpr auto name = "prefix";
        static constexpr auto op   = op_plus / op_minus;
        using operand              = dsl::atom;
    };
    struct product : dsl::infix_op_left
    {
        static constexpr auto name = "product";
        static constexpr auto op   = op_times / op_div;
        using operand              = prefix;
    };
    struct sum : dsl::infix_op_left
    {
        static constexpr auto name = "sum";
        static constexpr auto op   = op_plus / op_minus;
        using operand              = product;
    };
    using operation = sum;
};
} // namespace nested_operations

TEST_CASE("expression - nested operations")
{
    using namespace nested_operations;
    auto callback
        = lexy::callback<int>([](const char*, int value) { return value; },
                              [](const char*, lexy::op<op_plus>, int rhs) { return rhs; },
                              [](const char*, lexy::op<op_minus>, int rhs) { return -rhs; },
                              [](const char*, int lhs, lexy::op<op_plus>, int rhs) {
                                  return lhs + rhs;
                              },
                              [](const char*, int lhs, lexy::op<op_minus>, int rhs) {
                                  return lhs - rhs;
                              },
                              [](const char*, int lhs, lexy::op<op_times>, int rhs) {
                                  return lhs * rhs;
                              },
                              [](const char*, int lhs, lexy::op<op_div>, int rhs) {
                                  return lhs / rhs;
                              });

    using prod = sum_product;

    auto empty = LEXY_OP_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.value == -1);
    // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                   .expected_char_class(0, "digit.decimal")
                   .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
    // clang-format on

    auto atom = LEXY_OP_VERIFY("1");
    CHECK(atom.status == test_result::success);
    CHECK(atom.value == 1);
    CHECK(atom.tree == test_tree(prod{}).digits("1"));

    auto a = LEXY_OP_VERIFY("1+2");
    CHECK(a.status == test_result::success);
    CHECK(a.value == 3);
    CHECK(a.tree == test_tree(prod{}).production("sum").digits("1").literal("+").digits("2"));
    auto m = LEXY_OP_VERIFY("2-1");
    CHECK(m.status == test_result::success);
    CHECK(m.value == 1);
    CHECK(m.tree == test_tree(prod{}).production("sum").digits("2").literal("-").digits("1"));
    auto t = LEXY_OP_VERIFY("1*2");
    CHECK(t.status == test_result::success);
    CHECK(t.value == 2);
    CHECK(t.tree == test_tree(prod{}).production("product").digits("1").literal("*").digits("2"));
    auto d = LEXY_OP_VERIFY("4/2");
    CHECK(d.status == test_result::success);
    CHECK(d.value == 2);
    CHECK(d.tree == test_tree(prod{}).production("product").digits("4").literal("/").digits("2"));

    auto p = LEXY_OP_VERIFY("+1");
    CHECK(p.status == test_result::success);
    CHECK(p.value == 1);
    CHECK(p.tree == test_tree(prod{}).production("prefix").literal("+").digits("1"));
    auto n = LEXY_OP_VERIFY("-1");
    CHECK(n.status == test_result::success);
    CHECK(n.value == -1);
    CHECK(n.tree == test_tree(prod{}).production("prefix").literal("-").digits("1"));

    auto at = LEXY_OP_VERIFY("1+2*3");
    CHECK(at.status == test_result::success);
    CHECK(at.value == 7);
    // clang-format off
    CHECK(at.tree == test_tree(prod{})
            .production("sum")
                .digits("1")
                .literal("+")
                .production("product")
                    .digits("2")
                    .literal("*")
                    .digits("3"));
    // clang-format on
    auto ta = LEXY_OP_VERIFY("1*2+3");
    CHECK(ta.status == test_result::success);
    CHECK(ta.value == 5);
    // clang-format off
    CHECK(ta.tree == test_tree(prod{})
            .production("sum")
                .production("product")
                    .digits("1")
                    .literal("*")
                    .digits("2")
                    .finish()
                .literal("+")
                .digits("3"));
    // clang-format on

    auto ata = LEXY_OP_VERIFY("1+2*3+4");
    CHECK(ata.status == test_result::success);
    CHECK(ata.value == 11);
    // clang-format off
    CHECK(ata.tree == test_tree(prod{})
            .production("sum")
                .production("sum")
                    .digits("1")
                    .literal("+")
                    .production("product")
                        .digits("2")
                        .literal("*")
                        .digits("3")
                        .finish()
                    .finish()
                .literal("+")
                .digits("4"));
    // clang-format on
    auto tat = LEXY_OP_VERIFY("1*2+3*4");
    CHECK(tat.status == test_result::success);
    CHECK(tat.value == 14);
    // clang-format off
    CHECK(tat.tree == test_tree(prod{})
            .production("sum")
                .production("product")
                    .digits("1")
                    .literal("*")
                    .digits("2")
                    .finish()
                .literal("+")
                .production("product")
                    .digits("3")
                    .literal("*")
                    .digits("4"));
    // clang-format on

    auto tn = LEXY_OP_VERIFY("2*-1");
    CHECK(tn.status == test_result::success);
    CHECK(tn.value == -2);
    // clang-format off
    CHECK(tn.tree == test_tree(prod{})
            .production("product")
                .digits("2")
                .literal("*")
                .production("prefix")
                    .literal("-")
                    .digits("1"));
    // clang-format on
    auto mn = LEXY_OP_VERIFY("2--1");
    CHECK(mn.status == test_result::success);
    CHECK(mn.value == 3);
    // clang-format off
    CHECK(mn.tree == test_tree(prod{})
            .production("sum")
                .digits("2")
                .literal("-")
                .production("prefix")
                    .literal("-")
                    .digits("1"));
    // clang-format on
}

namespace groups
{
constexpr auto op_a = dsl::op(LEXY_LIT("a"));
constexpr auto op_b = dsl::op(LEXY_LIT("b"));
constexpr auto op_c = dsl::op(LEXY_LIT("c"));
constexpr auto op_d = dsl::op(LEXY_LIT("d"));

struct top_level : lexy::expression_production, test_production
{
    static constexpr auto atom = integer;

    struct op1 : dsl::infix_op_right
    {
        static constexpr auto name = "op1";
        static constexpr auto op   = op_a;
        using operand              = dsl::atom;
    };
    struct op2 : dsl::prefix_op
    {
        static constexpr auto name = "op2";
        static constexpr auto op   = op_b;
        using operand              = dsl::atom;
    };
    struct op3 : dsl::infix_op_left
    {
        static constexpr auto name = "op3";
        static constexpr auto op   = op_b;
        using operand              = dsl::atom;
    };

    using operation = dsl::groups<op1, op2, op3>;
};

struct nested_groups : lexy::expression_production, test_production
{
    static constexpr auto atom = integer;

    struct op11 : dsl::infix_op_right
    {
        static constexpr auto name = "op11";
        static constexpr auto op   = op_a;
        using operand              = dsl::atom;
    };
    struct op12 : dsl::infix_op_right
    {
        static constexpr auto name = "op12";
        static constexpr auto op   = op_b;
        using operand              = dsl::atom;
    };

    struct op1 : dsl::infix_op_right
    {
        static constexpr auto name = "op1";
        static constexpr auto op   = op_c;
        using operand              = dsl::groups<op11, op12>;
    };

    struct op0 : dsl::infix_op_right
    {
        static constexpr auto name = "op0";
        static constexpr auto op   = op_d;
        using operand              = dsl::groups<op1>;
    };
    using operation = op0;
};
} // namespace groups

TEST_CASE("expression - groups")
{
    using namespace groups;
    auto callback = lexy::callback<int>([](auto...) { return 0; });

    SUBCASE("top_level")
    {
        using prod = top_level;

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
        // clang-format on

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto op1_one = LEXY_OP_VERIFY("1a2");
        CHECK(op1_one.status == test_result::success);
        CHECK(op1_one.tree
              == test_tree(prod{}).production("op1").digits("1").literal("a").digits("2"));

        auto op1_two = LEXY_OP_VERIFY("1a2a3");
        CHECK(op1_two.status == test_result::success);
        // clang-format off
        CHECK(op1_two.tree == test_tree(prod{})
                .production("op1")
                    .digits("1")
                    .literal("a")
                    .production("op1")
                        .digits("2")
                        .literal("a")
                        .digits("3"));
        // clang-format on

        auto op2_one = LEXY_OP_VERIFY("b1");
        CHECK(op2_one.status == test_result::success);
        CHECK(op2_one.tree == test_tree(prod{}).production("op2").literal("b").digits("1"));

        auto op2_two = LEXY_OP_VERIFY("bb1");
        CHECK(op2_two.status == test_result::success);
        // clang-format off
        CHECK(op2_two.tree == test_tree(prod{})
                .production("op2")
                    .literal("b")
                    .production("op2")
                        .literal("b")
                        .digits("1"));
        // clang-format on

        auto op3_one = LEXY_OP_VERIFY("1b2");
        CHECK(op3_one.status == test_result::success);
        CHECK(op3_one.tree
              == test_tree(prod{}).production("op3").digits("1").literal("b").digits("2"));

        auto op3_two = LEXY_OP_VERIFY("1b2b3");
        CHECK(op3_two.status == test_result::success);
        // clang-format off
        CHECK(op3_two.tree == test_tree(prod{})
                .production("op3")
                    .production("op3")
                        .digits("1")
                        .literal("b")
                        .digits("2")
                        .finish()
                    .literal("b")
                    .digits("3"));
        // clang-format on

        auto op1_op2 = LEXY_OP_VERIFY("1ab2");
        CHECK(op1_op2.status == test_result::recovered_error);
        // clang-format off
        CHECK(op1_op2.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("a")
                    .operation_chain()
                        .operation_chain()
                            .error(2, 3, "operator cannot be mixed with previous operators")
                            .literal("b")
                            .operation_chain()
                                .digits("2")
                                .finish()
                            .operation("op2")
                            .finish()
                        .finish()
                    .operation("op1"));
        // clang-format on

        auto op3_op1 = LEXY_OP_VERIFY("1b2a3");
        CHECK(op3_op1.status == test_result::recovered_error);
        // clang-format off
        CHECK(op3_op1.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("b")
                    .operation_chain()
                        .digits("2")
                        .error(3, 4, "operator cannot be mixed with previous operators")
                        .literal("a")
                        .operation_chain()
                            .digits("3")
                            .finish()
                        .operation("op1")
                        .finish()
                    .operation("op3"));
        // clang-format on

        auto op2_op3 = LEXY_OP_VERIFY("b1b2");
        CHECK(op2_op3.status == test_result::recovered_error);
        // clang-format off
        CHECK(op2_op3.trace == test_trace()
                .operation_chain()
                    .operation_chain()
                        .literal("b")
                        .operation_chain()
                            .digits("1")
                            .finish()
                        .operation("op2")
                        .finish()
                    .error(2, 3, "operator cannot be mixed with previous operators")
                    .literal("b")
                    .operation_chain()
                        .digits("2")
                        .finish()
                    .operation("op3"));
        // clang-format on
    }
    SUBCASE("nested_groups")
    {
        using prod = nested_groups;

        auto empty = LEXY_OP_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        // clang-format off
        CHECK(empty.trace == test_trace()
                .operation_chain()
                    .expected_char_class(0, "digit.decimal")
                    .finish()
                .cancel());
        CHECK(empty.tree == test_tree());
        // clang-format on

        auto atom = LEXY_OP_VERIFY("1");
        CHECK(atom.status == test_result::success);
        CHECK(atom.tree == test_tree(prod{}).digits("1"));

        auto op0_one = LEXY_OP_VERIFY("1d2");
        CHECK(op0_one.status == test_result::success);
        CHECK(op0_one.tree
              == test_tree(prod{}).production("op0").digits("1").literal("d").digits("2"));

        auto op0_two = LEXY_OP_VERIFY("1d2d3");
        CHECK(op0_two.status == test_result::success);
        // clang-format off
        CHECK(op0_two.tree == test_tree(prod{})
                .production("op0")
                    .digits("1")
                    .literal("d")
                    .production("op0")
                        .digits("2")
                        .literal("d")
                        .digits("3"));
        // clang-format on

        auto op1_one = LEXY_OP_VERIFY("1c2");
        CHECK(op1_one.status == test_result::success);
        CHECK(op1_one.tree
              == test_tree(prod{}).production("op1").digits("1").literal("c").digits("2"));
        auto op1_two = LEXY_OP_VERIFY("1c2c3");
        CHECK(op1_two.status == test_result::success);
        // clang-format off
        CHECK(op1_two.tree == test_tree(prod{})
                .production("op1")
                    .digits("1")
                    .literal("c")
                    .production("op1")
                        .digits("2")
                        .literal("c")
                        .digits("3"));
        // clang-format on

        auto op11_one = LEXY_OP_VERIFY("1a2");
        CHECK(op11_one.status == test_result::success);
        CHECK(op11_one.tree
              == test_tree(prod{}).production("op11").digits("1").literal("a").digits("2"));
        auto op11_two = LEXY_OP_VERIFY("1a2a3");
        CHECK(op11_two.status == test_result::success);
        // clang-format off
        CHECK(op11_two.tree == test_tree(prod{})
                .production("op11")
                    .digits("1")
                    .literal("a")
                    .production("op11")
                        .digits("2")
                        .literal("a")
                        .digits("3"));
        // clang-format on

        auto op12_one = LEXY_OP_VERIFY("1b2");
        CHECK(op12_one.status == test_result::success);
        CHECK(op12_one.tree
              == test_tree(prod{}).production("op12").digits("1").literal("b").digits("2"));
        auto op12_two = LEXY_OP_VERIFY("1b2b3");
        CHECK(op12_two.status == test_result::success);
        // clang-format off
        CHECK(op12_two.tree == test_tree(prod{})
                .production("op12")
                    .digits("1")
                    .literal("b")
                    .production("op12")
                        .digits("2")
                        .literal("b")
                        .digits("3"));
        // clang-format on

        auto op0_op1 = LEXY_OP_VERIFY("1d2c3");
        CHECK(op0_op1.status == test_result::success);
        // clang-format off
        CHECK(op0_op1.tree == test_tree(prod{})
                .production("op0")
                    .digits("1")
                    .literal("d")
                    .production("op1")
                        .digits("2")
                        .literal("c")
                        .digits("3"));
        // clang-format on
        auto op1_op0 = LEXY_OP_VERIFY("1c2d3");
        CHECK(op1_op0.status == test_result::success);
        // clang-format off
        CHECK(op1_op0.tree == test_tree(prod{})
                .production("op0")
                    .production("op1")
                        .digits("1")
                        .literal("c")
                        .digits("2")
                        .finish()
                    .literal("d")
                    .digits("3"));
        // clang-format on
        auto op0_op11 = LEXY_OP_VERIFY("1d2a3");
        CHECK(op0_op11.status == test_result::success);
        // clang-format off
        CHECK(op0_op11.tree == test_tree(prod{})
                .production("op0")
                    .digits("1")
                    .literal("d")
                    .production("op11")
                        .digits("2")
                        .literal("a")
                        .digits("3"));
        // clang-format on
        auto op0_op12 = LEXY_OP_VERIFY("1d2b3");
        CHECK(op0_op12.status == test_result::success);
        // clang-format off
        CHECK(op0_op12.tree == test_tree(prod{})
                .production("op0")
                    .digits("1")
                    .literal("d")
                    .production("op12")
                        .digits("2")
                        .literal("b")
                        .digits("3"));
        // clang-format on

        auto op1_op11 = LEXY_OP_VERIFY("1c2a3");
        CHECK(op1_op11.status == test_result::recovered_error);
        // clang-format off
        CHECK(op1_op11.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("c")
                    .operation_chain()
                        .digits("2")
                        .error(3, 4, "operator cannot be mixed with previous operators")
                        .literal("a")
                        .operation_chain()
                            .digits("3")
                            .finish()
                        .operation("op11")
                        .finish()
                    .operation("op1"));
        // clang-format on
        auto op12_op1 = LEXY_OP_VERIFY("1b2c3");
        CHECK(op12_op1.status == test_result::recovered_error);
        // clang-format off
        CHECK(op12_op1.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("b")
                    .operation_chain()
                        .digits("2")
                        .finish()
                    .operation("op12")
                    .error(3, 4, "operator cannot be mixed with previous operators")
                    .literal("c")
                    .operation_chain()
                        .digits("3")
                        .finish()
                    .operation("op1"));
        // clang-format on
        auto op12_op11 = LEXY_OP_VERIFY("1b2a3");
        CHECK(op12_op11.status == test_result::recovered_error);
        // clang-format off
        CHECK(op12_op11.trace == test_trace()
                .operation_chain()
                    .digits("1")
                    .literal("b")
                    .operation_chain()
                        .digits("2")
                        .error(3, 4, "operator cannot be mixed with previous operators")
                        .literal("a")
                        .operation_chain()
                            .digits("3")
                            .finish()
                        .operation("op11")
                        .finish()
                    .operation("op12"));
        // clang-format on
    }
}

