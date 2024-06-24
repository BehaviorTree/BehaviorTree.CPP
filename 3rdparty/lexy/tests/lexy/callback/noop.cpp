// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/noop.hpp>

#include <doctest/doctest.h>

TEST_CASE("noop")
{
    SUBCASE("callback")
    {
        lexy::noop();
        lexy::noop(1, 2, 3);
    }
    SUBCASE("sink")
    {
        auto sink = lexy::noop.sink();
        sink(1, 2, 3);
        sink(1, 2, 3);
        LEXY_MOV(sink).finish();
    }
}

