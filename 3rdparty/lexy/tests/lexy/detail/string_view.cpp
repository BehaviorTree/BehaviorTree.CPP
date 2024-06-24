// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/string_view.hpp>

#include <doctest/doctest.h>

TEST_CASE("string_view")
{
    lexy::_detail::string_view str = "abc";
    REQUIRE(str.is_null_terminated());
    REQUIRE(str.size() == 3);
    REQUIRE(str == "abc");

    REQUIRE(str.substr(1) == "bc");
    REQUIRE(str.substr(1).is_null_terminated());
    REQUIRE(str.substr(1, 1) == "b");
    REQUIRE(!str.substr(1, 1).is_null_terminated());
    REQUIRE(str.substr(1, 24) == "bc");
    REQUIRE(str.substr(1, 24).is_null_terminated());

    REQUIRE(str.find("bc") == 1);
    REQUIRE(str.find("a", 1) == std::size_t(-1));
    REQUIRE(str.find('b') == 1);
    REQUIRE(str.find('a', 1) == std::size_t(-1));

    REQUIRE(str.starts_with("ab"));
    REQUIRE(!str.starts_with("abcdef"));
}

namespace
{
constexpr auto fn()
{
    lexy::_detail::string_view str = "abc";
    return str.substr(0, 2);
}
} // namespace

TEST_CASE("make_cstr")
{
    constexpr auto str = lexy::_detail::make_cstr<+fn>;
    REQUIRE(str == lexy::_detail::string_view("ab"));
}

