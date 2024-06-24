// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/xml.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/action/match.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
void fail(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    INFO(str);
    REQUIRE(!lexy::match<grammar::document>(input));
}

void pass(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    INFO(str);
    REQUIRE(lexy::match<grammar::document>(input));
}
} // namespace

TEST_CASE("document")
{
    fail(R"()");

    pass(R"(<hello></hello>)");

    fail(R"(<hello></hello><world></world>)");
    fail(R"(<hello></hello><world></world>)");

    pass(R"(<!-- comment -->
            <hello></hello>
            <!-- comment -->
          )");
}

TEST_CASE("element")
{
    pass(R"(<hello></hello>)");
    pass(R"(<hello><world></world></hello>)");
    pass(R"(<hello><world/></hello>)");

    fail(R"(<hello>)");
    fail(R"(<hello></world>)");
    fail(R"(<hello></hello></hello>)");
    fail(R"(<hello><world></hello></world>)");

    pass(R"(<hello  ></hello >)");
}

TEST_CASE("name")
{
    pass(R"(<a/>)");
    pass(R"(<_/>)");
    pass(R"(<:/>)");

    pass(R"(<aa/>)");
    pass(R"(<a_/>)");
    pass(R"(<a:/>)");
    pass(R"(<a-/>)");
    pass(R"(<a./>)");
    pass(R"(<a1/>)");

    fail(R"(</>)");
    fail(R"(<-/>)");
    fail(R"(<./>)");
    fail(R"(<1/>)");
}

TEST_CASE("comment")
{
    pass(R"(<hello><!----></hello>)");
    pass(R"(<hello><!-- comment --></hello>)");
    pass(R"(<hello><!-- comment <!-- comment --></hello>)");
    pass(R"(<hello><!-- comment <tag> comment --></hello>)");

    fail(R"(<hello><!-- comment)");
    fail(R"(<hello><!-- comment </hello>)");
}

TEST_CASE("cdata")
{
    pass(R"(<hello><![CDATA[]]></hello>)");
    pass(R"(<hello><![CDATA[ cdata ]]></hello>)");
    pass(R"(<hello><![CDATA[ <hello> ]]></hello>)");

    fail(R"(<hello><![CDATA[ cdata)");
    fail(R"(<hello><![CDATA[ cdata </hello>)");
}

TEST_CASE("reference")
{
    pass(R"(<hello>&quot;</hello>)");
    pass(R"(<hello>&amp;</hello>)");
    pass(R"(<hello>&apos;</hello>)");
    pass(R"(<hello>&lt;</hello>)");
    pass(R"(<hello>&gt;</hello>)");

    fail(R"(<hello>&;</hello>)");
    fail(R"(<hello>&hello;</hello>)");

    fail(R"(<hello>&quot</hello>)");
}

TEST_CASE("text")
{
    pass(R"(<hello>World</hello>)");
    pass(R"(<hello>World 1234567890 !@#$%^*()</hello>)");
    pass(R"(<hello>World 


    </hello>)");
    pass(R"(<hello>1 > 2</hello>)");

    fail(R"(<hello>World & People</hello>)");
    fail(R"(<hello>1 < 2</hello>)");
}

