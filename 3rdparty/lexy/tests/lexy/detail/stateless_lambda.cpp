// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/stateless_lambda.hpp>

#include <doctest/doctest.h>

TEST_CASE("_detail::stateless_lambda")
{
    auto                                              lambda = [] { return 42; };
    lexy::_detail::stateless_lambda<decltype(lambda)> holder;
    CHECK(holder() == 42);
}

