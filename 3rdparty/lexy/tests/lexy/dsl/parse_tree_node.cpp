// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/parse_tree_node.hpp>

#include "verify.hpp"
#include <doctest/doctest.h>
#include <lexy/dsl/any.hpp>
#include <lexy/dsl/if.hpp>

namespace
{
enum class token_kind
{
    a,
    b,
    c,
    child_p,
};

constexpr const char* token_kind_name(token_kind k)
{
    switch (k)
    {
    case token_kind::a:
        return "a";
    case token_kind::b:
        return "b";
    case token_kind::c:
        return "c";
    case token_kind::child_p:
        return "child_p";
    }

    return "";
}

struct child_p
{
    static constexpr auto name = "child_p";
    static constexpr auto rule = lexy::dsl::any;
};

struct root_p
{
    static constexpr auto name = "root_p";
    static constexpr auto rule = lexy::dsl::any;
};

using parse_tree = lexy::parse_tree_for<lexy::string_input<>, token_kind>;
} // namespace

TEST_CASE("dsl::tnode")
{
    constexpr auto rule = dsl::tnode<token_kind::a>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY_RUNTIME([] {
        parse_tree::builder b(root_p{});
        b.token(lexy::eof_token_kind, nullptr, nullptr);
        return LEXY_MOV(b).finish(nullptr);
    }());
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "a").cancel());

    auto a = LEXY_VERIFY_RUNTIME([] {
        auto                str = "abc";
        parse_tree::builder b(root_p{});
        b.token(token_kind::a, str, str + 3);
        return LEXY_MOV(b).finish(str + 3);
    }());
    CHECK(a.status == test_result::success);
    CHECK(a.trace == test_trace().token("a", "abc"));

    auto abc = LEXY_VERIFY_RUNTIME([] {
        auto                str = "abc";
        parse_tree::builder b(root_p{});
        b.token(token_kind::a, str, str + 1);
        b.token(token_kind::b, str + 1, str + 2);
        b.token(token_kind::c, str + 2, str + 3);
        return LEXY_MOV(b).finish(str + 3);
    }());
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().token("a", "a"));
}

TEST_CASE("dsl::tnode(rule)")
{
    constexpr auto tnode = dsl::tnode<token_kind::a>(LEXY_LIT("abc"));
    CHECK(lexy::is_branch_rule<decltype(tnode)>);

    constexpr auto callback = lexy_test::token_callback;

    SUBCASE("basic")
    {
        constexpr auto rule = tnode;

        auto empty = LEXY_VERIFY_RUNTIME([] {
            parse_tree::builder b(root_p{});
            b.token(lexy::eof_token_kind, nullptr, nullptr);
            return LEXY_MOV(b).finish(nullptr);
        }());
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "a").cancel());

        auto b = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});
            b.token(token_kind::b, str, str + 3);
            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(b.status == test_result::fatal_error);
        CHECK(b.trace == test_trace().expected_char_class(0, "a").cancel());

        auto a_abc = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});
            b.token(token_kind::a, str, str + 3);
            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(a_abc.status == test_result::success);
        CHECK(a_abc.trace == test_trace().token("literal", "abc"));

        auto a_abcd = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abcd";
            parse_tree::builder b(root_p{});
            b.token(token_kind::a, str, str + 4);
            return LEXY_MOV(b).finish(str + 4);
        }());
        CHECK(a_abcd.status == test_result::recovered_error);
        CHECK(a_abcd.trace
              == test_trace()
                     .token("literal", "abc")
                     .error_token("d")
                     .error(3, 4, "expected token end"));

        auto a_ab = LEXY_VERIFY_RUNTIME([] {
            auto                str = "ab";
            parse_tree::builder b(root_p{});
            b.token(token_kind::a, str, str + 2);
            return LEXY_MOV(b).finish(str + 2);
        }());
        CHECK(a_ab.status == test_result::fatal_error);
        CHECK(a_ab.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());
    }
}

