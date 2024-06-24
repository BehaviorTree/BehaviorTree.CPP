// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/nttp_string.hpp>

#include <doctest/doctest.h>
#include <lexy/_detail/string_view.hpp>

TEST_CASE("LEXY_NTTP_STRING")
{
    using string = LEXY_NTTP_STRING(lexy::_detail::type_string, "abc");
    CHECK(std::is_same_v<typename string::char_type, char>);
    CHECK(string::c_str<> == lexy::_detail::string_view("abc"));
    CHECK(string::c_str<wchar_t> == lexy::_detail::basic_string_view<wchar_t>(L"abc"));

    using wstring = LEXY_NTTP_STRING(lexy::_detail::type_string, L"abc");
    CHECK(std::is_same_v<typename wstring::char_type, wchar_t>);
    CHECK(wstring::c_str<> == lexy::_detail::basic_string_view<wchar_t>(L"abc"));
    CHECK(wstring::c_str<char> == lexy::_detail::string_view("abc"));

#if LEXY_HAS_NTTP
    using lit_string = lexy::_detail::to_type_string<lexy::_detail::type_string,
                                                     lexy::_detail::string_literal("abc")>;
    CHECK(lit_string::c_str<> == lexy::_detail::string_view("abc"));
#endif
}

