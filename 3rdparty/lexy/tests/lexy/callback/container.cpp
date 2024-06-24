// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/container.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/adapter.hpp>
#include <lexy/dsl/option.hpp>
#include <set>
#include <string>
#include <vector>

namespace
{
template <typename T>
struct my_allocator : std::allocator<T>
{
    template <typename U>
    using rebind = my_allocator<U>;

    my_allocator(int i)
    {
        CHECK(i == 42);
    }
    template <typename U>
    my_allocator(my_allocator<U>)
    {}
};
} // namespace

TEST_CASE("as_list")
{
    my_allocator<std::string> alloc(42);

    SUBCASE("callback default")
    {
        constexpr auto callback = lexy::as_list<std::vector<std::string>>;

        CHECK(callback(lexy::nullopt{}) == std::vector<std::string>());
        CHECK(callback(callback(lexy::nullopt{})) == std::vector<std::string>());

        CHECK(callback() == std::vector<std::string>());
        CHECK(callback("a", std::string("b"), "c") == std::vector<std::string>{"a", "b", "c"});
    }
    SUBCASE("callback allocator")
    {
        constexpr auto callback
            = lexy::as_list<std::vector<std::string, my_allocator<std::string>>>;

        CHECK(callback(42).empty());
        CHECK(callback(42, "a", std::string("b"), "c")
              == std::vector<std::string, my_allocator<std::string>>({"a", "b", "c"}, 42));
    }
    SUBCASE("callback state allocator")
    {
        constexpr auto callback
            = lexy::as_list<std::vector<std::string, my_allocator<std::string>>> //
                  .allocator();

        CHECK(callback[alloc]().empty());
        CHECK(callback[alloc](lexy::nullopt{}).empty());
        CHECK(callback[alloc]("a", std::string("b"), "c")
              == std::vector<std::string, my_allocator<std::string>>({"a", "b", "c"}, 42));
    }

    SUBCASE("sink default")
    {
        constexpr auto sink = lexy::as_list<std::vector<std::string>>;
        auto           cb   = sink.sink();
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        std::vector<std::string> result = LEXY_MOV(cb).finish();
        CHECK(result == std::vector<std::string>{"a", "b", "c"});
    }
    SUBCASE("sink allocator")
    {
        constexpr auto sink = lexy::as_list<std::vector<std::string, my_allocator<std::string>>>;
        auto           cb   = sink.sink(42);
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({"a", "b", "c"}, 42));
    }
    SUBCASE("sink state allocator")
    {
        constexpr auto sink = lexy::as_list<std::vector<std::string, my_allocator<std::string>>> //
                                  .allocator();

        auto cb = sink.sink(alloc);
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({"a", "b", "c"}, 42));
    }
}

TEST_CASE("as_collection")
{
    my_allocator<std::string> alloc(42);

    SUBCASE("callback default")
    {
        constexpr auto callback = lexy::as_collection<std::set<std::string>>;

        CHECK(callback(lexy::nullopt{}) == std::set<std::string>());
        CHECK(callback(callback(lexy::nullopt{})) == std::set<std::string>());

        CHECK(callback() == std::set<std::string>());
        CHECK(callback("a", std::string("b"), "c") == std::set<std::string>{"a", "b", "c"});
    }
    SUBCASE("callback allocator")
    {
        using set               = std::set<std::string, std::less<>, my_allocator<std::string>>;
        constexpr auto callback = lexy::as_collection<set>;

        CHECK(callback(42) == set(42));
        CHECK(callback(42, "a", std::string("b"), "c") == set({"a", "b", "c"}, 42));
    }
    SUBCASE("callback state allocator")
    {
        using set               = std::set<std::string, std::less<>, my_allocator<std::string>>;
        constexpr auto callback = lexy::as_collection<set>.allocator();

        CHECK(callback[alloc]().empty());
        CHECK(callback[alloc](lexy::nullopt{}).empty());
        CHECK(callback[alloc]("a", std::string("b"), "c") == set({"a", "b", "c"}, 42));
    }

    SUBCASE("sink default")
    {
        constexpr auto sink = lexy::as_collection<std::set<std::string>>;
        auto           cb   = sink.sink();
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        std::set<std::string> result = LEXY_MOV(cb).finish();
        CHECK(result == std::set<std::string>{"a", "b", "c"});
    }
    SUBCASE("sink allocator")
    {
        constexpr auto sink
            = lexy::as_collection<std::set<std::string, std::less<>, my_allocator<std::string>>>;
        auto cb = sink.sink(42);
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({"a", "b", "c"}, 42));
    }
    SUBCASE("sink state allocator")
    {
        constexpr auto sink
            = lexy::as_collection<std::set<std::string, std::less<>, my_allocator<std::string>>> //
                  .allocator();

        auto cb = sink.sink(alloc);
        cb("a");
        cb(std::string("b"));
        cb(1, 'c');

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({"a", "b", "c"}, 42));
    }
}

