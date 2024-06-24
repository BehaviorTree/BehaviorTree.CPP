// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/any_ref.hpp>

#include <doctest/doctest.h>

namespace
{
constexpr int get(lexy::_detail::any_cref any)
{
    return any->get<int>();
}
} // namespace

TEST_CASE("_detail::any_ref")
{
    lexy::_detail::any_holder runtime(42);
    CHECK(get(&runtime) == 42);

    constexpr lexy::_detail::any_holder comptime(42);
    constexpr auto                      comptime_result = get(&comptime);
    CHECK(comptime_result == 42);
}

