// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/lazy_init.hpp>

#include <doctest/doctest.h>
#include <string>

namespace
{
constexpr auto my_int = 42;
}

TEST_CASE("_detail::lazy_init")
{
    SUBCASE("trivial")
    {
        using lazy_init = lexy::_detail::lazy_init<int>;

        constexpr auto empty = lazy_init();
        CHECK(!empty);

        constexpr auto emplaced = [] {
            lazy_init result;
            result.emplace(42);
            return result;
        }();
        CHECK(emplaced);
        CHECK(*emplaced == 42);

        constexpr auto emplaced_result = [] {
            lazy_init result;
            result.emplace_result([](int i) { return 2 * i; }, 21);
            return result;
        }();
        CHECK(emplaced_result);
        CHECK(*emplaced_result == 42);
    }
    SUBCASE("non-trivial")
    {
        using lazy_init = lexy::_detail::lazy_init<std::string>;

        auto empty = lazy_init();
        CHECK(!empty);

        auto emplaced = [] {
            lazy_init result;
            result.emplace(5u, 'a');
            return result;
        }();
        CHECK(emplaced);
        CHECK(*emplaced == "aaaaa");
        CHECK(emplaced->size() == 5);

        auto emplaced_result = [] {
            lazy_init result;
            result.emplace_result([](char c) { return std::string(5, c); }, 'a');
            return result;
        }();
        CHECK(emplaced_result);
        CHECK(*emplaced_result == "aaaaa");

        SUBCASE("move constructor from empty")
        {
            auto copy = LEXY_MOV(empty);
            CHECK(!copy);
        }
        SUBCASE("move constructor from emplaced")
        {
            auto copy = LEXY_MOV(emplaced);
            CHECK(copy);
            CHECK(*copy == "aaaaa");
            CHECK(copy->size() == 5);
        }

        SUBCASE("move assignment from empty to empty")
        {
            lazy_init assigned;

            assigned = LEXY_MOV(empty);
            CHECK(!assigned);
        }
        SUBCASE("move assignment from emplaced to empty")
        {
            lazy_init assigned;

            assigned = LEXY_MOV(emplaced);
            CHECK(assigned);
            CHECK(*assigned == "aaaaa");
            CHECK(assigned->size() == 5);
        }
        SUBCASE("move assignment from empty to emplaced")
        {
            lazy_init assigned;
            assigned.emplace(3u, 'b');

            assigned = LEXY_MOV(empty);
            CHECK(!assigned);
        }
        SUBCASE("move assignment from emplaced to emplaced")
        {
            lazy_init assigned;
            assigned.emplace(3u, 'b');

            assigned = LEXY_MOV(emplaced);
            CHECK(assigned);
            CHECK(*assigned == "aaaaa");
            CHECK(assigned->size() == 5);
        }
    }
    SUBCASE("reference")
    {
        using lazy_init = lexy::_detail::lazy_init<const int&>;

        constexpr auto empty = lazy_init();
        CHECK(!empty);

        constexpr auto emplaced = [] {
            lazy_init result;
            result.emplace(my_int);
            return result;
        }();
        CHECK(emplaced);
        CHECK(&*emplaced == &my_int);
        CHECK(emplaced.operator->() == &my_int);

        constexpr auto emplaced_result = [] {
            lazy_init result;
            result.emplace_result([](int) -> const int& { return my_int; }, 21);
            return result;
        }();
        CHECK(emplaced_result);
        CHECK(*emplaced_result == 42);
        CHECK(&*emplaced_result == &my_int);
        CHECK(emplaced_result.operator->() == &my_int);
    }
    SUBCASE("void")
    {
        using lazy_init = lexy::_detail::lazy_init<void>;

        constexpr auto empty = lazy_init();
        CHECK(!empty);

        constexpr auto emplaced = [] {
            lazy_init result;
            result.emplace();
            return result;
        }();
        CHECK(emplaced);
    }
}

