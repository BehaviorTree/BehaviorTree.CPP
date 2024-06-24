// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/grammar.hpp>

#include <vector>

#include <doctest/doctest.h>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/bind.hpp>
#include <lexy/callback/container.hpp>
#include <lexy/callback/fold.hpp>
#include <lexy/callback/noop.hpp>
#include <lexy/callback/object.hpp>
#include <lexy/dsl/any.hpp>

namespace
{
struct prod
{
    static constexpr auto name = "prod";
    static constexpr auto rule = lexy::dsl::any;
};

struct prod_ws
{
    static constexpr auto whitespace = lexy::dsl::any;
};

struct prod_token : lexy::token_production
{};

struct prod_depth
{
    static constexpr auto max_recursion_depth = 32;
};
} // namespace

TEST_CASE("production traits simple")
{
    CHECK(lexy::production_name<prod>() == lexy::_detail::string_view("prod"));

    CHECK(std::is_same_v<const lexy::production_rule<prod>, decltype(lexy::dsl::any)>);
}

TEST_CASE("production_whitespace")
{
    CHECK(std::is_same_v<const lexy::production_whitespace<prod_token, prod_ws>,
                         decltype(lexy::dsl::any)>);
    CHECK(std::is_same_v<const lexy::production_whitespace<prod, prod_ws>, //
                         decltype(lexy::dsl::any)>);
    CHECK(std::is_same_v<const lexy::production_whitespace<prod_ws, prod>, //
                         decltype(lexy::dsl::any)>);
    CHECK(std::is_same_v<const lexy::production_whitespace<prod, prod>, const void>);
}

TEST_CASE("max_recursion_depth()")
{
    CHECK(lexy::max_recursion_depth<prod>() == 1024);
    CHECK(lexy::max_recursion_depth<prod_depth>() == 32);
}

namespace
{
template <const auto& Value>
struct prod_value
{
    static constexpr auto value = Value;
};

constexpr auto bind_construct    = lexy::bind(lexy::construct<int>, lexy::parse_state);
constexpr auto as_allocator_list = lexy::as_list<std::vector<int>>.allocator();
} // namespace

TEST_CASE("production_value_callback")
{
    SUBCASE("callback only, no state")
    {
        lexy::production_value_callback<prod_value<lexy::construct<int>>> cb;
        CHECK(std::is_same_v<decltype(cb)::return_type, int>);
        CHECK(cb() == 0);
        CHECK(cb(42) == 42);
    }
    SUBCASE("sink only, no state")
    {
        lexy::production_value_callback<prod_value<lexy::count>> cb;
        CHECK(std::is_same_v<decltype(cb)::return_type, std::size_t>);

        auto sink = cb.sink();
        sink(0);
        sink(1);
        sink(2);
        CHECK(LEXY_MOV(sink).finish() == 3);

        CHECK(cb(std::size_t(3)) == 3);
    }
    SUBCASE("callback and sink, no state")
    {
        lexy::production_value_callback<prod_value<lexy::as_list<std::vector<int>>>> cb;
        CHECK(std::is_same_v<decltype(cb)::return_type, std::vector<int>>);

        auto sink = cb.sink();
        sink(0);
        sink(1);
        sink(2);
        CHECK(LEXY_MOV(sink).finish() == std::vector<int>{0, 1, 2});

        CHECK(cb(std::vector<int>{0, 1, 2}) == std::vector<int>{0, 1, 2});
        CHECK(cb(0, 1, 2) == std::vector<int>{0, 1, 2});
    }

    SUBCASE("callback with state")
    {
        auto state = 42;

        lexy::production_value_callback<prod_value<bind_construct>, int> cb(state);
        CHECK(std::is_same_v<decltype(cb)::return_type, int>);
        CHECK(cb() == 42);
    }
    SUBCASE("sink with state")
    {
        auto state = std::allocator<int>();

        lexy::production_value_callback<prod_value<as_allocator_list>, std::allocator<int>> cb(
            state);
        CHECK(std::is_same_v<decltype(cb)::return_type, std::vector<int>>);

        auto sink = cb.sink();
        sink(0);
        sink(1);
        sink(2);
        CHECK(LEXY_MOV(sink).finish() == std::vector<int>{0, 1, 2});
    }

    SUBCASE("parse state overrides callback")
    {
        struct state_t
        {
            auto value_of(prod_value<lexy::construct<void>>) const
            {
                return lexy::construct<int>;
            }
        } state;

        lexy::production_value_callback<prod_value<lexy::construct<void>>, state_t> cb(state);

        CHECK(std::is_same_v<decltype(cb)::return_type, int>);
        CHECK(cb() == 0);
        CHECK(cb(42) == 42);
    }
    SUBCASE("parse state overrides callback that access state")
    {
        struct state_t
        {
            int result;

            auto value_of(prod_value<lexy::construct<void>>) const
            {
                return lexy::callback<int>([this] { return result; });
            }
        } state{42};

        lexy::production_value_callback<prod_value<lexy::construct<void>>, state_t> cb(state);

        CHECK(std::is_same_v<decltype(cb)::return_type, int>);
        CHECK(cb() == 42);
    }
    SUBCASE("parse state overrides sink")
    {
        struct state_t
        {
            auto value_of(prod_value<lexy::count>) const
            {
                return lexy::as_list<std::vector<int>>;
            }
        } state;

        lexy::production_value_callback<prod_value<lexy::count>, state_t> cb(state);
        CHECK(std::is_same_v<decltype(cb)::return_type, std::vector<int>>);

        auto sink = cb.sink();
        sink(0);
        sink(1);
        sink(2);
        CHECK(LEXY_MOV(sink).finish() == std::vector<int>{0, 1, 2});
    }

    SUBCASE("lexy::noop") // special case of returning void
    {
        lexy::production_value_callback<prod_value<lexy::noop>> cb;
        CHECK(std::is_same_v<decltype(cb)::return_type, void>);

        auto sink = cb.sink();
        sink(0);
        sink(1);
        sink(2);
        LEXY_MOV(sink).finish();

        cb();
        cb(1, 2, 3);
    }
}

