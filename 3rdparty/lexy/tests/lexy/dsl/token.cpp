// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/token.hpp>

#include "verify.hpp"
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};
} // namespace

TEST_CASE("token whitespace")
{
    struct production : test_production_for<decltype(LEXY_LIT("abc"))>, with_whitespace
    {};

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY_P(production, "");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

    auto abc = LEXY_VERIFY_P(production, "abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("abc"));

    auto leading_ws = LEXY_VERIFY_P(production, "..abc");
    CHECK(leading_ws.status == test_result::success);
    CHECK(leading_ws.trace == test_trace().whitespace("..").literal("abc"));
    auto inner_ws = LEXY_VERIFY_P(production, "ab..c");
    CHECK(inner_ws.status == test_result::fatal_error);
    CHECK(inner_ws.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());
    auto trailing_ws = LEXY_VERIFY_P(production, "abc..");
    CHECK(trailing_ws.status == test_result::success);
    CHECK(trailing_ws.trace == test_trace().literal("abc").whitespace(".."));
}

namespace
{
struct my_error
{
    static constexpr auto name()
    {
        return "my_error";
    }
};
} // namespace

TEST_CASE("token.error<Tag>")
{
    // We use literal, as it has a pre-defined kind that needs to be maintained.
    constexpr auto rule = LEXY_LIT("abc").error<my_error>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().literal("abc"));
}

namespace
{
enum class token_kind
{
    my_kind,
};

[[maybe_unused]] constexpr const char* token_kind_name(token_kind)
{
    return "my_kind";
}
} // namespace

TEST_CASE("token.kind<Tag>")
{
    // We use literal, as it has a pre-defined kind that needs to be overriden.
    constexpr auto rule = LEXY_LIT("abc").kind<token_kind::my_kind>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());
}

TEST_CASE("token.kind<Tag>.error<Tag>")
{
    constexpr auto rule = LEXY_LIT("abc").kind<token_kind::my_kind>.error<my_error>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().token("my_kind", "abc"));
}

TEST_CASE("token.error<Tag>.kind<Tag>")
{
    constexpr auto rule = LEXY_LIT("abc").error<my_error>.kind<token_kind::my_kind>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "my_error").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().token("my_kind", "abc"));
}

TEST_CASE("dsl::token()")
{
    constexpr auto rule = dsl::token(dsl::list(LEXY_LIT("ab") >> dsl::lit_c<'c'> + dsl::position));
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "missing token").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace().token("abc"));
    auto abcabc = LEXY_VERIFY("abcabc");
    CHECK(abcabc.status == test_result::success);
    CHECK(abcabc.trace == test_trace().token("abcabc"));
    auto abcabcabc = LEXY_VERIFY("abcabcabc");
    CHECK(abcabcabc.status == test_result::success);
    CHECK(abcabcabc.trace == test_trace().token("abcabcabc"));

    auto abca = LEXY_VERIFY("abca");
    CHECK(abca.status == test_result::success);
    CHECK(abca.trace == test_trace().token("abc"));

    auto a = LEXY_VERIFY("a");
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == test_trace().error_token("a").error(0, 1, "missing token").cancel());
    auto ab = LEXY_VERIFY("ab");
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().error_token("ab").error(0, 2, "missing token").cancel());
    auto abd = LEXY_VERIFY("abd");
    CHECK(abd.status == test_result::fatal_error);
    CHECK(abd.trace == test_trace().error_token("ab").error(0, 2, "missing token").cancel());

    auto abcabd = LEXY_VERIFY("abcabd");
    CHECK(abcabd.status == test_result::fatal_error);
    CHECK(abcabd.trace == test_trace().error_token("abcab").error(0, 5, "missing token").cancel());
}

TEST_CASE("dsl::token(token)")
{
    constexpr auto rule = dsl::token(LEXY_LIT("abc"));
    CHECK(equivalent_rules(rule, LEXY_LIT("abc")));
}

