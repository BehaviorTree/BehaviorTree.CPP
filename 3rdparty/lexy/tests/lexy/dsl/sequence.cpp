// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/sequence.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/recover.hpp>

TEST_CASE("dsl::operator+")
{
    constexpr auto rule = dsl::lit_c<'a'> + dsl::position + dsl::try_(LEXY_LIT("bc"))
                          + dsl::capture(LEXY_LIT("de"));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = [](const char* begin, const char* pos, lexy::string_lexeme<> lexeme) {
        CHECK(pos == begin + 1);

        CHECK(lexeme.begin() <= begin + 4);
        CHECK(lexeme.size() == 2);
        CHECK(lexeme[0] == 'd');
        CHECK(lexeme[1] == 'e');

        return 0;
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

    auto a       = LEXY_VERIFY("a");
    auto a_trace = test_trace()
                       .literal("a")
                       .position()
                       .expected_literal(1, "bc", 0)
                       .expected_literal(1, "de", 0)
                       .cancel();
    CHECK(a.status == test_result::fatal_error);
    CHECK(a.trace == a_trace);

    auto ab       = LEXY_VERIFY("ab");
    auto ab_trace = test_trace()
                        .literal("a")
                        .position()
                        .error_token("b")
                        .expected_literal(1, "bc", 1)
                        .expected_literal(2, "de", 0)
                        .cancel();
    CHECK(ab.status == test_result::fatal_error);
    CHECK(ab.trace == ab_trace);

    auto abc = LEXY_VERIFY("abc");
    auto abc_trace
        = test_trace().literal("a").position().literal("bc").expected_literal(3, "de", 0).cancel();
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace == abc_trace);

    auto abcd       = LEXY_VERIFY("abcd");
    auto abcd_trace = test_trace()
                          .literal("a")
                          .position()
                          .literal("bc")
                          .error_token("d")
                          .expected_literal(3, "de", 1)
                          .cancel();
    CHECK(abcd.status == test_result::fatal_error);
    CHECK(abcd.trace == abcd_trace);

    auto abcde       = LEXY_VERIFY("abcde");
    auto abcde_trace = test_trace().literal("a").position().literal("bc").literal("de");
    CHECK(abcde.status == test_result::success);
    CHECK(abcde.trace == abcde_trace);
    auto abcdef       = LEXY_VERIFY("abcdef");
    auto abcdef_trace = test_trace().literal("a").position().literal("bc").literal("de");
    CHECK(abcdef.status == test_result::success);
    CHECK(abcdef.trace == abcdef_trace);

    auto ade = LEXY_VERIFY("ade");
    auto ade_trace
        = test_trace().literal("a").position().expected_literal(1, "bc", 0).literal("de");
    CHECK(ade.status == test_result::recovered_error);
    CHECK(ade.trace == ade_trace);
}

