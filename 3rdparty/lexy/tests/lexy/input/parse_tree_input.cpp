// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/parse_tree_input.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/parse_tree_node.hpp>
#include <lexy/dsl/until.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/parse_tree.hpp>

#include "../dsl/verify.hpp"

namespace
{
enum class token_kind
{
    a,
    b,
    c,
};

struct child_p
{
    static constexpr auto rule = lexy::dsl::any;
};

struct root_p
{
    static constexpr auto rule = lexy::dsl::any;
};

using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
} // namespace

TEST_CASE("parse_tree_input")
{
    auto input = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        return LEXY_MOV(builder).finish(input.data() + 11);
    }();
    CHECK(!tree.empty());

    auto tree_input = lexy::parse_tree_input(tree);
    CHECK(tree_input.root().address() == tree.root().address());

    auto root_reader = tree_input.reader();
    CHECK(root_reader.position() == input.data());
    CHECK(root_reader.peek().kind() == token_kind::a);

    {
        auto child_reader = root_reader.child_reader();
        CHECK(child_reader.peek().address() == nullptr);
    }

    root_reader.bump();
    CHECK(root_reader.position() == input.data() + 3);
    CHECK(root_reader.peek().kind() == child_p{});

    {
        auto child_reader = root_reader.child_reader();
        CHECK(child_reader.position() == input.data() + 3);
        CHECK(child_reader.peek().kind() == token_kind::b);

        child_reader.bump();
        CHECK(child_reader.position() == input.data() + 4);
        CHECK(child_reader.peek().kind() == token_kind::c);

        child_reader.bump();
        CHECK(child_reader.position() == input.data() + 7);
        CHECK(child_reader.peek().kind() == token_kind::b);

        child_reader.bump();
        CHECK(child_reader.position() == input.data() + 8);
        CHECK(child_reader.peek().address() == nullptr);
    }

    root_reader.bump();
    CHECK(root_reader.position() == input.data() + 8);
    CHECK(root_reader.peek().kind() == token_kind::a);

    {
        auto child_reader = root_reader.child_reader();
        CHECK(child_reader.peek().address() == nullptr);
    }

    root_reader.bump();
    CHECK(root_reader.position() == input.data() + 11);
    CHECK(root_reader.peek().address() == nullptr);
}

TEST_CASE("dsl::eof on parse_tree_input")
{
    constexpr auto rule     = dsl::any + dsl::eof;
    constexpr auto callback = lexy_test::token_callback;

    auto result = LEXY_VERIFY_RUNTIME([&] {
        auto                str = "abc";
        parse_tree::builder b(root_p{});
        b.token(token_kind::a, str, str + 1);
        b.token(token_kind::b, str, str + 2);
        b.token(token_kind::c, str, str + 2);
        return LEXY_MOV(b).finish(str + 3);
    }());
    CHECK(result.status == test_result::success);
    CHECK(result.trace == test_trace().token("any", "abc").token("EOF", ""));
}

TEST_CASE("dsl::any on parse_tree_input")
{
    constexpr auto rule     = dsl::any;
    constexpr auto callback = lexy_test::token_callback;

    auto result = LEXY_VERIFY_RUNTIME([&] {
        auto                str = "abc";
        parse_tree::builder b(root_p{});
        b.token(token_kind::a, str, str + 1);
        b.token(token_kind::b, str + 1, str + 2);
        b.token(token_kind::c, str + 2, str + 3);
        return LEXY_MOV(b).finish(str + 3);
    }());
    CHECK(result.status == test_result::success);
    CHECK(result.trace == test_trace().token("any", "abc"));
}

TEST_CASE("dsl::until on parse_tree_input")
{
    constexpr auto rule     = dsl::until(dsl::tnode<token_kind::b>);
    constexpr auto callback = lexy_test::token_callback;

    auto result = LEXY_VERIFY_RUNTIME([&] {
        auto                str = "abc";
        parse_tree::builder b(root_p{});
        b.token(token_kind::a, str, str + 1);
        b.token(token_kind::b, str + 1, str + 2);
        b.token(token_kind::c, str + 2, str + 3);
        return LEXY_MOV(b).finish(str + 3);
    }());
    CHECK(result.status == test_result::success);
    CHECK(result.trace == test_trace().token("any", "ab"));
}

TEST_CASE("dsl::capture on parse_tree_input")
{
    constexpr auto rule     = dsl::capture(dsl::tnode<token_kind::a>);
    constexpr auto callback = [](const char* position, lexy::string_lexeme<> lexeme) {
        CHECK(position == lexeme.begin());
        return int(lexeme.size());
    };

    auto result = LEXY_VERIFY_RUNTIME([&] {
        auto                str = "abc";
        parse_tree::builder b(root_p{});
        b.token(token_kind::a, str, str + 3);
        return LEXY_MOV(b).finish(str + 3);
    }());
    CHECK(result.status == test_result::success);
    CHECK(result.value == 3);
    CHECK(result.trace == test_trace().token("token", "abc"));
}

