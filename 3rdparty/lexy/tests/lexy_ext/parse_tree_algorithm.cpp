// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy_ext/parse_tree_algorithm.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

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
    static constexpr auto rule = 0; // Need a rule to identify as production.
};

struct root_p
{};
} // namespace

TEST_CASE("tokens()")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        child = builder.start_production(child_p{});
        builder.finish_production(LEXY_MOV(child));

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    SUBCASE("entire tree")
    {
        auto tokens = lexy_ext::tokens(tree);
        CHECK(!tokens.empty());

        doctest::String result;
        for (auto token : tokens)
            result += doctest::String(token.lexeme().data(), unsigned(token.lexeme().size()));
        CHECK(result == "123(abc)321");
    }
    SUBCASE("token")
    {
        auto tokens = lexy_ext::tokens(tree, *tree.root().children().begin());
        CHECK(!tokens.empty());

        auto iter = tokens.begin();
        CHECK(iter != tokens.end());
        CHECK(*iter == *tree.root().children().begin());

        ++iter;
        CHECK(iter == tokens.end());
    }
    SUBCASE("non-empty production")
    {
        auto tokens = lexy_ext::tokens(tree, *++tree.root().children().begin());
        CHECK(!tokens.empty());

        doctest::String result;
        for (auto token : tokens)
            result += doctest::String(token.lexeme().data(), unsigned(token.lexeme().size()));
        CHECK(result == "(abc)");
    }
    SUBCASE("empty production")
    {
        auto tokens = lexy_ext::tokens(tree, [&] {
            auto iter = tree.root().children().begin();
            ++iter;
            ++iter;
            ++iter;
            return *iter;
        }());
        CHECK(tokens.empty());
    }
}

TEST_CASE("find_covering_node()")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        child = builder.start_production(child_p{});
        builder.finish_production(LEXY_MOV(child));

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    auto a = lexy_ext::find_covering_node(tree, input.data() + 1);
    CHECK(a.lexeme().begin() == input.data());

    auto b = lexy_ext::find_covering_node(tree, input.data() + 3);
    CHECK(b.lexeme().begin() == input.data() + 3);

    auto c = lexy_ext::find_covering_node(tree, input.data() + 6);
    CHECK(c.lexeme().begin() == input.data() + 4);
}

TEST_CASE("children()")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        child = builder.start_production(child_p{});
        builder.finish_production(LEXY_MOV(child));

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    auto token_as = lexy_ext::children(tree, tree.root(), token_kind::a);
    CHECK(!token_as.empty());
    {
        auto iter = token_as.begin();
        CHECK(iter != token_as.end());
        CHECK(iter->kind() == token_kind::a);
        CHECK(iter->lexeme().begin() == input.data());

        ++iter;
        CHECK(iter != token_as.end());
        CHECK(iter->kind() == token_kind::a);
        CHECK(iter->lexeme().begin() == input.data() + 8);

        ++iter;
        CHECK(iter == token_as.end());
    }

    auto child_ps = lexy_ext::children(tree, tree.root(), child_p{});
    CHECK(!child_ps.empty());
    {
        auto iter = child_ps.begin();
        CHECK(iter != child_ps.end());
        CHECK(iter->kind() == child_p{});

        ++iter;
        CHECK(iter != child_ps.end());
        CHECK(iter->kind() == child_p{});

        ++iter;
        CHECK(iter == child_ps.end());
    }

    auto token_cs = lexy_ext::children(tree, *child_ps.begin(), token_kind::c);
    CHECK(!token_cs.empty());
    {
        auto iter = token_cs.begin();
        CHECK(iter != token_cs.end());
        CHECK(iter->kind() == token_kind::c);

        ++iter;
        CHECK(iter == token_cs.end());
    }

    auto empty = lexy_ext::children(tree, *child_ps.begin(), token_kind::a);
    CHECK(empty.empty());
}

TEST_CASE("child()")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        child = builder.start_production(child_p{});
        builder.finish_production(LEXY_MOV(child));

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    auto token_a = lexy_ext::child(tree, tree.root(), token_kind::a);
    CHECK(token_a);
    CHECK(token_a->kind() == token_kind::a);
    CHECK(token_a->lexeme().begin() == input.data());

    auto child_p = lexy_ext::child(tree, tree.root(), ::child_p{});
    CHECK(child_p);
    CHECK(child_p->kind() == ::child_p{});

    auto token_c = lexy_ext::child(tree, *child_p, token_kind::c);
    CHECK(token_c);
    CHECK(token_c->kind() == token_kind::c);

    auto empty = lexy_ext::child(tree, *child_p, token_kind::a);
    CHECK(!empty);
}

TEST_CASE("node_position()")
{
    using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
    auto input       = lexy::zstring_input("123(abc)321");

    auto tree = [&] {
        parse_tree::builder builder(root_p{});
        builder.token(token_kind::a, input.data(), input.data() + 3);

        auto child = builder.start_production(child_p{});

        auto child2 = builder.start_production(child_p{});
        builder.token(lexy::position_token_kind, input.data() + 2, input.data() + 2);
        builder.finish_production(LEXY_MOV(child2));

        child2 = builder.start_production(child_p{});
        builder.token(token_kind::b, input.data() + 3, input.data() + 4);
        builder.token(token_kind::c, input.data() + 4, input.data() + 7);
        builder.token(token_kind::b, input.data() + 7, input.data() + 8);
        builder.finish_production(LEXY_MOV(child2));

        builder.finish_production(LEXY_MOV(child));

        builder.token(token_kind::a, input.data() + 8, input.data() + 11);

        child  = builder.start_production(child_p{});
        child2 = builder.start_production(child_p{});
        builder.finish_production(LEXY_MOV(child2));
        builder.finish_production(LEXY_MOV(child));

        return LEXY_MOV(builder).finish();
    }();
    CHECK(!tree.empty());

    auto prod_count = 0;
    for (auto [event, node] : tree.traverse())
        if (event == lexy::traverse_event::enter)
        {
            if (prod_count == 0)
                CHECK(lexy_ext::node_position(tree, node) == input.data());
            else if (prod_count == 1)
                CHECK(lexy_ext::node_position(tree, node) == input.data() + 2);
            else if (prod_count == 2)
                CHECK(lexy_ext::node_position(tree, node) == input.data() + 2);
            else if (prod_count == 3)
                CHECK(lexy_ext::node_position(tree, node) == input.data() + 3);
            else if (prod_count <= 5)
                CHECK(lexy_ext::node_position(tree, node)
                      == lexy::input_reader<lexy::string_input<>>::iterator());

            ++prod_count;
        }
        else if (event == lexy::traverse_event::leaf)
            CHECK(lexy_ext::node_position(tree, node) == node.lexeme().begin());
    REQUIRE(prod_count == 6);
}

