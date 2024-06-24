// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/error.hpp>

#include <doctest/doctest.h>
#include <lexy/input/lexeme_input.hpp>
#include <lexy/input/string_input.hpp>

TEST_CASE("error_context")
{
    struct production
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "production";
        }
    };

    SUBCASE("normal input")
    {
        auto input   = lexy::zstring_input("abc");
        auto context = lexy::error_context(production{}, input, input.data());
        CHECK(&context.input() == &input);
        CHECK(context.production() == lexy::_detail::string_view("production"));
        CHECK(context.position() == input.data());
    }
    SUBCASE("lexeme input")
    {
        auto parent  = lexy::zstring_input("abc");
        auto input   = lexy::lexeme_input(parent, parent.data() + 1, parent.data() + 2);
        auto context = lexy::error_context(production{}, input, parent.data() + 1);
        CHECK(context.input().data() == parent.data());
        CHECK(context.input().size() == parent.size());
        CHECK(context.production() == lexy::_detail::string_view("production"));
        CHECK(context.position() == parent.data() + 1);
    }
}

