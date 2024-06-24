// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#undef LEXY_DISABLE_FILE
#include <lexy/input/file.hpp>

#include <cstdio>
#include <doctest/doctest.h>

#if defined(__has_include) && __has_include(<memory_resource>)
#    include <memory_resource>
#    define LEXY_HAS_RESOURCE 1
#else
#    define LEXY_HAS_RESOURCE 0
#endif

namespace
{
constexpr auto test_file_name = "lexy-input-file.test.delete-me";

void write_test_data(const char* data)
{
    auto file = std::fopen(test_file_name, "wb");
    std::fputs(data, file);
    std::fclose(file);
}
} // namespace

TEST_CASE("read_file")
{
    std::remove(test_file_name);

    SUBCASE("non-existing file")
    {
        auto result = lexy::read_file(test_file_name);
        CHECK(!result);
        CHECK(result.error() == lexy::file_error::file_not_found);
    }
    SUBCASE("empty file")
    {
        write_test_data("");

        auto result = lexy::read_file(test_file_name);
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("tiny file")
    {
        write_test_data("abc");

        auto result = lexy::read_file(test_file_name);
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("small file")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);
        }

        auto result = lexy::read_file(test_file_name);
        REQUIRE(result);

        auto reader = result.buffer().reader();
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

        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("medium file")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 10 * 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 10 * 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);
        }

        auto result = lexy::read_file(test_file_name);
        REQUIRE(result);

        auto reader = result.buffer().reader();
        for (auto i = 0; i != 10 * 1024; ++i)
        {
            CHECK(reader.peek() == 'a');
            reader.bump();
        }

        for (auto i = 0; i != 10 * 1024; ++i)
        {
            CHECK(reader.peek() == 'b');
            reader.bump();
        }

        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("big file")
    {
        {
            auto file = std::fopen(test_file_name, "wb");
            for (auto i = 0; i != 200 * 1024; ++i)
                std::fputc('a', file);
            for (auto i = 0; i != 200 * 1024; ++i)
                std::fputc('b', file);
            std::fclose(file);
        }

        auto result = lexy::read_file(test_file_name);
        REQUIRE(result);

        auto reader = result.buffer().reader();
        for (auto i = 0; i != 200 * 1024; ++i)
        {
            CHECK(reader.peek() == 'a');
            reader.bump();
        }

        for (auto i = 0; i != 200 * 1024; ++i)
        {
            CHECK(reader.peek() == 'b');
            reader.bump();
        }

        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
#if LEXY_HAS_RESOURCE
    SUBCASE("custom encoding and resource")
    {
        write_test_data("abc");

        auto result = lexy::read_file<lexy::ascii_encoding>(test_file_name,
                                                            std::pmr::new_delete_resource());
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == lexy::ascii_encoding::eof());
    }
#endif
    SUBCASE("custom encoding and byte order")
    {
        const unsigned char data[] = {0xFF, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x00};
        write_test_data(reinterpret_cast<const char*>(data));

        auto result = lexy::read_file<lexy::utf16_encoding>(test_file_name);
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == 0x2211);

        reader.bump();
        CHECK(reader.peek() == 0x4433);

        reader.bump();
        CHECK(reader.peek() == lexy::utf16_encoding::eof());
    }

    std::remove(test_file_name);
}

TEST_CASE("read_stdin")
{
    // Here, we'll reassociate stdin with our test file.
    // This means that we'll permantently loose stdin, but that's okay -- unit tests don't need it.
    auto write_stdin = [](const char* data) {
        write_test_data(data);

        auto result = std::freopen(test_file_name, "rb", stdin);
        REQUIRE(result == stdin);
    };

    std::remove(test_file_name);

    SUBCASE("empty")
    {
        write_stdin("");

        auto result = lexy::read_stdin();
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
    SUBCASE("small")
    {
        write_stdin("abc");

        auto result = lexy::read_stdin();
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == lexy::default_encoding::eof());
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

        auto result = lexy::read_stdin();
        REQUIRE(result);

        auto reader = result.buffer().reader();
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

        CHECK(reader.peek() == lexy::default_encoding::eof());
    }
#if LEXY_HAS_RESOURCE
    SUBCASE("custom encoding and resource")
    {
        write_stdin("abc");

        auto result = lexy::read_stdin<lexy::ascii_encoding>(std::pmr::new_delete_resource());
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == 'a');

        reader.bump();
        CHECK(reader.peek() == 'b');

        reader.bump();
        CHECK(reader.peek() == 'c');

        reader.bump();
        CHECK(reader.peek() == lexy::ascii_encoding::eof());
    }
#endif
    SUBCASE("custom encoding and byte order")
    {
        const unsigned char data[] = {0xFF, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x00};
        write_stdin(reinterpret_cast<const char*>(data));

        auto result = lexy::read_stdin<lexy::utf16_encoding>();
        REQUIRE(result);

        auto reader = result.buffer().reader();
        CHECK(reader.peek() == 0x2211);

        reader.bump();
        CHECK(reader.peek() == 0x4433);

        reader.bump();
        CHECK(reader.peek() == lexy::utf16_encoding::eof());
    }

    std::remove(test_file_name);
}

