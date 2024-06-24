// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/bind.hpp>

#include <doctest/doctest.h>
#include <lexy/action/parse.hpp>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/fold.hpp>
#include <lexy/dsl/option.hpp>

TEST_CASE("bind a callback")
{
    constexpr auto cb = lexy::callback<int>(
        [](int i, float f, const char* str) { return i + int(f) + (str[0] - '0'); });

    SUBCASE("constants")
    {
        auto bound = lexy::bind(cb, 42, 3.14f, "123");
        CHECK(bound() == 42 + 3 + 1);
        CHECK(bound(1, 2, 3) == 42 + 3 + 1);
    }
    SUBCASE("nth_value")
    {
        auto bound = lexy::bind(cb, lexy::_3, lexy::_1, lexy::_2);
        CHECK(bound(3.14f, "123", 42) == 42 + 3 + 1);
        CHECK(bound(3.14f, "123", 42, nullptr, 11) == 42 + 3 + 1);
    }
    SUBCASE("nth_value or")
    {
        auto bound = lexy::bind(cb, lexy::_3 or 11, lexy::_1 or 2.71f, lexy::_2 or "987");
        CHECK(bound(3.14f, "123", 42) == 42 + 3 + 1);
        CHECK(bound(3.14f, "123", 42, nullptr, 11) == 42 + 3 + 1);

        CHECK(bound(3.14f, "123") == 11 + 3 + 1);
        CHECK(bound(3.14f) == 11 + 3 + 9);
        CHECK(bound() == 11 + 2 + 9);

        CHECK(bound(3.14f, "123", lexy::nullopt{}) == 11 + 3 + 1);
        CHECK(bound(3.14f, lexy::nullopt{}, lexy::nullopt{}) == 11 + 3 + 9);
        CHECK(bound(lexy::nullopt{}, lexy::nullopt{}, lexy::nullopt{}) == 11 + 2 + 9);

        CHECK(bound(3.14f, lexy::nullopt{}, 42) == 42 + 3 + 9);
        CHECK(bound(lexy::nullopt{}, lexy::nullopt{}, 42) == 42 + 2 + 9);
    }
    SUBCASE("nth_value or_default")
    {
        auto bound = lexy::bind(cb, lexy::_3.or_default(), lexy::_1, lexy::_2);
        CHECK(bound(3.14f, "123", 42) == 42 + 3 + 1);
        CHECK(bound(3.14f, "123", 42, nullptr, 11) == 42 + 3 + 1);

        CHECK(bound(3.14f, "123") == 0 + 3 + 1);
        CHECK(bound(3.14f, "123", lexy::nullopt{}) == 0 + 3 + 1);
    }
    SUBCASE("nth_value map")
    {
        auto bound
            = lexy::bind(cb, lexy::_3, lexy::_1.map([](float f) { return 2 * f; }), lexy::_2);
        CHECK(bound(3.14f, "123", 42) == 42 + 6 + 1);
        CHECK(bound(3.14f, "123", 42, nullptr, 11) == 42 + 6 + 1);
    }
    SUBCASE("nth_value or + map")
    {
        auto bound = lexy::bind(cb, lexy::_3, lexy::_1.map([](float f) { return 2 * f; }) or 2.71f,
                                lexy::_2);
        CHECK(bound(3.14f, "123", 42) == 42 + 6 + 1);
        CHECK(bound(3.14f, "123", 42, nullptr, 11) == 42 + 6 + 1);

        CHECK(bound(lexy::nullopt{}, "123", 42) == 42 + 2 + 1);
    }
    SUBCASE("values")
    {
        auto bound = lexy::bind(cb, lexy::values);
        CHECK(bound(42, 3.14f, "123") == 42 + 3 + 1);
    }
    SUBCASE("parse_state")
    {
        auto bound = lexy::bind(cb, lexy::_1, 3.14f, lexy::parse_state);
        CHECK(bound["123"](42) == 42 + 3 + 1);
    }
    SUBCASE("parse_state map")
    {
        auto bound = lexy::bind(cb, lexy::_1, lexy::parse_state.map([](float f) { return 2 * f; }),
                                lexy::_2);
        CHECK(bound[3.14f](42, "13") == 42 + 6 + 1);
    }

    SUBCASE("mixed")
    {
        auto bound = lexy::bind(cb, lexy::_1, 3.14f, lexy::_2);
        CHECK(bound(42, "123") == 42 + 3 + 1);
        CHECK(bound(42, "123", nullptr, 11) == 42 + 3 + 1);
    }
}

TEST_CASE("bind_sink")
{
    struct my_sink
    {
        auto sink(int i, float f) const
        {
            return lexy::fold_inplace<int>(0, [i, f](int& result,
                                                     int  arg) { result += i * arg + int(f); })
                .sink();
        }
    };

    SUBCASE("bound with state")
    {
        constexpr auto bound = lexy::bind_sink(my_sink{}, lexy::parse_state, 3.14f);

        auto cb = bound.sink(2);
        cb(11);
        cb(42);
        CHECK(LEXY_MOV(cb).finish() == 2 * 11 + 3 + 2 * 42 + 3);
    }

    SUBCASE("bound without state")
    {
        constexpr auto bound = lexy::bind_sink(my_sink{}, 2, 3.14f);

        auto cb = bound.sink();
        cb(11);
        cb(42);
        CHECK(LEXY_MOV(cb).finish() == 2 * 11 + 3 + 2 * 42 + 3);
    }

    SUBCASE("bound passes nullopt to underlying sink")
    {
        constexpr auto const expected = 12345;

        struct sink_handles_nullopt
        {
            struct dummy_impl
            {
                void operator()(int) {}

                auto finish() && -> int
                {
                    return 7;
                }
            };

            constexpr auto operator()(lexy::nullopt&&) const
            {
                return expected;
            }

            constexpr auto sink(int)
            {
                return dummy_impl{};
            }
        };

        constexpr auto bound = lexy::bind_sink(sink_handles_nullopt{}, 15);

        CHECK(bound(lexy::nullopt{}) == expected);
    }
}
