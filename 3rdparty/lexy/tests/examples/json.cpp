// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/json.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/action/match.hpp>
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/parse_tree_doctest.hpp>

// We copy the conformance tests from https://github.com/miloyip/nativejson-benchmark.

//=== pass/fail validation ===//
namespace
{
void fail(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    REQUIRE(!lexy::match<grammar::json>(input));
}

void pass(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);
    REQUIRE(lexy::match<grammar::json>(input));
}
} // namespace

TEST_CASE("fail02")
{
    fail(R"(["Unclosed array")");
}

TEST_CASE("fail03")
{
    fail(R"({unquoted_key: "keys must be quoted"})");
}

TEST_CASE("fail04")
{
    fail(R"(["extra comma",])");
}

TEST_CASE("fail05")
{
    fail(R"(["double extra comma",,])");
}

TEST_CASE("fail06")
{
    fail(R"([   , "<-- missing value"])");
}

TEST_CASE("fail07")
{
    fail(R"(["Comma after the close"],)");
}

TEST_CASE("fail08")
{
    fail(R"(["Extra close"]])");
}

TEST_CASE("fail09")
{
    fail(R"({"Extra comma": true,})");
}

TEST_CASE("fail10")
{
    fail(R"({"Extra value after close": true} "misplaced quoted value")");
}

TEST_CASE("fail11")
{
    fail(R"({"Illegal expression": 1 + 2})");
}

TEST_CASE("fail12")
{
    fail(R"({"Illegal invocation": alert()})");
}

TEST_CASE("fail13")
{
    fail(R"({"Numbers cannot have leading zeroes": 013})");
}

TEST_CASE("fail14")
{
    fail(R"({"Numbers cannot be hex": 0x14})");
}

TEST_CASE("fail15")
{
    fail(R"(["Illegal backslash escape: \x15"])");
}

TEST_CASE("fail16")
{
    fail(R"([\naked])");
}

TEST_CASE("fail17")
{
    fail(R"(["Illegal backslash escape: \017"])");
}

TEST_CASE("fail18")
{
    fail(R"([[[[[[[[[[[[[[[[[[[["Too deep"]]]]]]]]]]]]]]]]]]]])");
}

TEST_CASE("fail19")
{
    fail(R"({"Missing colon" null})");
}

TEST_CASE("fail20")
{
    fail(R"({"Double colon":: null})");
}

TEST_CASE("fail21")
{
    fail(R"({"Comma instead of colon", null})");
}

TEST_CASE("fail22")
{
    fail(R"(["Colon instead of comma": false])");
}

TEST_CASE("fail23")
{
    fail(R"(["Bad value", truth])");
}

TEST_CASE("fail24")
{
    fail(R"(['single quote'])");
}

TEST_CASE("fail25")
{
    fail("[\"\ttab\tcharacter\tin\tstring\t\"]");
}

TEST_CASE("fail27")
{
    fail("[\"\\\ttab\\\tcharacter\\\tin\\\tstring\\\t\"]");
}

TEST_CASE("fail27")
{
    fail(R"(["line
break"])");
}

TEST_CASE("fail28")
{
    fail(R"(["line\
break"])");
}

TEST_CASE("fail29")
{
    fail(R"([0e])");
}

TEST_CASE("fail30")
{
    fail(R"([0e+])");
}

TEST_CASE("fail31")
{
    fail(R"([0e+-1])");
}

TEST_CASE("fail32")
{
    fail(R"({"Comma instead if closing brace": true,)");
}

TEST_CASE("fail33")
{
    fail(R"(["mismatch"})");
}

TEST_CASE("pass01")
{
    pass(R"([
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&*()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /* <!-- --",
        "# -- --> */": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"])");
}

TEST_CASE("pass02")
{
    pass(R"([[[[[[[[[[[[[[[[[[["Not too deep"]]]]]]]]]]]]]]]]]]])");
}

