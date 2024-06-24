// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/ip_address.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/action/match.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
void fail(const char* str)
{
    auto input = lexy::zstring_input(str);
    INFO(str);
    REQUIRE(!lexy::match<grammar::ip_address>(input));
}

auto success(const char* str)
{
    auto input  = lexy::zstring_input(str);
    auto result = lexy::parse<grammar::ip_address>(input, lexy::noop);
    INFO(str);
    REQUIRE(result);
    return result.value();
}

auto recovered(const char* str)
{
    auto input  = lexy::zstring_input(str);
    auto result = lexy::parse<grammar::ip_address>(input, lexy::noop);
    INFO(str);
    REQUIRE(result.is_recovered_error());
    REQUIRE(result.has_value());
    return result.value();
}
} // namespace

namespace ip
{
bool operator==(ip_address lhs, ip_address rhs)
{
    if (lhs.version != rhs.version)
        return false;

    for (auto i = 0; i < 8; ++i)
        if (lhs.pieces[i] != rhs.pieces[i])
            return false;

    return true;
}

doctest::String toString(ip_address addr)
{
    doctest::String result;
    for (auto piece : addr.pieces)
    {
        result += doctest::toString(piece);
        result += " ";
    }
    return result;
}
} // namespace ip

TEST_CASE("IPv4")
{
    fail("");
    fail("1.2.3");
    fail("1..2.3.4");

    CHECK(success("0.0.0.0") == ip::ip_address{4, {0, 0}});
    CHECK(success("1.2.3.4") == ip::ip_address{4, {258, 772}});
    CHECK(success("255.255.255.255") == ip::ip_address{4, {0xFFFF, 0xFFFF}});

    CHECK(recovered("1.2.3.4.5") == ip::ip_address{4, {258, 772}});
    CHECK(recovered("0.0.0.256") == ip::ip_address{4, {0, 25}});
}

TEST_CASE("IPv6")
{
    fail("");

    SUBCASE("no elision")
    {
        CHECK(success("0:0:0:0:0:0:0:0") == ip::ip_address{6, {}});
        CHECK(success("1:2:3:4:5:6:7:8") == ip::ip_address{6, {1, 2, 3, 4, 5, 6, 7, 8}});
        CHECK(success("FF:FF:FF:FF:FF:FF:FF:FF")
              == ip::ip_address{6, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}});
        CHECK(
            success("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF")
            == ip::ip_address{6, {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF}});

        CHECK(recovered("1:2") == ip::ip_address{6, {1, 2}});
        CHECK(recovered("1:2x:3:4:5:6:7:8") == ip::ip_address{6, {1, 2}});
        CHECK(recovered("1:2:3:4:5:6:7:8:9") == ip::ip_address{6, {1, 2, 3, 4, 5, 6, 7, 8}});
    }
    SUBCASE("elision")
    {
        CHECK(success("::0") == ip::ip_address{6, {}});
        CHECK(success("::42") == ip::ip_address{6, {0, 0, 0, 0, 0, 0, 0, 0x42}});
        CHECK(success("::FFFF") == ip::ip_address{6, {0, 0, 0, 0, 0, 0, 0, 0xFFFF}});

        CHECK(success("0::") == ip::ip_address{6, {}});
        CHECK(success("42::") == ip::ip_address{6, {0x42}});
        CHECK(success("FFFF::") == ip::ip_address{6, {0xFFFF}});

        CHECK(success("1:2::3:4") == ip::ip_address{6, {1, 2, 0, 0, 0, 0, 3, 4}});

        CHECK(recovered("1::2::3") == ip::ip_address{6, {1, 0, 0, 0, 0, 0, 2, 3}});
    }
    SUBCASE("IPv4")
    {
        CHECK(success("0:0:0:0:0:0:0.0.0.0") == ip::ip_address{6, {}});
        CHECK(success("1:2:3:4:5:6:7.8.9.10") == ip::ip_address{6, {1, 2, 3, 4, 5, 6, 1800, 2314}});
        CHECK(success("1:2:3:4:5:6:255.255.255.255")
              == ip::ip_address{6, {1, 2, 3, 4, 5, 6, 0xFFFF, 0xFFFF}});

        CHECK(success("::1.2.3.4") == ip::ip_address{6, {0, 0, 0, 0, 0, 0, 258, 772}});
        CHECK(success("::255.255.255.255")
              == ip::ip_address{6, {0, 0, 0, 0, 0, 0, 0xFFFF, 0xFFFF}});
        CHECK(success("1:2::3.4.5.6") == ip::ip_address{6, {1, 2, 0, 0, 0, 0, 772, 1286}});
        CHECK(success("1:2::255.255.255.255")
              == ip::ip_address{6, {1, 2, 0, 0, 0, 0, 0xFFFF, 0xFFFF}});

        CHECK(recovered("1:2:3:4:255.255.255.255:255.255.255.255")
              == ip::ip_address{6, {1, 2, 3, 4, 0xFFFF, 0xFFFF, 0, 0}});
        CHECK(recovered("1::255.255.255.255:255.255.255.255")
              == ip::ip_address{6, {1, 0, 0, 0, 0, 0, 0xFFFF, 0xFFFF}});
    }
}

