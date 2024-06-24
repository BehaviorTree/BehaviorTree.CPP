// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/lexeme_input.hpp>

#include <cstring>
#include <doctest/doctest.h>
#include <lexy/input/buffer.hpp>
#include <lexy/input/string_input.hpp>

TEST_CASE("lexeme_input")
{
    const auto str    = "Hello World";
    const auto length = std::strlen(str);

    SUBCASE("view")
    {
        auto input = lexy::lexeme_input(lexy::zstring_input(str), str + 6, str + 11);
        CHECK(input.parent_input().data() == str);
        CHECK(input.parent_input().size() == length);
        CHECK(input.lexeme().begin() == str + 6);
        CHECK(input.lexeme().end() == str + 11);

        auto reader = input.reader();
        CHECK(reader.position() == str + 6);
        CHECK(reader.peek() == 'W');

        reader.bump();
        CHECK(reader.position() == str + 7);
        CHECK(reader.peek() == 'o');
        reader.bump();
        CHECK(reader.position() == str + 8);
        CHECK(reader.peek() == 'r');
        reader.bump();
        CHECK(reader.position() == str + 9);
        CHECK(reader.peek() == 'l');
        reader.bump();
        CHECK(reader.position() == str + 10);
        CHECK(reader.peek() == 'd');

        reader.bump();
        CHECK(reader.position() == str + 11);
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("non view")
    {
        auto buffer = lexy::buffer<lexy::default_encoding>(str, length);

        auto input = lexy::lexeme_input(buffer, buffer.data() + 6, buffer.data() + 11);
        CHECK(&input.parent_input() == &buffer);
        CHECK(input.lexeme().begin() == buffer.data() + 6);
        CHECK(input.lexeme().end() == buffer.data() + 11);

        auto reader = input.reader();
        CHECK(reader.position() == buffer.data() + 6);
        CHECK(reader.peek() == 'W');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 7);
        CHECK(reader.peek() == 'o');
        reader.bump();
        CHECK(reader.position() == buffer.data() + 8);
        CHECK(reader.peek() == 'r');
        reader.bump();
        CHECK(reader.position() == buffer.data() + 9);
        CHECK(reader.peek() == 'l');
        reader.bump();
        CHECK(reader.position() == buffer.data() + 10);
        CHECK(reader.peek() == 'd');

        reader.bump();
        CHECK(reader.position() == buffer.data() + 11);
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
}

