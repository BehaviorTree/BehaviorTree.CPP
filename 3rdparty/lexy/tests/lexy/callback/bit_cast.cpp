// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/bit_cast.hpp>

#include <doctest/doctest.h>

TEST_CASE("bit_cast")
{
    constexpr auto callback = lexy::bit_cast<unsigned>;
    CHECK(lexy::is_callback<decltype(callback)>);
    CHECK(std::is_same_v<decltype(callback)::return_type, unsigned>);

    CHECK(callback(0u) == 0u);
    CHECK(callback(42u) == 42u);

    CHECK(callback(0) == 0);
    CHECK(callback(42) == 42);
}

