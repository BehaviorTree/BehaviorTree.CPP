// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/email.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/action/match.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
void fail(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    INFO(str);
    REQUIRE(!lexy::match<grammar::message>(input));
}

void pass(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    INFO(str);
    REQUIRE(lexy::match<grammar::message>(input));
}

auto parse_address(const char* str)
{
    auto input  = lexy::zstring_input<lexy::utf8_encoding>(str);
    auto result = lexy::parse<grammar::address>(input, lexy::noop);
    INFO(str);
    REQUIRE(result);
    return result.value();
}
} // namespace

TEST_CASE("address")
{
    auto a = parse_address("test@example.com");
    REQUIRE(!a.display_name);
    REQUIRE(a.local_part == "test");
    REQUIRE(a.domain == "example.com");

    auto b = parse_address("test.foo.bar@example.com");
    REQUIRE(!b.display_name);
    REQUIRE(b.local_part == "test.foo.bar");
    REQUIRE(b.domain == "example.com");

    auto c = parse_address(" test . foo . bar @example.com");
    REQUIRE(!c.display_name);
    REQUIRE(c.local_part == "test.foo.bar");
    REQUIRE(c.domain == "example.com");

    auto d = parse_address(R"("Hello World @ foo bar"@example.com)");
    REQUIRE(!d.display_name);
    REQUIRE(d.local_part == "Hello World @ foo bar");
    REQUIRE(d.domain == "example.com");

    auto e = parse_address("Test <test@example.com>");
    REQUIRE(e.display_name == "Test");
    REQUIRE(e.local_part == "test");
    REQUIRE(e.domain == "example.com");

    auto f = parse_address("Mr Test <test@example.com>");
    REQUIRE(f.display_name == "MrTest");
    REQUIRE(f.local_part == "test");
    REQUIRE(f.domain == "example.com");

    auto g = parse_address(R"("Mr. " Test <test@example.com>)");
    REQUIRE(g.display_name == "Mr. Test");
    REQUIRE(g.local_part == "test");
    REQUIRE(g.domain == "example.com");
}

TEST_CASE("message")
{
    pass(R"(
    )");

    pass(R"(From: test@example.com, test2@example.com, "Also you?" <test3@example.com>

    )");
    pass(R"(To: test@example.com, test2@example.com, "Also you?" <test3@example.com>

    )");
    pass(R"(Cc: test@example.com, test2@example.com, "Also you?" <test3@example.com>

    )");

    pass(R"(Subject: Hello 

    )");
    pass(R"(Subject: Hello World!

    )");
    pass(R"(Subject: Hello Sequence of printable ASCII 1234567890 !@#$%^&*()_ []

    )");

    pass(R"(

    Body of the message.)");
    pass(R"(To: test@example.com

    Body of the message.

    It can really be *anything*!

    1234567890 !@#$%^&*()_

    To: test@example.com

    )");

    fail(R"()");
    fail(R"(From:)");
    fail(R"(Subject: A
    Subject: B

    Body)");
    fail(R"(Subject: A)");
    fail(R"(Subject: A
    )");
    fail(R"(Body of the message.)");
}

