// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/argv_input.hpp>

#include <doctest/doctest.h>

TEST_CASE("argv_iterator")
{
    char program[] = "IGNORED";
    char first[]   = "abc";
    char second[]  = "de";
    char third[]   = "f";

    char* argv[] = {program, first, second, third, nullptr};
    int   argc   = 4;

    SUBCASE("iterate forwards and back")
    {
        auto end  = lexy::argv_end(argc, argv);
        auto iter = lexy::argv_begin(argc, argv);

        REQUIRE(iter != end);
        REQUIRE(*iter == 'a');

        ++iter;
        REQUIRE(iter != end);
        REQUIRE(*iter == 'b');

        ++iter;
        REQUIRE(iter != end);
        REQUIRE(*iter == 'c');

        ++iter;
        REQUIRE(iter != end);
        REQUIRE(*iter == '\0');

        ++iter;
        REQUIRE(iter != end);
        REQUIRE(*iter == 'd');

        ++iter;
        REQUIRE(iter != end);
        REQUIRE(*iter == 'e');

        ++iter;
        REQUIRE(iter != end);
        REQUIRE(*iter == '\0');

        ++iter;
        REQUIRE(iter != end);
        REQUIRE(*iter == 'f');

        ++iter;
        REQUIRE(iter == end);

        auto begin = lexy::argv_begin(argc, argv);
        REQUIRE(iter != begin);

        --iter;
        REQUIRE(iter != begin);
        REQUIRE(*iter == 'f');

        --iter;
        REQUIRE(iter != begin);
        REQUIRE(*iter == '\0');

        --iter;
        REQUIRE(iter != begin);
        REQUIRE(*iter == 'e');

        --iter;
        REQUIRE(iter != begin);
        REQUIRE(*iter == 'd');

        --iter;
        REQUIRE(iter != begin);
        REQUIRE(*iter == '\0');

        --iter;
        REQUIRE(iter != begin);
        REQUIRE(*iter == 'c');

        --iter;
        REQUIRE(iter != begin);
        REQUIRE(*iter == 'b');

        --iter;
        REQUIRE(iter == begin);
        REQUIRE(*iter == 'a');
    }
    SUBCASE("post-increment")
    {
        auto begin = lexy::argv_begin(argc, argv);

        auto iter = begin;
        REQUIRE(iter++ == begin);
        REQUIRE(*iter == 'b');
    }
    SUBCASE("post-decrement")
    {
        auto end = lexy::argv_end(argc, argv);

        auto iter = end;
        REQUIRE(iter-- == end);
        REQUIRE(*iter == 'f');
    }
    SUBCASE("pointer")
    {
        auto iter = lexy::argv_begin(argc, argv);
        REQUIRE(&*iter == &argv[1][0]);
        REQUIRE(iter.operator->() == &argv[1][0]);
    }
    SUBCASE("comparison sentinel")
    {
        REQUIRE(lexy::argv_begin(argc, argv) != lexy::argv_sentinel());
        REQUIRE(lexy::argv_end(argc, argv) == lexy::argv_sentinel());
    }
    SUBCASE("empty argv")
    {
        int   argc   = 0;
        char* argv[] = {nullptr};

        auto begin = lexy::argv_begin(argc, argv);
        auto end   = lexy::argv_end(argc, argv);
        REQUIRE(begin == end);
        REQUIRE(begin == lexy::argv_sentinel{});
        REQUIRE(end == lexy::argv_sentinel{});
    }
    SUBCASE("single argv")
    {
        int   argc   = 1;
        char* argv[] = {program, nullptr};

        auto begin = lexy::argv_begin(argc, argv);
        auto end   = lexy::argv_end(argc, argv);
        REQUIRE(begin == end);
        REQUIRE(begin == lexy::argv_sentinel{});
        REQUIRE(end == lexy::argv_sentinel{});
    }
}

TEST_CASE("argv_input")
{
    char program[] = "IGNORED";
    char first[]   = "abc";
    char second[]  = "de";
    char third[]   = "f";

    char* argv[] = {program, first, second, third, nullptr};
    int   argc   = 4;

    lexy::argv_input input(argc, argv);
    auto             reader = input.reader();

    CHECK(reader.peek() == 'a');
    CHECK(reader.position() == lexy::argv_begin(argc, argv));

    reader.bump();
    CHECK(reader.peek() == 'b');
    reader.bump();
    CHECK(reader.peek() == 'c');
    reader.bump();
    CHECK(reader.peek() == '\0');
    reader.bump();
    CHECK(reader.peek() == 'd');
    reader.bump();
    CHECK(reader.peek() == 'e');
    reader.bump();
    CHECK(reader.peek() == '\0');
    reader.bump();
    CHECK(reader.peek() == 'f');

    reader.bump();
    CHECK(reader.peek() == lexy::default_encoding::eof());
    CHECK(reader.position() == lexy::argv_end(argc, argv));
}

