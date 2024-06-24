// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/string_input.hpp>

#include <doctest/doctest.h>

TEST_CASE("string_input")
{
    static const char str[] = {'a', 'b', 'c', '\0'};
    struct view
    {
        auto data() const
        {
            return str;
        }

        std::size_t size() const
        {
            return 3;
        }
    };

    SUBCASE("basic")
    {
        lexy::string_input<> input;
        CHECK(input.data() == nullptr);
        CHECK(input.size() == 0u);
        CHECK(input.reader().position() == nullptr);
        CHECK(input.reader().peek() == lexy::default_encoding::eof());

        SUBCASE("range ctor")
        {
            input = lexy::string_input(str, str + 3);
        }
        SUBCASE("ptr + size ctor")
        {
            input = lexy::string_input(str, 3);
        }
        SUBCASE("view")
        {
            input = lexy::string_input(view());
        }
        SUBCASE("zstring")
        {
            input = lexy::zstring_input(str);
        }
        CHECK(input.data() == str);
        CHECK(input.size() == 3);

        auto reader = input.reader();
        CHECK(reader.position() == str);
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.position() == str + 1);
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.position() == str + 2);
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.position() == str + 3);
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("converting ctor")
    {
        lexy::string_input<lexy::byte_encoding> input;

        SUBCASE("range ctor")
        {
            input = lexy::string_input<lexy::byte_encoding>(str, str + 3);
        }
        SUBCASE("ptr + size ctor")
        {
            input = lexy::string_input<lexy::byte_encoding>(str, 3);
        }
        SUBCASE("view")
        {
            input = lexy::string_input<lexy::byte_encoding>(view());
        }
        SUBCASE("zstring")
        {
            input = lexy::zstring_input<lexy::byte_encoding>(str);
        }

        CHECK(input.data() == reinterpret_cast<const unsigned char*>(str));
        CHECK(input.size() == 3);
    }
}

