// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/composition.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/bind.hpp>
#include <lexy/callback/fold.hpp>
#include <string>
#include <vector>

TEST_CASE("callback compose")
{
    SUBCASE("callbacks")
    {
        constexpr auto a = lexy::callback<int>([](int i) { return 2 * i; });
        constexpr auto b
            = lexy::callback<std::string>([](int i) { return std::string(std::size_t(i), 'a'); });
        constexpr auto c = lexy::callback<std::size_t>(&std::string::length);

        constexpr auto composed = a | b | c;
        CHECK(composed(0) == 0);
        CHECK(composed(8) == 16);
    }
    SUBCASE("callbacks with state")
    {
        constexpr auto a
            = lexy::bind(lexy::callback<int>([](int factor, int i) { return factor * i; }),
                         lexy::parse_state, lexy::values);
        constexpr auto b
            = lexy::callback<std::string>([](int i) { return std::string(std::size_t(i), 'a'); });
        constexpr auto c = lexy::bind(lexy::callback<std::size_t>([](int factor, const auto& str) {
                                          return std::size_t(factor) * str.size();
                                      }),
                                      lexy::parse_state, lexy::values);

        constexpr auto composed = a | b | c;
        CHECK(composed[2](0) == 2 * 0);
        CHECK(composed[2](8) == 2 * 16);
    }

    SUBCASE("sink and callback")
    {
        constexpr auto sink = lexy::fold_inplace<int>(0, [](int& result, int i) { result += i; });
        constexpr auto cb   = lexy::callback<std::string>([](int i) { return std::to_string(i); });

        constexpr auto composed = sink >> cb;

        auto s = sink.sink();
        s(1);
        s(2);
        s(3);
        auto result = composed(LEXY_MOV(s).finish());
        CHECK(result == "6");
    }
    SUBCASE("sink and two callback")
    {
        constexpr auto sink = lexy::fold_inplace<int>(0, [](int& result, int i) { result += i; });
        constexpr auto cb_a = lexy::callback<std::string>([](int i) { return std::to_string(i); });
        constexpr auto cb_b = lexy::callback<std::size_t>(&std::string::length);

        constexpr auto composed = sink >> cb_a | cb_b;

        auto s = sink.sink();
        s(1);
        s(2);
        s(3);
        auto result = composed(LEXY_MOV(s).finish());
        CHECK(result == 1);
    }
    SUBCASE("sink and two callback with state")
    {
        constexpr auto sink = lexy::fold_inplace<int>(0, [](int& result, int i) { result += i; });
        constexpr auto cb_a = lexy::callback<std::string>([](int i) { return std::to_string(i); });
        constexpr auto cb_b
            = lexy::bind(lexy::callback<std::size_t>([](int factor, const auto& str) {
                             return std::size_t(factor) * str.size();
                         }),
                         lexy::parse_state, lexy::values);

        constexpr auto composed = sink >> cb_a | cb_b;

        auto s = sink.sink();
        s(1);
        s(2);
        s(3);
        auto result = composed[2](LEXY_MOV(s).finish());
        CHECK(result == 2);
    }
}