TEST_CASE("pass03")
{
    // Note: outdated comment.
    pass(R"({
    "JSON Test Pattern pass3": {
        "The outermost value": "must be an object or array.",
        "In this test": "It is an object."
    }
})");
}

//=== parse double ===//
// Not necessary, we don't create doubles.

//=== parse string ===//
namespace
{
auto parse_string(const char* str)
{
    auto input  = lexy::zstring_input<lexy::utf8_encoding>(str);
    auto result = lexy::parse<grammar::json>(input, lexy::noop);
    REQUIRE(result);
    return std::get<ast::json_string>(result.value().v);
}
} // namespace

TEST_CASE("parse_string")
{
    CHECK(parse_string(R"("")") == "");
    CHECK(parse_string(R"("Hello")") == "Hello");
    CHECK(parse_string(R"("Hello\nWorld")") == "Hello\nWorld");
    CHECK(parse_string(R"("Hello\u0000World")") == std::string("Hello\0World", 11));
    CHECK(parse_string(R"("\"\\\/\b\f\n\r\t")") == "\"\\/\b\f\n\r\t");
    CHECK(parse_string(R"("\u0024")") == "\u0024");
    CHECK(parse_string(R"("\u00A2")") == "\u00A2");
    CHECK(parse_string(R"("\u20AC")") == "\u20AC");

    // I'm not sure how the final string test is supposed to work.
}

//=== roundtrip ===//
namespace
{
auto parse_tree(const char* str)
{
    auto input = lexy::zstring_input<lexy::utf8_encoding>(str);

    lexy::parse_tree_for<decltype(input)> tree;
    auto result = lexy::parse_as_tree<grammar::json>(tree, input, lexy::noop);
    REQUIRE(result);
    return tree;
}
} // namespace

