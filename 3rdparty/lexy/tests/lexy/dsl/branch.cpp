// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/branch.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/recover.hpp>

TEST_CASE("dsl::operator>>")
{
    constexpr auto branch
        = dsl::capture(LEXY_LIT("abc")) >> dsl::position + dsl::try_(LEXY_LIT("!"));
    CHECK(lexy::is_branch_rule<decltype(branch)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char* begin, lexy::string_lexeme<> lex, const char* pos) {
                                  CHECK(lex.begin() == begin);
                                  CHECK(lex.size() == 3);
                                  CHECK(lex[0] == 'a');
                                  CHECK(lex[1] == 'b');
                                  CHECK(lex[2] == 'c');

                                  CHECK(pos == begin + 3);

                                  return 1;
                              });

    SUBCASE("as rule")
    {
        constexpr auto rule = branch;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == test_trace().error_token("ab").expected_literal(0, "abc", 2).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::recovered_error);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc").position().expected_literal(3, "!", 0));
        auto abc_mark = LEXY_VERIFY("abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.value == 1);
        CHECK(abc_mark.trace == test_trace().literal("abc").position().literal("!"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(branch);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());
        auto ab = LEXY_VERIFY("ab");
        CHECK(ab.status == test_result::success);
        CHECK(ab.value == 0);
        CHECK(ab.trace == test_trace());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::recovered_error);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc").position().expected_literal(3, "!", 0));
        auto abc_mark = LEXY_VERIFY("abc!");
        CHECK(abc_mark.status == test_result::success);
        CHECK(abc_mark.value == 1);
        CHECK(abc_mark.trace == test_trace().literal("abc").position().literal("!"));
    }
}

TEST_CASE("dsl::operator+ and dsl::operator>> combinations")
{
    SUBCASE("nested operator>>")
    {
        constexpr auto rule = LEXY_LIT("a") >> LEXY_LIT("b") >> LEXY_LIT("c");
        CHECK(equivalent_rules(rule, LEXY_LIT("a") >> LEXY_LIT("b") + LEXY_LIT("c")));
    }

    SUBCASE("rule + branch")
    {
        constexpr auto rule = LEXY_LIT("a") + (LEXY_LIT("b") >> LEXY_LIT("c"));
        CHECK(equivalent_rules(rule, LEXY_LIT("a") + LEXY_LIT("b") + LEXY_LIT("c")));
    }
    SUBCASE("sequence + branch")
    {
        constexpr auto rule = (LEXY_LIT("a") + LEXY_LIT("b")) + (LEXY_LIT("c") >> LEXY_LIT("d"));
        CHECK(equivalent_rules(rule, //
                               LEXY_LIT("a") + LEXY_LIT("b") + LEXY_LIT("c") + LEXY_LIT("d")));
    }

    SUBCASE("branch + rule")
    {
        constexpr auto rule = (LEXY_LIT("a") >> LEXY_LIT("b")) + LEXY_LIT("c");
        CHECK(equivalent_rules(rule, LEXY_LIT("a") >> LEXY_LIT("b") + LEXY_LIT("c")));
    }
    SUBCASE("branch + sequence")
    {
        constexpr auto rule = (LEXY_LIT("a") >> LEXY_LIT("b")) + (LEXY_LIT("c") + LEXY_LIT("d"));
        CHECK(equivalent_rules(rule, //
                               LEXY_LIT("a") >> LEXY_LIT("b") + LEXY_LIT("c") + LEXY_LIT("d")));
    }

    SUBCASE("branch + branch")
    {
        constexpr auto rule = (LEXY_LIT("a") >> LEXY_LIT("b")) + (LEXY_LIT("c") >> LEXY_LIT("d"));
        CHECK(equivalent_rules(rule, //
                               LEXY_LIT("a") >> LEXY_LIT("b") + LEXY_LIT("c") + LEXY_LIT("d")));
    }
}

TEST_CASE("dsl::else_")
{
    constexpr auto branch = dsl::else_ >> LEXY_LIT("abc");
    CHECK(!lexy::is_rule<decltype(dsl::else_)>);
    CHECK(lexy::is_unconditional_branch_rule<decltype(branch)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = branch;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = branch | LEXY_LIT("123");

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("abc"));
    }
}

