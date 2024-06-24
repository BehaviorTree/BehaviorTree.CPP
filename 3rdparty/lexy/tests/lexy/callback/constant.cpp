// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/constant.hpp>

#include <doctest/doctest.h>

TEST_CASE("constant")
{
    auto cb = lexy::constant(42);
    CHECK(cb() == 42);
}