TEST_CASE("concat")
{
    SUBCASE("string")
    {
        auto concat = lexy::concat<std::string>;
        CHECK(lexy::is_callback<decltype(concat)>);
        CHECK(lexy::is_sink<decltype(concat)>);

        CHECK(concat(lexy::nullopt{}) == "");
        CHECK(concat("abc") == "abc");
        CHECK(concat("abc", "def", "ghi") == "abcdefghi");

        auto sink = concat.sink();
        sink("");
        sink("");
        sink("abc");
        sink("def");
        CHECK(LEXY_MOV(sink).finish() == "abcdef");
    }
    SUBCASE("vector")
    {
        auto concat = lexy::concat<std::vector<int>>;
        CHECK(lexy::is_callback<decltype(concat)>);
        CHECK(lexy::is_sink<decltype(concat)>);

        CHECK(concat(lexy::nullopt{}).empty());
        CHECK(concat(std::vector{1, 2, 3}) == std::vector{1, 2, 3});
        CHECK(concat(std::vector{1, 2, 3}, std::vector{4, 5, 6}, std::vector{7, 8, 9})
              == std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9});

        auto sink = concat.sink();
        sink(std::vector<int>{});
        sink(std::vector<int>{});
        sink(std::vector{1, 2, 3});
        sink(std::vector{4, 5, 6});
        CHECK(LEXY_MOV(sink).finish() == std::vector{1, 2, 3, 4, 5, 6});
    }
}

TEST_CASE("collect")
{
    SUBCASE("void")
    {
        auto sum      = 0;
        auto callback = lexy::callback([&sum](int i) mutable { sum += i; });

        auto collect = lexy::collect(callback);
        CHECK(lexy::is_sink<decltype(collect)>);

        auto cb = collect.sink();
        cb(1);
        cb(2);
        cb(3);

        std::size_t count = LEXY_MOV(cb).finish();
        CHECK(count == 3);
        CHECK(sum == 6);
    }
    SUBCASE("non-void")
    {
        constexpr auto callback = lexy::callback<int>([](int i) { return 2 * i; });

        constexpr auto collect = lexy::collect<std::vector<int>>(callback);
        CHECK(lexy::is_sink<decltype(collect)>);

        auto cb = collect.sink();
        cb(1);
        cb(2);
        cb(3);

        std::vector<int> result = LEXY_MOV(cb).finish();
        CHECK(result == std::vector<int>{2, 4, 6});
    }
    SUBCASE("non-void with allocator")
    {
        constexpr auto callback = lexy::callback<int>([](int i) { return 2 * i; });

        constexpr auto collect = lexy::collect<std::vector<int, my_allocator<int>>>(callback);

        auto cb = collect.sink(42);
        cb(1);
        cb(2);
        cb(3);

        auto result = LEXY_MOV(cb).finish();
        CHECK(result == decltype(result)({2, 4, 6}, 42));
    }
}

