// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/recover.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/until.hpp>
#include <lexy/dsl/whitespace.hpp>

TEST_CASE("dsl::find()")
{
    constexpr auto rule = dsl::find(LEXY_LIT("!"), LEXY_LIT("."), LEXY_LIT(";"));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, dsl::find(dsl::literal_set(LEXY_LIT("!"), LEXY_LIT("."),
                                                            LEXY_LIT(";")))));

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().recovery().cancel().cancel());

    auto zero = LEXY_VERIFY("!");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().recovery().finish());

    auto one = LEXY_VERIFY("a!");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().recovery().error_token("a").finish());
    auto two = LEXY_VERIFY("ab.");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().recovery().error_token("ab").finish());
    auto three = LEXY_VERIFY("abc;");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().recovery().error_token("abc").finish());

    auto multiple = LEXY_VERIFY("abc;.!");
    CHECK(multiple.status == test_result::success);
    CHECK(multiple.trace == test_trace().recovery().error_token("abc").finish());

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated.status == test_result::fatal_error);
    CHECK(unterminated.trace == test_trace().recovery().error_token("abc").cancel().cancel());
}

TEST_CASE("dsl::find().limit()")
{
    constexpr auto rule
        = dsl::find(LEXY_LIT("!"), LEXY_LIT(".")).limit(LEXY_LIT(";"), LEXY_LIT(","));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, dsl::find(dsl::literal_set(LEXY_LIT("!"), LEXY_LIT(".")))
                                     .limit(LEXY_LIT(";"), LEXY_LIT(","))));

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().recovery().cancel().cancel());

    auto zero = LEXY_VERIFY("!");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().recovery().finish());

    auto one = LEXY_VERIFY("a!");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().recovery().error_token("a").finish());
    auto two = LEXY_VERIFY("ab.");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().recovery().error_token("ab").finish());
    auto three = LEXY_VERIFY("abc!");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().recovery().error_token("abc").finish());

    auto multiple = LEXY_VERIFY("abc!.");
    CHECK(multiple.status == test_result::success);
    CHECK(multiple.trace == test_trace().recovery().error_token("abc").finish());

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated.status == test_result::fatal_error);
    CHECK(unterminated.trace == test_trace().recovery().error_token("abc").cancel().cancel());

    auto limited = LEXY_VERIFY("abc;def");
    CHECK(limited.status == test_result::fatal_error);
    CHECK(limited.trace == test_trace().recovery().error_token("abc").cancel().cancel());
}

TEST_CASE("dsl::recover()")
{
    constexpr auto rule = dsl::recover(LEXY_LIT("!"), LEXY_LIT("."), LEXY_LIT(";"));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().recovery().cancel().cancel());

    auto zero = LEXY_VERIFY("!");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().recovery().finish().literal("!"));

    auto one = LEXY_VERIFY("a!");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().recovery().error_token("a").finish().literal("!"));
    auto two = LEXY_VERIFY("ab.");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().recovery().error_token("ab").finish().literal("."));
    auto three = LEXY_VERIFY("abc;");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().recovery().error_token("abc").finish().literal(";"));

    auto multiple = LEXY_VERIFY("abc;.!");
    CHECK(multiple.status == test_result::success);
    CHECK(multiple.trace == test_trace().recovery().error_token("abc").finish().literal(";"));

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated.status == test_result::fatal_error);
    CHECK(unterminated.trace == test_trace().recovery().error_token("abc").cancel().cancel());
}

TEST_CASE("dsl::recover().limit()")
{
    constexpr auto rule
        = dsl::recover(LEXY_LIT("!"), LEXY_LIT(".")).limit(LEXY_LIT(";"), LEXY_LIT(","));
    CHECK(lexy::is_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, dsl::recover(LEXY_LIT("!"), LEXY_LIT("."))
                                     .limit(LEXY_LIT(";"), LEXY_LIT(","))));

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().recovery().cancel().cancel());

    auto zero = LEXY_VERIFY("!");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().recovery().finish().literal("!"));

    auto one = LEXY_VERIFY("a!");
    CHECK(one.status == test_result::success);
    CHECK(one.trace == test_trace().recovery().error_token("a").finish().literal("!"));
    auto two = LEXY_VERIFY("ab.");
    CHECK(two.status == test_result::success);
    CHECK(two.trace == test_trace().recovery().error_token("ab").finish().literal("."));
    auto three = LEXY_VERIFY("abc!");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().recovery().error_token("abc").finish().literal("!"));

    auto multiple = LEXY_VERIFY("abc!.");
    CHECK(multiple.status == test_result::success);
    CHECK(multiple.trace == test_trace().recovery().error_token("abc").finish().literal("!"));

    auto unterminated = LEXY_VERIFY("abc");
    CHECK(unterminated.status == test_result::fatal_error);
    CHECK(unterminated.trace == test_trace().recovery().error_token("abc").cancel().cancel());

    auto limited = LEXY_VERIFY("abc;def");
    CHECK(limited.status == test_result::fatal_error);
    CHECK(limited.trace == test_trace().recovery().error_token("abc").cancel().cancel());
}

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = dsl::whitespace(dsl::lit_c<'.'>);
};
} // namespace

