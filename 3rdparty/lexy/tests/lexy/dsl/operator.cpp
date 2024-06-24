// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/operator.hpp>

#include "verify.hpp"
#include <lexy/dsl/brackets.hpp>
#include <lexy/dsl/if.hpp>

// We can't use `lexy::op`, GCC 7 doesn't like it combined with function-local statics.
#define LEXY_OP_OF(Rule) LEXY_DECAY_DECLTYPE(Rule)::op_tag_type

TEST_CASE("dsl::op")
{
    SUBCASE("token")
    {
        constexpr auto rule = dsl::op(dsl::lit_c<'+'>);
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = [](const char*, LEXY_OP_OF(rule)) { return 0; };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        CHECK(empty.trace == test_trace().expected_literal(0, "+", 0).cancel());

        auto plus = LEXY_VERIFY("+");
        CHECK(plus.status == test_result::success);
        CHECK(plus.value == 0);
        CHECK(plus.trace == test_trace().literal("+"));

        auto double_plus = LEXY_VERIFY("++");
        CHECK(double_plus.status == test_result::success);
        CHECK(double_plus.value == 0);
        CHECK(double_plus.trace == test_trace().literal("+"));
    }
    SUBCASE("branch")
    {
        constexpr auto rule = dsl::op(dsl::square_bracketed(LEXY_LIT("0")));
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        constexpr auto callback = [](const char*, LEXY_OP_OF(rule)) { return 0; };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        CHECK(empty.trace == test_trace().expected_literal(0, "[", 0).cancel());

        auto token_only = LEXY_VERIFY("[");
        CHECK(token_only.status == test_result::fatal_error);
        CHECK(token_only.value == -1);
        CHECK(token_only.trace == test_trace().literal("[").expected_literal(1, "0", 0).cancel());

        auto success = LEXY_VERIFY("[0]");
        CHECK(success.status == test_result::success);
        CHECK(success.value == 0);
        CHECK(success.trace == test_trace().literal("[").literal("0").literal("]"));
    }

    SUBCASE("custom tag")
    {
        struct tag
        {
            const char* pos;

            constexpr explicit tag(const char* pos) : pos(pos) {}
        };
        constexpr auto rule = dsl::op<tag>(dsl::square_bracketed(LEXY_LIT("0")));
        CHECK(std::is_same_v<LEXY_OP_OF(rule), tag>);

        constexpr auto callback = [](const char* pos, LEXY_OP_OF(rule) t) {
            CHECK(pos == t.pos);
            return 0;
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        CHECK(empty.trace == test_trace().expected_literal(0, "[", 0).cancel());

        auto token_only = LEXY_VERIFY("[");
        CHECK(token_only.status == test_result::fatal_error);
        CHECK(token_only.value == -1);
        CHECK(token_only.trace == test_trace().literal("[").expected_literal(1, "0", 0).cancel());

        auto success = LEXY_VERIFY("[0]");
        CHECK(success.status == test_result::success);
        CHECK(success.value == 0);
        CHECK(success.trace == test_trace().literal("[").literal("0").literal("]"));
    }
    SUBCASE("custom tag value")
    {
        constexpr auto rule = dsl::op<42>(dsl::square_bracketed(LEXY_LIT("0")));

        constexpr auto callback = [](const char*, LEXY_OP_OF(rule) t) {
            CHECK(t == 42);
            return 0;
        };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        CHECK(empty.trace == test_trace().expected_literal(0, "[", 0).cancel());

        auto token_only = LEXY_VERIFY("[");
        CHECK(token_only.status == test_result::fatal_error);
        CHECK(token_only.value == -1);
        CHECK(token_only.trace == test_trace().literal("[").expected_literal(1, "0", 0).cancel());

        auto success = LEXY_VERIFY("[0]");
        CHECK(success.status == test_result::success);
        CHECK(success.value == 0);
        CHECK(success.trace == test_trace().literal("[").literal("0").literal("]"));
    }
    SUBCASE("no value")
    {
        constexpr auto rule = dsl::op<void>(dsl::square_bracketed(LEXY_LIT("0")));

        constexpr auto callback = [](const char*) { return 0; };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        CHECK(empty.trace == test_trace().expected_literal(0, "[", 0).cancel());

        auto token_only = LEXY_VERIFY("[");
        CHECK(token_only.status == test_result::fatal_error);
        CHECK(token_only.value == -1);
        CHECK(token_only.trace == test_trace().literal("[").expected_literal(1, "0", 0).cancel());

        auto success = LEXY_VERIFY("[0]");
        CHECK(success.status == test_result::success);
        CHECK(success.value == 0);
        CHECK(success.trace == test_trace().literal("[").literal("0").literal("]"));
    }

    SUBCASE("as branch")
    {
        constexpr auto op   = dsl::op(dsl::square_bracketed(LEXY_LIT("0")));
        constexpr auto rule = dsl::if_(op);

        constexpr auto callback
            = lexy::callback<int>([](const char*) { return 0; },
                                  [](const char*, LEXY_OP_OF(op)) { return 1; });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto token_only = LEXY_VERIFY("[");
        CHECK(token_only.status == test_result::fatal_error);
        CHECK(token_only.value == -1);
        CHECK(token_only.trace == test_trace().literal("[").expected_literal(1, "0", 0).cancel());

        auto success = LEXY_VERIFY("[0]");
        CHECK(success.status == test_result::success);
        CHECK(success.value == 1);
        CHECK(success.trace == test_trace().literal("[").literal("0").literal("]"));
    }
    SUBCASE("as branch with custom tag")
    {
        struct tag
        {
            const char* pos;

            constexpr explicit tag(const char* pos) : pos(pos) {}
        };
        constexpr auto op = dsl::op<tag>(dsl::square_bracketed(LEXY_LIT("0")));
        CHECK(std::is_same_v<LEXY_OP_OF(op), tag>);

        constexpr auto rule = dsl::if_(op);

        constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                      [](const char* pos, LEXY_OP_OF(op) t) {
                                                          CHECK(pos == t.pos);
                                                          return 1;
                                                      });

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto token_only = LEXY_VERIFY("[");
        CHECK(token_only.status == test_result::fatal_error);
        CHECK(token_only.value == -1);
        CHECK(token_only.trace == test_trace().literal("[").expected_literal(1, "0", 0).cancel());

        auto success = LEXY_VERIFY("[0]");
        CHECK(success.status == test_result::success);
        CHECK(success.value == 1);
        CHECK(success.trace == test_trace().literal("[").literal("0").literal("]"));
    }
}

