// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/invoke.hpp>

#include <doctest/doctest.h>

TEST_CASE("_detail::invoke")
{
    const auto function
        = lexy::_detail::invoke([](int a, char b, float c) { return float(a) + float(b) + c; }, 1,
                                '\x02', 3.f);
    CHECK(function == 6.);

    struct test_class
    {
        int data = 42;

        int memfn(int arg) const
        {
            return arg + data;
        }
    };
    test_class  object;
    test_class* ptr = &object;

    const auto memfn_object = lexy::_detail::invoke(&test_class::memfn, object, 2);
    CHECK(memfn_object == 44);
    const auto memfn_ptr = lexy::_detail::invoke(&test_class::memfn, ptr, 2);
    CHECK(memfn_ptr == 44);

    const auto mem_object = lexy::_detail::invoke(&test_class::data, object);
    CHECK(mem_object == 42);
    const auto mem_ptr = lexy::_detail::invoke(&test_class::data, ptr);
    CHECK(mem_ptr == 42);
}