TEST_CASE("dsl::try_(rule)")
{
    SUBCASE("token")
    {
        constexpr auto try_ = dsl::try_(LEXY_LIT("abc"));
        constexpr auto rule = try_ + LEXY_LIT("!");
        CHECK(lexy::is_branch_rule<decltype(try_)>);

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace()
                     .expected_literal(0, "abc", 0)
                     .recovery()
                     .finish()
                     .expected_literal(0, "!", 0)
                     .cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace
              == test_trace()
                     .error_token("a")
                     .expected_literal(0, "abc", 1)
                     .recovery()
                     .finish()
                     .expected_literal(1, "!", 0)
                     .cancel());
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .finish()
                     .expected_literal(2, "!", 0)
                     .cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().literal("abc").expected_literal(3, "!", 0).cancel());

        auto abc_mark = LEXY_VERIFY("abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.trace == test_trace().literal("abc").literal("!"));

        auto mark = LEXY_VERIFY("!");
        CHECK(mark.status == test_result::recovered_error);
        CHECK(mark.trace
              == test_trace().expected_literal(0, "abc", 0).recovery().finish().literal("!"));

        auto ab_mark = LEXY_VERIFY("ab!");
        CHECK(ab_mark.status == test_result::recovered_error);
        CHECK(ab_mark.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .finish()
                     .literal("!"));
    }
    SUBCASE("rule")
    {
        constexpr auto try_ = dsl::try_(LEXY_LIT("ab") + dsl::position + LEXY_LIT("c"));
        constexpr auto rule = try_ + LEXY_LIT("!");
        CHECK(lexy::is_rule<decltype(try_)>);

        constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                      [](const char*, const char*) { return 1; });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace()
                     .expected_literal(0, "ab", 0)
                     .recovery()
                     .finish()
                     .expected_literal(0, "!", 0)
                     .cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace
              == test_trace()
                     .error_token("a")
                     .expected_literal(0, "ab", 1)
                     .recovery()
                     .finish()
                     .expected_literal(1, "!", 0)
                     .cancel());

        auto ab       = LEXY_VERIFY("ab");
        auto ab_trace = test_trace()
                            .literal("ab")
                            .position()
                            .expected_literal(2, "c", 0)
                            .recovery()
                            .finish()
                            .expected_literal(2, "!", 0)
                            .cancel();
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == ab_trace);

        auto abc       = LEXY_VERIFY("abc");
        auto abc_trace = test_trace()
                             .literal("ab")
                             .position()
                             .literal("c")
                             .expected_literal(3, "!", 0)
                             .cancel();
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == abc_trace);

        auto abc_mark = LEXY_VERIFY("abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.value == 1);
        CHECK(abc_mark.trace == test_trace().literal("ab").position().literal("c").literal("!"));

        auto mark = LEXY_VERIFY("!");
        CHECK(mark.status == test_result::recovered_error);
        CHECK(mark.value == 0);
        CHECK(mark.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().finish().literal("!"));

        auto ab_mark = LEXY_VERIFY("ab!");
        CHECK(ab_mark.status == test_result::recovered_error);
        CHECK(ab_mark.value == 0);
        CHECK(ab_mark.trace
              == test_trace()
                     .literal("ab")
                     .position()
                     .expected_literal(2, "c", 0)
                     .recovery()
                     .finish()
                     .literal("!"));
    }

    SUBCASE("as branch")
    {
        constexpr auto try_ = dsl::try_(LEXY_LIT("ab") >> dsl::position + LEXY_LIT("c"));
        constexpr auto rule = dsl::if_(try_) + LEXY_LIT("!");
        CHECK(lexy::is_branch_rule<decltype(try_)>);

        constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                      [](const char*, const char*) { return 1; });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "!", 0).cancel());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == test_trace().expected_literal(0, "!", 0).cancel());

        auto ab       = LEXY_VERIFY("ab");
        auto ab_trace = test_trace()
                            .literal("ab")
                            .position()
                            .expected_literal(2, "c", 0)
                            .recovery()
                            .finish()
                            .expected_literal(2, "!", 0)
                            .cancel();
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == ab_trace);

        auto abc       = LEXY_VERIFY("abc");
        auto abc_trace = test_trace()
                             .literal("ab")
                             .position()
                             .literal("c")
                             .expected_literal(3, "!", 0)
                             .cancel();
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == abc_trace);

        auto abc_mark = LEXY_VERIFY("abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.value == 1);
        CHECK(abc_mark.trace == test_trace().literal("ab").position().literal("c").literal("!"));

        auto mark = LEXY_VERIFY("!");
        CHECK(mark.status == test_result::success);
        CHECK(mark.value == 0);
        CHECK(mark.trace == test_trace().literal("!"));

        auto ab_mark = LEXY_VERIFY("ab!");
        CHECK(ab_mark.status == test_result::recovered_error);
        CHECK(ab_mark.value == 0);
        CHECK(ab_mark.trace
              == test_trace()
                     .literal("ab")
                     .position()
                     .expected_literal(2, "c", 0)
                     .recovery()
                     .finish()
                     .literal("!"));
    }
    SUBCASE("with whitespace")
    {
        constexpr auto try_ = dsl::try_(LEXY_LIT("abc"));

        struct production : test_production_for<decltype(try_ + dsl::lit_c<'!'>)>, with_whitespace
        {};

        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY_P(production, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace()
                     .expected_literal(0, "abc", 0)
                     .recovery()
                     .finish()
                     .expected_literal(0, "!", 0)
                     .cancel());

        auto a = LEXY_VERIFY_P(production, "a");
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace
              == test_trace()
                     .error_token("a")
                     .expected_literal(0, "abc", 1)
                     .recovery()
                     .finish()
                     .expected_literal(1, "!", 0)
                     .cancel());
        auto ab = LEXY_VERIFY_P(production, "ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .finish()
                     .expected_literal(2, "!", 0)
                     .cancel());

        auto abc = LEXY_VERIFY_P(production, "abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().literal("abc").expected_literal(3, "!", 0).cancel());

        auto abc_mark = LEXY_VERIFY_P(production, "abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.trace == test_trace().literal("abc").literal("!"));
        auto abc_ws_mark = LEXY_VERIFY_P(production, "abc..!");
        CHECK(abc_ws_mark.status == test_result::success);
        CHECK(abc_ws_mark.trace == test_trace().literal("abc").whitespace("..").literal("!"));

        auto mark = LEXY_VERIFY_P(production, "!");
        CHECK(mark.status == test_result::recovered_error);
        CHECK(mark.trace
              == test_trace().expected_literal(0, "abc", 0).recovery().finish().literal("!"));
        auto ws_mark = LEXY_VERIFY_P(production, "..!");
        CHECK(ws_mark.status == test_result::recovered_error);
        CHECK(ws_mark.trace
              == test_trace()
                     .whitespace("..")
                     .expected_literal(2, "abc", 0)
                     .recovery()
                     .finish()
                     .literal("!"));

        auto ab_mark = LEXY_VERIFY_P(production, "ab!");
        CHECK(ab_mark.status == test_result::recovered_error);
        CHECK(ab_mark.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .finish()
                     .literal("!"));
        auto ab_ws_mark = LEXY_VERIFY_P(production, "ab..!");
        CHECK(ab_ws_mark.status == test_result::recovered_error);
        CHECK(ab_ws_mark.trace
              == test_trace()
                     .error_token("ab")
                     .expected_literal(0, "abc", 2)
                     .recovery()
                     .whitespace("..")
                     .finish()
                     .literal("!"));
    }
}

