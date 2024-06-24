// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/return.hpp>

#include "verify.hpp"

TEST_CASE("dsl::return_")
{
    constexpr auto rule = dsl::return_ + LEXY_LIT("abc");
    CHECK(lexy::is_rule<decltype(dsl::return_)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.trace == test_trace());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::success);
    CHECK(abc.trace == test_trace());
}

