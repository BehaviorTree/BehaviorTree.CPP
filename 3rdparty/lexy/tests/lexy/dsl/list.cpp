// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/list.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/recover.hpp>

TEST_CASE("dsl::list(branch)")
{
    constexpr auto list = dsl::list(LEXY_LIT("a") >> dsl::position + dsl::try_(LEXY_LIT("bc")));
    CHECK(lexy::is_branch_rule<decltype(list)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char*, std::size_t n) { return static_cast<int>(n); });

    SUBCASE("as rule")
    {
        constexpr auto rule = list;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abcabc");
        auto two_trace = test_trace(one_trace).literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abcabcabc");
        auto three_trace = test_trace(two_trace).literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("aabc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(list);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abcabc");
        auto two_trace = test_trace(one_trace).literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abcabcabc");
        auto three_trace = test_trace(two_trace).literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("aabc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);
    }
}

TEST_CASE("dsl::list(rule, sep)")
{
    constexpr auto rule = dsl::list(LEXY_LIT("a") + dsl::position + dsl::try_(LEXY_LIT("bc")),
                                    dsl::sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char*, std::size_t n) { return static_cast<int>(n); });

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

    auto one       = LEXY_VERIFY("abc");
    auto one_trace = test_trace().literal("a").position().literal("bc");
    CHECK(one.status == test_result::success);
    CHECK(one.value == 1);
    CHECK(one.trace == one_trace);
    auto two       = LEXY_VERIFY("abc,abc");
    auto two_trace = test_trace(one_trace).literal(",").literal("a").position().literal("bc");
    CHECK(two.status == test_result::success);
    CHECK(two.value == 2);
    CHECK(two.trace == two_trace);
    auto three       = LEXY_VERIFY("abc,abc,abc");
    auto three_trace = test_trace(two_trace).literal(",").literal("a").position().literal("bc");
    CHECK(three.status == test_result::success);
    CHECK(three.value == 3);
    CHECK(three.trace == three_trace);

    auto recover       = LEXY_VERIFY("a,abc");
    auto recover_trace = test_trace()
                             .literal("a")
                             .position()
                             .expected_literal(1, "bc", 0)
                             .literal(",")
                             .literal("a")
                             .position()
                             .literal("bc");
    CHECK(recover.status == test_result::recovered_error);
    CHECK(recover.value == 2);
    CHECK(recover.trace == recover_trace);

    auto missing_sep       = LEXY_VERIFY("abcabc");
    auto missing_sep_trace = test_trace().literal("a").position().literal("bc");
    CHECK(missing_sep.status == test_result::success);
    CHECK(missing_sep.value == 1);
    CHECK(missing_sep.trace == missing_sep_trace);

    auto trailing_sep       = LEXY_VERIFY("abc,");
    auto trailing_sep_trace = test_trace()
                                  .literal("a")
                                  .position()
                                  .literal("bc")
                                  .literal(",")
                                  .expected_literal(4, "a", 0)
                                  .cancel();
    CHECK(trailing_sep.status == test_result::fatal_error);
    CHECK(trailing_sep.trace == trailing_sep_trace);
}

TEST_CASE("dsl::list(branch, sep)")
{
    constexpr auto list = dsl::list(LEXY_LIT("a") >> dsl::position + dsl::try_(LEXY_LIT("bc")),
                                    dsl::sep(LEXY_LIT(",")));
    CHECK(lexy::is_branch_rule<decltype(list)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char*, std::size_t n) { return static_cast<int>(n); });

    SUBCASE("as rule")
    {
        constexpr auto rule = list;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abc,abc");
        auto two_trace = test_trace(one_trace).literal(",").literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abc,abc,abc");
        auto three_trace = test_trace(two_trace).literal(",").literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("a,abc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal(",")
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);

        auto missing_sep       = LEXY_VERIFY("abcabc");
        auto missing_sep_trace = test_trace().literal("a").position().literal("bc");
        CHECK(missing_sep.status == test_result::success);
        CHECK(missing_sep.value == 1);
        CHECK(missing_sep.trace == missing_sep_trace);

        auto trailing_sep       = LEXY_VERIFY("abc,");
        auto trailing_sep_trace = test_trace()
                                      .literal("a")
                                      .position()
                                      .literal("bc")
                                      .literal(",")
                                      .error(3, 4, "unexpected trailing separator");
        CHECK(trailing_sep.status == test_result::recovered_error);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace == trailing_sep_trace);
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(list);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abc,abc");
        auto two_trace = test_trace(one_trace).literal(",").literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abc,abc,abc");
        auto three_trace = test_trace(two_trace).literal(",").literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("a,abc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal(",")
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);

        auto missing_sep       = LEXY_VERIFY("abcabc");
        auto missing_sep_trace = test_trace().literal("a").position().literal("bc");
        CHECK(missing_sep.status == test_result::success);
        CHECK(missing_sep.value == 1);
        CHECK(missing_sep.trace == missing_sep_trace);

        auto trailing_sep       = LEXY_VERIFY("abc,");
        auto trailing_sep_trace = test_trace()
                                      .literal("a")
                                      .position()
                                      .literal("bc")
                                      .literal(",")
                                      .error(3, 4, "unexpected trailing separator");
        CHECK(trailing_sep.status == test_result::recovered_error);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace == trailing_sep_trace);
    }
}

