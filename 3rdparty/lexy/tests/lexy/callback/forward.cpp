// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/forward.hpp>

#include <doctest/doctest.h>
#include <lexy/dsl/option.hpp>

TEST_CASE("forward")
{
    auto cb = lexy::forward<int>;
    CHECK(lexy::is_callback<decltype(cb)>);
    CHECK(cb(0) == 0);

    constexpr auto cb_void = lexy::forward<void>;
    CHECK(lexy::is_callback<decltype(cb_void)>);
    cb_void();
    cb_void(lexy::nullopt{});

    CHECK(lexy::is_sink<decltype(lexy::forward<void>)>);
    auto sink_void = lexy::forward<void>.sink();
    sink_void();
    sink_void(lexy::nullopt{});
    sink_void();
    LEXY_MOV(sink_void).finish();
}

