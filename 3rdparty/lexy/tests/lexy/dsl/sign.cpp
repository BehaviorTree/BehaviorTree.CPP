// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/sign.hpp>

#include "verify.hpp"

TEST_CASE("dsl::plus_sign")
{
    constexpr auto rule = dsl::plus_sign;
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                  [](const char*, lexy::plus_sign s) {
                                                      CHECK(s == +1);
                                                      return 1;
                                                  });

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace());

    auto plus = LEXY_VERIFY("+");
    CHECK(plus.status == test_result::success);
    CHECK(plus.value == 1);
    CHECK(plus.trace == test_trace().literal("+"));
    auto minus = LEXY_VERIFY("-");
    CHECK(minus.status == test_result::success);
    CHECK(minus.value == 0);
    CHECK(minus.trace == test_trace());
}

TEST_CASE("dsl::minus_sign")
{
    constexpr auto rule = dsl::minus_sign;
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                  [](const char*, lexy::minus_sign s) {
                                                      CHECK(s == -1);
                                                      return 1;
                                                  });

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace());

    auto plus = LEXY_VERIFY("+");
    CHECK(plus.status == test_result::success);
    CHECK(plus.value == 0);
    CHECK(plus.trace == test_trace());
    auto minus = LEXY_VERIFY("-");
    CHECK(minus.status == test_result::success);
    CHECK(minus.value == 1);
    CHECK(minus.trace == test_trace().literal("-"));
}

TEST_CASE("dsl::sign")
{
    constexpr auto rule = dsl::sign;
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                  [](const char*, lexy::plus_sign s) {
                                                      CHECK(s == +1);
                                                      return 1;
                                                  },
                                                  [](const char*, lexy::minus_sign s) {
                                                      CHECK(s == -1);
                                                      return 2;
                                                  });

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == 0);
    CHECK(empty.trace == test_trace());

    auto plus = LEXY_VERIFY("+");
    CHECK(plus.status == test_result::success);
    CHECK(plus.value == 1);
    CHECK(plus.trace == test_trace().literal("+"));
    auto minus = LEXY_VERIFY("-");
    CHECK(minus.status == test_result::success);
    CHECK(minus.value == 2);
    CHECK(minus.trace == test_trace().literal("-"));
}

