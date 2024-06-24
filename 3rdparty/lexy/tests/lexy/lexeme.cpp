// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/lexeme.hpp>

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>
#include <string>

TEST_CASE("lexeme")
{
    lexy::string_input input  = lexy::zstring_input("abc");
    auto               reader = input.reader();

    auto begin = reader.position();
    reader.bump();
    reader.bump();
    reader.bump();

    lexy::lexeme lexeme(reader, begin);
    CHECK(lexeme.begin() == begin);
    CHECK(lexeme.end() == reader.position());
    CHECK(lexeme.size() == 3);
    CHECK(lexeme.data() == begin);
    CHECK(lexeme[0] == 'a');
}

