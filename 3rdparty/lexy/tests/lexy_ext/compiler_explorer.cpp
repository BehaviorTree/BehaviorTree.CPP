// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy_ext/compiler_explorer.hpp>

#include <doctest/doctest.h>

TEST_CASE("compiler_explorer_input()")
{
    constexpr auto test_file_name = "lexy_ext-compiler_explorer.test.delete-me";

    // Here, we'll reassociate stdin with our test file.
    // This means that we'll permantently loose stdin, but that's okay -- unit tests don't need it.
    auto write_stdin = [](const char* data) {
        auto file = std::fopen(test_file_name, "wb");
        std::fputs(data, file);
        std::fclose(file);

        auto result = std::freopen(test_file_name, "rb", stdin);
        REQUIRE(result == stdin);
    };

    std::remove(test_file_name);

    SUBCASE("empty")
    {
        write_stdin("");

        auto input  = lexy_ext::compiler_explorer_input();
        auto reader = input.reader();
        CHECK(reader.peek() == lexy::utf8_encoding::eof());
    }
    SUBCASE("small")
    {
        write_stdin("abc");

        auto input  = lexy_ext::compiler_explorer_input();
        auto reader = input.reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == lexy::utf8_encoding::eof());
    }
    SUBCASE("big")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);

            auto result = std::freopen(test_file_name, "rb", stdin);
            REQUIRE(result == stdin);
        }

        auto input  = lexy_ext::compiler_explorer_input();
        auto reader = input.reader();

        for (auto i = 0; i != 1024; ++i)
        {
            CHECK(reader.peek() == 'a');
            reader.bump();
        }

        for (auto i = 0; i != 1024; ++i)
        {
            CHECK(reader.peek() == 'b');
            reader.bump();
        }

        CHECK(reader.peek() == lexy::utf8_encoding::eof());
    }

    std::remove(test_file_name);
}