TEST_CASE("roundtrip01")
{
    auto tree = parse_tree(R"(null)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::null{})
                        .literal("null")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip02")
{
    auto tree = parse_tree(R"(true)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::boolean{})
                        .literal("true")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip03")
{
    auto tree = parse_tree(R"(false)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::boolean{})
                        .literal("false")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip04")
{
    auto tree = parse_tree(R"(0)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "0")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip05")
{
    auto tree = parse_tree(R"("foo")");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::string{})
                        .literal("\"")
                        .token("foo")
                        .literal("\"")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip06")
{
    auto tree = parse_tree(R"([])");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::array{})
                        .literal("[")
                        .literal("]")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip07")
{
    auto tree = parse_tree(R"({})");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::object{})
                        .literal("{")
                        .literal("}")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip08")
{
    auto tree = parse_tree(R"([0,1])");

    // clang-format off
    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
        .production(grammar::array{})
        .literal("[")
        .production(grammar::number{})
            .token(lexy::digits_token_kind, "0")
            .finish()
        .literal(",")
        .production(grammar::number{})
            .token(lexy::digits_token_kind, "1")
            .finish()
        .literal("]")
        .finish()
        .eof();
    // clang-format on
    CHECK(tree == expected);
}

TEST_CASE("roundtrip08 - whitespace")
{
    auto tree = parse_tree(R"([ 0 , 1 ])");

    // clang-format off
    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
        .production(grammar::array{})
        .literal("[")
        .whitespace(" ")
        .production(grammar::number{})
            .token(lexy::digits_token_kind, "0")
            .finish()
        .whitespace(" ")
        .literal(",")
        .whitespace(" ")
        .production(grammar::number{})
            .token(lexy::digits_token_kind, "1")
            .finish()
        .whitespace(" ")
        .literal("]")
        .finish()
        .eof();
    // clang-format on
    CHECK(tree == expected);
}

TEST_CASE("roundtrip09")
{
    auto tree = parse_tree(R"({"foo":"bar"})");

    // clang-format off
    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
        .production(grammar::object{})
        .literal("{")
        .production(grammar::string{})
            .literal("\"")
            .token("foo")
            .literal("\"")
            .finish()
        .literal(":")
        .production(grammar::string{})
            .literal("\"")
            .token("bar")
            .literal("\"")
            .finish()
        .literal("}")
        .finish()
        .eof();
    // clang-format on
    CHECK(tree == expected);
}

TEST_CASE("roundtrip09 - whitespace")
{
    auto tree = parse_tree(R"({ "foo" : "bar" })");

    // clang-format off
    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
        .production(grammar::object{})
        .literal("{")
        .whitespace(" ")
        .production(grammar::string{})
            .literal("\"")
            .token("foo")
            .literal("\"")
            .finish()
        .whitespace(" ")
        .literal(":")
        .whitespace(" ")
        .production(grammar::string{})
            .literal("\"")
            .token("bar")
            .literal("\"")
            .finish()
        .whitespace(" ")
        .literal("}")
        .finish()
        .eof();
    // clang-format on
    CHECK(tree == expected);
}

TEST_CASE("roundtrip10")
{
    auto tree = parse_tree(R"({"a":null,"foo":"bar"})");

    // clang-format off
    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
        .production(grammar::object{})
        .literal("{")
        .production(grammar::string{})
            .literal("\"")
            .token("a")
            .literal("\"")
            .finish()
        .literal(":")
        .production(grammar::null{})
            .literal("null")
            .finish()
        .literal(",")
        .production(grammar::string{})
            .literal("\"")
            .token("foo")
            .literal("\"")
            .finish()
        .literal(":")
        .production(grammar::string{})
            .literal("\"")
            .token("bar")
            .literal("\"")
            .finish()
        .literal("}")
        .finish()
        .eof();
    // clang-format on
    CHECK(tree == expected);
}

TEST_CASE("roundtrip11")
{
    auto tree = parse_tree(R"(-1)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .literal("-")
                        .token(lexy::digits_token_kind, "1")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip12")
{
    auto tree = parse_tree(R"(-2147483648)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .literal("-")
                        .token(lexy::digits_token_kind, "2147483648")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip13")
{
    auto tree = parse_tree(R"(-1234567890123456789)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .literal("-")
                        .token(lexy::digits_token_kind, "1234567890123456789")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

// 14 has an integer overflow.

TEST_CASE("roundtrip15")
{
    auto tree = parse_tree(R"(1)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "1")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip16")
{
    auto tree = parse_tree(R"(2147483647)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "2147483647")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip17")
{
    auto tree = parse_tree(R"(4294967295)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "4294967295")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip18")
{
    auto tree = parse_tree(R"(1234567890123456789)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "1234567890123456789")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip19")
{
    auto tree = parse_tree(R"(9223372036854775807)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "9223372036854775807")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip20")
{
    auto tree = parse_tree(R"(0.0)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "0")
                        .literal(".")
                        .token(lexy::digits_token_kind, "0")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip21")
{
    auto tree = parse_tree(R"(-0.0)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .literal("-")
                        .token(lexy::digits_token_kind, "0")
                        .literal(".")
                        .token(lexy::digits_token_kind, "0")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip22")
{
    auto tree = parse_tree(R"(1.2345)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "1")
                        .literal(".")
                        .token(lexy::digits_token_kind, "2345")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip23")
{
    auto tree = parse_tree(R"(-1.2345)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .literal("-")
                        .token(lexy::digits_token_kind, "1")
                        .literal(".")
                        .token(lexy::digits_token_kind, "2345")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

TEST_CASE("roundtrip24")
{
    auto tree = parse_tree(R"(5e-324)");

    auto expected = lexy_ext::parse_tree_desc(grammar::json{})
                        .production(grammar::number{})
                        .token(lexy::digits_token_kind, "5")
                        .literal("e")
                        .literal("-")
                        .token(lexy::digits_token_kind, "324")
                        .finish()
                        .eof();
    CHECK(tree == expected);
}

// roundtrip25-27 just test for precision/range, which aren't too interesting here

