// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/turing.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

namespace
{
int parse(const char* str)
{
    auto input  = lexy::zstring_input(str);
    auto result = lexy::parse<grammar::program<'a', 'b', 'c', 'i', 'o', 'x'>>(input, lexy::noop);
    INFO(str);
    REQUIRE(result);
    return result.value();
}
int parse(const doctest::String& str)
{
    return parse(str.c_str());
}
} // namespace

TEST_CASE("program")
{
    CHECK(parse("") == 0);

    CHECK(parse("o :=; ") == 0);
    CHECK(parse("o := |; ") == 1);
    CHECK(parse("o := |||; ") == 3);

    CHECK(parse("o += |; ") == 1);
    CHECK(parse("o += |||; ") == 3);

    CHECK(parse("o := |||||; o -= |; ") == 4);
    CHECK(parse("o := |||||; o -= |||; ") == 2);

    CHECK(parse("if a { o := |; } else { o := ||; }") == 2);
    CHECK(parse("a := |; if a { o := |; } else { o := ||; }") == 1);

    CHECK(parse("if a { o := |; }") == 0);
    CHECK(parse("a := |; if a { o := |; }") == 1);

    CHECK(parse("a := ||||; while a { a -= |; o += ||; }") == 8);
}

namespace
{
doctest::String fib(int i)
{
    doctest::String result("i := ");
    while (i-- > 0)
        result += "|";
    result += ";\n";

    return result + R"(
    a := ;
    b := |;

    while i {
        i -= |;

        // c := a;
        c := ;
        while a { a -= |; c += |; }

        // c += b;
        // x := b;
        x := ;
        while b { b -= |; c += |; x += |; }

        // a := x; (which is the original b)
        a := ;
        while x { x -= |; a += |; }

        // b := c; (which is the original a + b)
        b := ;
        while c { c -= |; b += |; }
    }

    // o := a;
    while a { a -= |; o += |; }
)";
}
} // namespace

TEST_CASE("fib")
{
    CHECK(parse(fib(0)) == 0);
    CHECK(parse(fib(1)) == 1);
    CHECK(parse(fib(2)) == 1);
    CHECK(parse(fib(3)) == 2);
    CHECK(parse(fib(4)) == 3);
    CHECK(parse(fib(5)) == 5);
    CHECK(parse(fib(6)) == 8);
    CHECK(parse(fib(7)) == 13);
    CHECK(parse(fib(8)) == 21);
}

