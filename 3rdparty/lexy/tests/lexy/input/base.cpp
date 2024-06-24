// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/base.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

TEST_CASE("partial_input()")
{
    auto input = lexy::zstring_input("abc");
    auto end   = input.data() + 2;

    auto partial_input = lexy::partial_input(input.reader(), end);
    auto partial       = partial_input.reader();
    CHECK(partial.position() == input.data());
    CHECK(partial.peek() == 'a');

    partial.bump();
    CHECK(partial.peek() == 'b');

    partial.bump();
    CHECK(partial.peek() == lexy::default_encoding::eof());
}

