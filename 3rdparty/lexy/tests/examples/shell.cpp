// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/shell.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/action/match.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
void fail(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    INFO(str);
    REQUIRE(!lexy::match<grammar::command>(input));
}

void pass(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    INFO(str);
    REQUIRE(lexy::match<grammar::command>(input));
}
} // namespace

TEST_CASE("command")
{
    pass("");
    pass("exit\n");
    pass("exit");

    pass("echo    hi");
    pass(R"(echo \
    hi)");
    pass("echo    hi  ");

    fail("unknown command");
}

TEST_CASE("cmd_exit")
{
    pass("exit");
    fail("exit trailing");
}

TEST_CASE("cmd_echo")
{
    pass("echo hi");

    fail("echo");
    fail("echo hi trailing");
}

TEST_CASE("cmd_set")
{
    pass("set var value");

    fail("set");
    fail("set var");
    fail("set var value trailing");
}

TEST_CASE("arg_string")
{
    pass("echo 'string'");
    pass("echo 'string \"\\n'");
    fail("echo 'unterminated");
}

TEST_CASE("arg_string")
{
    pass(R"(echo "string")");
    pass(R"(echo "string \\\"\n\r")");
    pass(R"(echo "${var}")");

    fail(R"(echo "unterminated)");
    fail(R"(echo "\x")");
}

TEST_CASE("arg_var")
{
    pass("echo $var");
    pass("echo ${'var'}");

    fail("echo $'var'");
    fail("echo ${'var'");
}