TEST_CASE("dsl::pnode")
{
    constexpr auto rule = dsl::pnode<child_p>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY_RUNTIME([] {
        parse_tree::builder b(root_p{});
        b.token(lexy::eof_token_kind, nullptr, nullptr);
        return LEXY_MOV(b).finish(nullptr);
    }());
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "child_p").cancel());

    auto child = LEXY_VERIFY_RUNTIME([] {
        auto                str = "abc";
        parse_tree::builder b(root_p{});

        auto m = b.start_production(child_p{});
        b.token(token_kind::a, str, str + 3);
        b.finish_production(LEXY_MOV(m));

        return LEXY_MOV(b).finish(str + 3);
    }());
    CHECK(child.status == test_result::success);
    CHECK(child.trace == test_trace().token("token", "abc"));

    auto child_b = LEXY_VERIFY_RUNTIME([] {
        auto                str = "abc!";
        parse_tree::builder b(root_p{});

        auto m = b.start_production(child_p{});
        b.token(token_kind::a, str, str + 3);
        b.finish_production(LEXY_MOV(m));

        b.token(token_kind::b, str + 3, str + 4);

        return LEXY_MOV(b).finish(str + 4);
    }());
    CHECK(child_b.status == test_result::success);
    CHECK(child_b.trace == test_trace().token("token", "abc"));
}

TEST_CASE("dsl::pnode(rule)")
{
    constexpr auto pnode = dsl::pnode<child_p>(dsl::tnode<token_kind::a>);
    CHECK(lexy::is_branch_rule<decltype(pnode)>);

    constexpr auto callback = lexy_test::token_callback;

    SUBCASE("basic")
    {
        constexpr auto rule = pnode;

        auto empty = LEXY_VERIFY_RUNTIME([] {
            parse_tree::builder b(root_p{});
            b.token(lexy::eof_token_kind, nullptr, nullptr);
            return LEXY_MOV(b).finish(nullptr);
        }());
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "child_p").cancel());

        auto root = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(root_p{});
            b.token(token_kind::a, str, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(root.status == test_result::fatal_error);
        CHECK(root.trace == test_trace().expected_char_class(0, "child_p").cancel());

        auto child_a = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(child_p{});
            b.token(token_kind::a, str, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(child_a.status == test_result::success);
        CHECK(child_a.trace == test_trace().token("a", "abc"));

        auto child_ab = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(child_p{});
            b.token(token_kind::a, str, str + 2);
            b.token(token_kind::b, str + 2, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(child_ab.status == test_result::recovered_error);
        // clang-format off
        CHECK(child_ab.trace == test_trace()
                .token("a", "ab")
                .error_token("c")
                .error(2, 3, "expected production end"));
        // clang-format on

        auto child_b = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(child_p{});
            b.token(token_kind::b, str, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(child_b.status == test_result::fatal_error);
        CHECK(child_b.trace
              == test_trace().expected_char_class(0, "a").error_token("abc").cancel());
    }
    SUBCASE("branch")
    {
        constexpr auto rule = dsl::if_(pnode);

        auto empty = LEXY_VERIFY_RUNTIME([] {
            parse_tree::builder b(root_p{});
            b.token(lexy::eof_token_kind, nullptr, nullptr);
            return LEXY_MOV(b).finish(nullptr);
        }());
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto root = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(root_p{});
            b.token(token_kind::a, str, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(root.status == test_result::success);
        CHECK(root.trace == test_trace());

        auto child_a = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(child_p{});
            b.token(token_kind::a, str, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(child_a.status == test_result::success);
        CHECK(child_a.trace == test_trace().token("a", "abc"));

        auto child_ab = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(child_p{});
            b.token(token_kind::a, str, str + 2);
            b.token(token_kind::b, str + 2, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(child_ab.status == test_result::recovered_error);
        // clang-format off
        CHECK(child_ab.trace == test_trace()
                .token("a", "ab")
                .error_token("c")
                .error(2, 3, "expected production end"));
        // clang-format on

        auto child_b = LEXY_VERIFY_RUNTIME([] {
            auto                str = "abc";
            parse_tree::builder b(root_p{});

            auto m = b.start_production(child_p{});
            b.token(token_kind::b, str, str + 3);
            b.finish_production(LEXY_MOV(m));

            return LEXY_MOV(b).finish(str + 3);
        }());
        CHECK(child_b.status == test_result::fatal_error);
        CHECK(child_b.trace
              == test_trace().expected_char_class(0, "a").error_token("abc").cancel());
    }
}

