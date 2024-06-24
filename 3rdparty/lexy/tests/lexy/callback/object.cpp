// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/object.hpp>

#include <doctest/doctest.h>
#include <memory>

TEST_CASE("construct")
{
    SUBCASE("single")
    {
        auto cb = lexy::construct<int>;
        CHECK(cb(0) == 0);
    }
    SUBCASE("parens")
    {
        struct type
        {
            int   a;
            float b;

            type(int a, float b) : a(a), b(b) {}
        };

        auto cb     = lexy::construct<type>;
        auto result = cb(11, 3.14f);
        CHECK(result.a == 11);
        CHECK(result.b == 3.14f);
    }
    SUBCASE("braces")
    {
        struct type
        {
            int   a;
            float b;
        };

        auto cb     = lexy::construct<type>;
        auto result = cb(11, 3.14f);
        CHECK(result.a == 11);
        CHECK(result.b == 3.14f);
    }

    SUBCASE("void")
    {
        auto cb = lexy::construct<void>;
        cb();
    }
}

TEST_CASE("new_")
{
    SUBCASE("single")
    {
        auto cb = lexy::new_<int, std::unique_ptr<int>>;
        CHECK(*cb(0) == 0);
    }
    SUBCASE("parens")
    {
        struct type
        {
            int   a;
            float b;

            type(int a, float b) : a(a), b(b) {}
        };

        auto                  cb     = lexy::new_<type, std::unique_ptr<type>>;
        std::unique_ptr<type> result = cb(11, 3.14f);
        CHECK(result->a == 11);
        CHECK(result->b == 3.14f);
    }
    SUBCASE("braces")
    {
        struct type
        {
            int   a;
            float b;
        };

        auto                  cb     = lexy::new_<type, std::unique_ptr<type>>;
        std::unique_ptr<type> result = cb(11, 3.14f);
        CHECK(result->a == 11);
        CHECK(result->b == 3.14f);
    }
}