TEST_CASE("dsl::list(branch, trailing_sep)")
{
    constexpr auto list = dsl::list(LEXY_LIT("a") >> dsl::position + dsl::try_(LEXY_LIT("bc")),
                                    dsl::trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_branch_rule<decltype(list)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char*, std::size_t n) { return static_cast<int>(n); });

    SUBCASE("as rule")
    {
        constexpr auto rule = list;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abc,abc");
        auto two_trace = test_trace(one_trace).literal(",").literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abc,abc,abc");
        auto three_trace = test_trace(two_trace).literal(",").literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("a,abc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal(",")
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);

        auto missing_sep       = LEXY_VERIFY("abcabc");
        auto missing_sep_trace = test_trace().literal("a").position().literal("bc");
        CHECK(missing_sep.status == test_result::success);
        CHECK(missing_sep.value == 1);
        CHECK(missing_sep.trace == missing_sep_trace);

        auto trailing_sep       = LEXY_VERIFY("abc,");
        auto trailing_sep_trace = test_trace().literal("a").position().literal("bc").literal(",");
        CHECK(trailing_sep.status == test_result::success);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace == trailing_sep_trace);
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(list);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abc,abc");
        auto two_trace = test_trace(one_trace).literal(",").literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abc,abc,abc");
        auto three_trace = test_trace(two_trace).literal(",").literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("a,abc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal(",")
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);

        auto missing_sep       = LEXY_VERIFY("abcabc");
        auto missing_sep_trace = test_trace().literal("a").position().literal("bc");
        CHECK(missing_sep.status == test_result::success);
        CHECK(missing_sep.value == 1);
        CHECK(missing_sep.trace == missing_sep_trace);

        auto trailing_sep       = LEXY_VERIFY("abc,");
        auto trailing_sep_trace = test_trace().literal("a").position().literal("bc").literal(",");
        CHECK(trailing_sep.status == test_result::success);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace == trailing_sep_trace);
    }
}

TEST_CASE("dsl::list(branch, trailing_sep)")
{
    constexpr auto list = dsl::list(LEXY_LIT("a") >> dsl::position + dsl::try_(LEXY_LIT("bc")),
                                    dsl::trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_branch_rule<decltype(list)>);

    constexpr auto callback
        = lexy::callback<int>([](const char*) { return 0; },
                              [](const char*, std::size_t n) { return static_cast<int>(n); });

    SUBCASE("as rule")
    {
        constexpr auto rule = list;

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "a", 0).cancel());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abc,abc");
        auto two_trace = test_trace(one_trace).literal(",").literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abc,abc,abc");
        auto three_trace = test_trace(two_trace).literal(",").literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("a,abc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal(",")
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);

        auto missing_sep       = LEXY_VERIFY("abcabc");
        auto missing_sep_trace = test_trace().literal("a").position().literal("bc");
        CHECK(missing_sep.status == test_result::success);
        CHECK(missing_sep.value == 1);
        CHECK(missing_sep.trace == missing_sep_trace);

        auto trailing_sep       = LEXY_VERIFY("abc,");
        auto trailing_sep_trace = test_trace().literal("a").position().literal("bc").literal(",");
        CHECK(trailing_sep.status == test_result::success);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace == trailing_sep_trace);
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(list);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto one       = LEXY_VERIFY("abc");
        auto one_trace = test_trace().literal("a").position().literal("bc");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace == one_trace);
        auto two       = LEXY_VERIFY("abc,abc");
        auto two_trace = test_trace(one_trace).literal(",").literal("a").position().literal("bc");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace == two_trace);
        auto three       = LEXY_VERIFY("abc,abc,abc");
        auto three_trace = test_trace(two_trace).literal(",").literal("a").position().literal("bc");
        CHECK(three.status == test_result::success);
        CHECK(three.value == 3);
        CHECK(three.trace == three_trace);

        auto recover       = LEXY_VERIFY("a,abc");
        auto recover_trace = test_trace()
                                 .literal("a")
                                 .position()
                                 .expected_literal(1, "bc", 0)
                                 .literal(",")
                                 .literal("a")
                                 .position()
                                 .literal("bc");
        CHECK(recover.status == test_result::recovered_error);
        CHECK(recover.value == 2);
        CHECK(recover.trace == recover_trace);

        auto missing_sep       = LEXY_VERIFY("abcabc");
        auto missing_sep_trace = test_trace().literal("a").position().literal("bc");
        CHECK(missing_sep.status == test_result::success);
        CHECK(missing_sep.value == 1);
        CHECK(missing_sep.trace == missing_sep_trace);

        auto trailing_sep       = LEXY_VERIFY("abc,");
        auto trailing_sep_trace = test_trace().literal("a").position().literal("bc").literal(",");
        CHECK(trailing_sep.status == test_result::success);
        CHECK(trailing_sep.value == 1);
        CHECK(trailing_sep.trace == trailing_sep_trace);
    }
}

