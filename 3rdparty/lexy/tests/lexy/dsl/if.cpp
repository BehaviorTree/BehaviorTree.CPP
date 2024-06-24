// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/if.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>

TEST_CASE("dsl::if_()")
{
    constexpr auto rule = dsl::if_(dsl::capture(LEXY_LIT("ab")) >> dsl::capture(LEXY_LIT("cd")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback //
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char* begin, lexy::string_lexeme<> ab,
                                 lexy::string_lexeme<> cd) {
                                  CHECK(ab.size() == 2);
                                  CHECK(ab.begin() == begin);
                                  CHECK(ab[0] == 'a');
                                  CHECK(ab[1] == 'b');

                                  CHECK(cd.size() == 2);
                                  CHECK(cd.begin() == begin + 2);
                                  CHECK(cd[0] == 'c');
                                  CHECK(cd[1] == 'd');

                                  return 1;
                              });

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace());
    auto a = LEXY_VERIFY("a");
    CHECK(a.status == test_result::success);
    CHECK(a.value == 0);
    CHECK(a.trace == test_trace());

    auto ab = LEXY_VERIFY("ab");
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == test_trace().literal("ab").expected_literal(2, "cd", 0).cancel());
    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace
          == test_trace().literal("ab").error_token("c").expected_literal(2, "cd", 1).cancel());

    auto abcd = LEXY_VERIFY("abcd");
    CHECK(abcd.status == test_result::success);
    CHECK(abcd.value == 1);
    CHECK(abcd.trace == test_trace().literal("ab").literal("cd"));
    auto abcde = LEXY_VERIFY("abcde");
    CHECK(abcde.status == test_result::success);
    CHECK(abcde.value == 1);
    CHECK(abcde.trace == test_trace().literal("ab").literal("cd"));
}

TEST_CASE("dsl::if_(unconditional)")
{
    constexpr auto rule = dsl::if_(dsl::else_ >> dsl::capture(LEXY_LIT("cd")));
    CHECK(lexy::is_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::else_ >> dsl::capture(LEXY_LIT("cd"))));
}

