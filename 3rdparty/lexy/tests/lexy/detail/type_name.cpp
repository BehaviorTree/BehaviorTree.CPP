// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/type_name.hpp>

#include <doctest/doctest.h>

namespace
{
struct test_type
{
    static constexpr const char* name = "some type";
};
} // namespace

namespace ns
{
struct test_type
{};
class test_class
{};

namespace inner
{
    struct test_type
    {};
} // namespace inner
} // namespace ns

TEST_CASE("_detail::type_name")
{
    SUBCASE("function")
    {
        struct type
        {
            static LEXY_CONSTEVAL const char* name()
            {
                return "some type";
            }
        };
        CHECK(lexy::_detail::type_name<type>() == lexy::_detail::string_view("some type"));
    }
    SUBCASE("variable")
    {
        CHECK(lexy::_detail::type_name<test_type>() == lexy::_detail::string_view("some type"));
    }
#if LEXY_HAS_CONSTEXPR_AUTOMATIC_TYPE_NAME
    SUBCASE("automatic")
    {
        CHECK(lexy::_detail::type_name<int, 0>() == lexy::_detail::string_view("int"));

        CHECK(lexy::_detail::type_name<ns::test_type, 0>()
              == lexy::_detail::string_view("ns::test_type"));
        CHECK(lexy::_detail::type_name<ns::test_class, 0>()
              == lexy::_detail::string_view("ns::test_class"));
        CHECK(lexy::_detail::type_name<ns::inner::test_type, 0>()
              == lexy::_detail::string_view("ns::inner::test_type"));

        CHECK(lexy::_detail::type_name<ns::test_type>() == lexy::_detail::string_view("test_type"));
        CHECK(lexy::_detail::type_name<ns::test_class>()
              == lexy::_detail::string_view("test_class"));
        CHECK(lexy::_detail::type_name<ns::inner::test_type>()
              == lexy::_detail::string_view("inner::test_type"));

        CHECK(lexy::_detail::type_name<ns::test_type, 2>()
              == lexy::_detail::string_view("test_type"));
        CHECK(lexy::_detail::type_name<ns::test_class, 2>()
              == lexy::_detail::string_view("test_class"));
        CHECK(lexy::_detail::type_name<ns::inner::test_type, 2>()
              == lexy::_detail::string_view("test_type"));
    }
#endif
}

