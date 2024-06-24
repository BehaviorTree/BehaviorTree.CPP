// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/callback/adapter.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/constant.hpp>
#include <lexy/callback/fold.hpp>

namespace
{
int test_fn(std::nullptr_t)
{
    return 0;
}
} // namespace

TEST_CASE("callback")
{
    SUBCASE("basic")
    {
        constexpr auto callback
            = lexy::callback<int>([](int i) { return 2 * i; }, [](const char* ptr) { return *ptr; },
                                  &test_fn);

        CHECK(lexy::is_callback<decltype(callback)>);
        CHECK(std::is_same_v<typename decltype(callback)::return_type, int>);

        CHECK(lexy::is_callback_for<decltype(callback), int>);
        CHECK(callback(11) == 22);

        CHECK(lexy::is_callback_for<decltype(callback), const char*>);
        CHECK(callback("abc") == 'a');

        CHECK(lexy::is_callback_for<decltype(callback), std::nullptr_t>);
        CHECK(callback(nullptr) == 0);
    }
    SUBCASE("match all case")
    {
        constexpr auto callback
            = lexy::callback<int>([](const auto&... args) { return sizeof...(args); });
        CHECK(callback() == 0);
        CHECK(callback(1) == 1);
        CHECK(callback(1, 2, 3) == 3);
    }
    SUBCASE("member ptr")
    {
        struct foo
        {
            int member;

            int fn(int i) const
            {
                return i;
            }
        };
        foo obj{42};

        constexpr auto callback = lexy::callback<int>(&foo::fn, &foo::member);
        CHECK(callback(foo(), 4) == 4);
        CHECK(callback(&obj) == 42);
    }
    SUBCASE("with state")
    {
        constexpr auto callback = lexy::callback<int>([i = 42](int arg) { return arg + i; });
        CHECK(callback(0) == 42);
        CHECK(callback(11) == 53);
    }
    SUBCASE("from other callbacks")
    {
        constexpr auto callback
            = lexy::callback(lexy::callback<int>([](int arg) { return 2 * arg; }),
                             lexy::callback<float>([](float f) { return f + 1.5; }));
        CHECK(std::is_same_v<decltype(callback)::return_type, float>);
        CHECK(callback(4) == 8);
        CHECK(callback(2.f) == 3.5);

        struct no_default
        {
            constexpr no_default(int) {}
        };

        constexpr auto callback_without_default_ctor
            = lexy::callback(lexy::constant(no_default(42)));
        CHECK(std::is_same_v<decltype(callback_without_default_ctor)::return_type, no_default>);
    }
}

TEST_CASE("callback from sink")
{
    constexpr auto sink     = lexy::fold_inplace<int>(0, [](int& result, int i) { result += i; });
    constexpr auto callback = lexy::callback(sink);

    CHECK(callback() == 0);
    CHECK(callback(1, 2, 3) == 6);
}

TEST_CASE("mem_fn")
{
    struct foo
    {
        int member;

        int do_sth(int i) volatile& noexcept
        {
            return member + i;
        }
    };

    SUBCASE("member function")
    {
        constexpr auto callback = lexy::mem_fn(&foo::do_sth);
        CHECK(std::is_same_v<decltype(callback)::return_type, int>);

        foo f{42};
        CHECK(callback(f, 11) == 53);
    }
    SUBCASE("member data")
    {
        constexpr auto callback = lexy::mem_fn(&foo::member);
        CHECK(std::is_same_v<decltype(callback)::return_type, int>);

        CHECK(callback(foo{42}) == 42);
    }

    SUBCASE("nested member data")
    {
        struct weird
        {
            int foo::*ptr;
        };

        constexpr auto callback = lexy::mem_fn(&weird::ptr);
        CHECK(std::is_same_v<decltype(callback)::return_type, int foo::*>);

        CHECK(callback(weird{&foo::member}) == &foo::member);
    }
}