TEST_CASE("dsl::try_(rule, recover)")
{
    SUBCASE("recover using find")
    {
        constexpr auto try_
            = dsl::try_(LEXY_LIT("ab") + dsl::position + LEXY_LIT("c"), dsl::find(LEXY_LIT("!")));
        constexpr auto rule = try_ + LEXY_LIT("!");
        CHECK(lexy::is_rule<decltype(try_)>);

        constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                      [](const char*, const char*) { return 1; });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().cancel().cancel());

        auto a       = LEXY_VERIFY("a");
        auto a_trace = test_trace()
                           .error_token("a")
                           .expected_literal(0, "ab", 1)
                           .recovery()
                           .cancel()
                           .cancel();
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == a_trace);

        auto ab       = LEXY_VERIFY("ab");
        auto ab_trace = test_trace()
                            .literal("ab")
                            .position()
                            .expected_literal(2, "c", 0)
                            .recovery()
                            .cancel()
                            .cancel();
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == ab_trace);

        auto abc       = LEXY_VERIFY("abc");
        auto abc_trace = test_trace()
                             .literal("ab")
                             .position()
                             .literal("c")
                             .expected_literal(3, "!", 0)
                             .cancel();
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == abc_trace);

        auto abc_mark = LEXY_VERIFY("abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.value == 1);
        CHECK(abc_mark.trace == test_trace().literal("ab").position().literal("c").literal("!"));

        auto mark = LEXY_VERIFY("!");
        CHECK(mark.status == test_result::recovered_error);
        CHECK(mark.value == 0);
        CHECK(mark.trace
              == test_trace().expected_literal(0, "ab", 0).recovery().finish().literal("!"));

        auto ab_mark       = LEXY_VERIFY("ab!");
        auto ab_mark_trace = test_trace()
                                 .literal("ab")
                                 .position()
                                 .expected_literal(2, "c", 0)
                                 .recovery()
                                 .finish()
                                 .literal("!");
        CHECK(ab_mark.status == test_result::recovered_error);
        CHECK(ab_mark.value == 0);
        CHECK(ab_mark.trace == ab_mark_trace);
    }
    SUBCASE("recover using custom rule")
    {
        constexpr auto my_find = dsl::until(dsl::token(dsl::peek(LEXY_LIT("!"))));

        constexpr auto try_ = dsl::try_(LEXY_LIT("ab") + dsl::position + LEXY_LIT("c"), my_find);
        constexpr auto rule = try_ + LEXY_LIT("!");
        CHECK(lexy::is_rule<decltype(try_)>);

        constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                      [](const char*, const char*) { return 1; });

        auto empty       = LEXY_VERIFY("");
        auto empty_trace = test_trace()
                               .expected_literal(0, "ab", 0)
                               .recovery()
                               .error(0, 0, "missing token")
                               .cancel()
                               .cancel();
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == empty_trace);

        auto a       = LEXY_VERIFY("a");
        auto a_trace = test_trace()
                           .error_token("a")
                           .expected_literal(0, "ab", 1)
                           .recovery()
                           .error(1, 1, "missing token")
                           .cancel()
                           .cancel();
        CHECK(a.status == test_result::fatal_error);
        CHECK(a.trace == a_trace);

        auto ab       = LEXY_VERIFY("ab");
        auto ab_trace = test_trace()
                            .literal("ab")
                            .position()
                            .expected_literal(2, "c", 0)
                            .recovery()
                            .error(2, 2, "missing token")
                            .cancel()
                            .cancel();
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == ab_trace);

        auto abc       = LEXY_VERIFY("abc");
        auto abc_trace = test_trace()
                             .literal("ab")
                             .position()
                             .literal("c")
                             .expected_literal(3, "!", 0)
                             .cancel();
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == abc_trace);

        auto abc_mark = LEXY_VERIFY("abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.value == 1);
        CHECK(abc_mark.trace == test_trace().literal("ab").position().literal("c").literal("!"));

        auto mark = LEXY_VERIFY("!");
        CHECK(mark.status == test_result::recovered_error);
        CHECK(mark.value == 0);
        CHECK(mark.trace
              == test_trace()
                     .expected_literal(0, "ab", 0)
                     .recovery()
                     .token("any", "")
                     .finish()
                     .literal("!"));

        auto ab_mark       = LEXY_VERIFY("ab!");
        auto ab_mark_trace = test_trace()
                                 .literal("ab")
                                 .position()
                                 .expected_literal(2, "c", 0)
                                 .recovery()
                                 .token("any", "")
                                 .finish()
                                 .literal("!");
        CHECK(ab_mark.status == test_result::recovered_error);
        CHECK(ab_mark.value == 0);
        CHECK(ab_mark.trace == ab_mark_trace);
    }
}