TEST_CASE("dsl::op choice")
{
    constexpr auto op_plus        = dsl::op(dsl::lit_c<'+'>);
    constexpr auto op_double_plus = dsl::op(LEXY_LIT("++"));
    constexpr auto op_minus       = dsl::op(dsl::lit_c<'-'>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char*, LEXY_OP_OF(op_plus)) { return 1; },
                              [](const char*, LEXY_OP_OF(op_double_plus)) { return 2; },
                              [](const char*, LEXY_OP_OF(op_minus)) { return 3; });

    SUBCASE("as rule")
    {
        constexpr auto rule = op_plus / op_double_plus / op_minus;
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.value == -1);
        CHECK(empty.trace == test_trace().error(0, 0, "expected literal set").cancel());

        auto plus = LEXY_VERIFY("+");
        CHECK(plus.status == test_result::success);
        CHECK(plus.value == 1);
        CHECK(plus.trace == test_trace().literal("+"));

        auto double_plus = LEXY_VERIFY("++");
        CHECK(double_plus.status == test_result::success);
        CHECK(double_plus.value == 2);
        CHECK(double_plus.trace == test_trace().literal("++"));

        auto minus = LEXY_VERIFY("-");
        CHECK(minus.status == test_result::success);
        CHECK(minus.value == 3);
        CHECK(minus.trace == test_trace().literal("-"));

        auto double_minus = LEXY_VERIFY("--");
        CHECK(double_minus.status == test_result::success);
        CHECK(double_minus.value == 3);
        CHECK(double_minus.trace == test_trace().literal("-"));
    }
    SUBCASE("as branch rule")
    {
        constexpr auto rule = dsl::if_(op_plus / op_double_plus / op_minus);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto plus = LEXY_VERIFY("+");
        CHECK(plus.status == test_result::success);
        CHECK(plus.value == 1);
        CHECK(plus.trace == test_trace().literal("+"));

        auto double_plus = LEXY_VERIFY("++");
        CHECK(double_plus.status == test_result::success);
        CHECK(double_plus.value == 2);
        CHECK(double_plus.trace == test_trace().literal("++"));

        auto minus = LEXY_VERIFY("-");
        CHECK(minus.status == test_result::success);
        CHECK(minus.value == 3);
        CHECK(minus.trace == test_trace().literal("-"));

        auto double_minus = LEXY_VERIFY("--");
        CHECK(double_minus.status == test_result::success);
        CHECK(double_minus.value == 3);
        CHECK(double_minus.trace == test_trace().literal("-"));
    }
}

