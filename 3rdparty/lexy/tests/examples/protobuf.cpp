// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "../../examples/protobuf.cpp" // NOLINT

#include <doctest/doctest.h>
#include <lexy/input/string_input.hpp>

TEST_CASE("varint")
{
    auto parse = [](auto... bytes) {
        unsigned char array[] = {static_cast<unsigned char>(bytes)..., 0};
        auto          input   = lexy::string_input(array, sizeof...(bytes));
        return lexy::parse<grammar::varint>(input, lexy::noop);
    };

    auto empty = parse();
    CHECK(!empty);

    for (auto i = 0; i < 128; ++i)
    {
        auto result = parse(i);
        CHECK(result);
        CHECK(result.value() == i);
    }

    auto v128 = parse(0b1000'0000, 0b0000'0001);
    CHECK(v128.value() == 128);
    auto v255 = parse(0b1111'1111, 0b0000'0001);
    CHECK(v255.value() == 255);
    auto v300 = parse(0b1010'1100, 0b0000'0010);
    CHECK(v300.value() == 300);
    auto v16383 = parse(0b1111'1111, 0b0111'1111);
    CHECK(v16383.value() == 16383);

    auto v16384 = parse(0b1000'0000, 0b1000'0000, 0b0000'0001);
    CHECK(v16384.value() == 16384);

    auto missing = parse(0b1010'1010);
    CHECK(!missing);
}

namespace ast
{
bool operator==(field_varint lhs, field_varint rhs)
{
    return lhs.value == rhs.value;
}
bool operator==(field_32 lhs, field_32 rhs)
{
    return lhs.value == rhs.value;
}
bool operator==(field_64 lhs, field_64 rhs)
{
    return lhs.value == rhs.value;
}
bool operator==(field_bytes lhs, field_bytes rhs)
{
    return lexy::_detail::equal_lexemes(lhs.value, rhs.value);
}
} // namespace ast

TEST_CASE("field")
{
    auto parse = [](auto... bytes) {
        // We're leaking memory, but that's okay.
        auto array = new unsigned char[sizeof...(bytes)];
        {
            auto idx = 0;
            ((array[idx++] = static_cast<unsigned char>(bytes)), ...);
        }
        auto input = lexy::string_input(array, sizeof...(bytes));
        return lexy::parse<grammar::field>(input, lexy::noop);
    };

    auto empty = parse();
    CHECK(!empty);

    SUBCASE("varint")
    {
        auto a = parse(0b0000'0000, 0x42);
        CHECK(a.value().number == 0);
        CHECK(a.value().value == ast::field_value(ast::field_varint{0x42}));

        auto b = parse(0b0000'0000, 0b1000'0000, 0b0000'0001);
        CHECK(b.value().number == 0);
        CHECK(b.value().value == ast::field_value(ast::field_varint{128}));
    }
    SUBCASE("64")
    {
        auto a = parse(0b0000'0001, 0x42, 0, 0, 0, 0, 0, 0, 0);
        CHECK(a.value().number == 0);
        CHECK(a.value().value == ast::field_value(ast::field_64{0x42}));

        auto b = parse(0b0000'0001, 0x42, 0, 0, 0x11, 0, 0, 0, 0);
        CHECK(b.value().number == 0);
        CHECK(b.value().value == ast::field_value(ast::field_64{0x11000042}));

        auto missing = parse(0b0000'0001, 0x42);
        CHECK(!missing);
    }
    SUBCASE("32")
    {
        auto a = parse(0b0000'0101, 0x42, 0, 0, 0);
        CHECK(a.value().number == 0);
        CHECK(a.value().value == ast::field_value(ast::field_32{0x42}));

        auto b = parse(0b0000'0101, 0x42, 0, 0, 0x11);
        CHECK(b.value().number == 0);
        CHECK(b.value().value == ast::field_value(ast::field_32{0x11000042}));

        auto missing = parse(0b0000'0101, 0x42);
        CHECK(!missing);
    }
    SUBCASE("bytes")
    {
        auto a = parse(0b0000'0010, 0x0);
        CHECK(a.value().number == 0);
        CHECK(a.value().value == ast::field_value(ast::field_bytes()));

        unsigned char b_bytes[] = {0xAB};
        auto          b         = parse(0b0000'0010, 0x1, 0xAB);
        CHECK(b.value().number == 0);
        CHECK(b.value().value == ast::field_value(ast::field_bytes{{b_bytes, 1}}));

        unsigned char c_bytes[] = {0xAB, 0xCD, 0xEF};
        auto          c         = parse(0b0000'0010, 0x3, 0xAB, 0xCD, 0xEF);
        CHECK(c.value().number == 0);
        CHECK(c.value().value == ast::field_value(ast::field_bytes{{c_bytes, 3}}));

        auto missing = parse(0b0000'0010, 0x3, 0xAB);
        CHECK(!missing);
    }
    SUBCASE("unknown type")
    {
        auto result = parse(0b0000'0111);
        CHECK(!result);
    }

    SUBCASE("number")
    {
        auto a = parse(0b0000'1000, 0x0);
        CHECK(a.value().number == 1);
        CHECK(a.value().value == ast::field_value(ast::field_varint{0}));
        auto b = parse(0b0100'1000, 0x0);
        CHECK(b.value().number == 9);
        CHECK(b.value().value == ast::field_value(ast::field_varint{0}));
        auto c = parse(0b0111'1000, 0x0);
        CHECK(c.value().number == 15);
        CHECK(c.value().value == ast::field_value(ast::field_varint{0}));

        auto d = parse(0b1000'0000, 0b0000'0001, 0x0);
        CHECK(d.value().number == 16);
        CHECK(d.value().value == ast::field_value(ast::field_varint{0}));
        auto e = parse(0b1000'1000, 0b0000'0001, 0x0);
        CHECK(e.value().number == 17);
        CHECK(e.value().value == ast::field_value(ast::field_varint{0}));
        auto f = parse(0b1100'1000, 0b0000'0001, 0x0);
        CHECK(f.value().number == 25);
        CHECK(f.value().value == ast::field_value(ast::field_varint{0}));
    }
}

TEST_CASE("message")
{
    auto parse = [](auto... bytes) {
        unsigned char array[] = {static_cast<unsigned char>(bytes)..., 0};
        auto          input   = lexy::string_input(array, sizeof...(bytes));
        return lexy::parse<grammar::message>(input, lexy::noop);
    };

    auto empty = parse();
    CHECK(empty.value().empty());

    auto one = parse(0x0, 0x0);
    CHECK(one.value().size() == 1);
    CHECK(one.value()[0].number == 0);
    CHECK(one.value()[0].value == ast::field_value(ast::field_varint{0}));

    auto two = parse(0x0, 0x0, 0b000'1000, 0x11);
    CHECK(two.value().size() == 2);
    CHECK(two.value()[0].number == 0);
    CHECK(two.value()[0].value == ast::field_value(ast::field_varint{0}));
    CHECK(two.value()[1].number == 1);
    CHECK(two.value()[1].value == ast::field_value(ast::field_varint{0x11}));
}

