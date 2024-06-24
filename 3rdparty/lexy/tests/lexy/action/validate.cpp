// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/action/validate.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/adapter.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/input/string_input.hpp>
#include <vector>

namespace
{
struct prod_a
{
    static constexpr auto name = "prod_a";
    static constexpr auto rule = list(LEXY_LIT("abc"));
};
struct prod_b
{
    static constexpr auto name = "prod_b";
    static constexpr auto rule = LEXY_LIT("(") + lexy::dsl::p<prod_a> + LEXY_LIT(")");
};
} // namespace

TEST_CASE("validate")
{
    SUBCASE("void callback")
    {
        constexpr auto check_error = [](const auto& result) {
            CHECK(!result);
            CHECK(!result.is_success());
            CHECK(!result.is_recovered_error());
            CHECK(result.is_fatal_error());
            CHECK(result.error_count() == 1);
        };

        SUBCASE("success")
        {
            constexpr auto callback = [](auto, auto) { FAIL_CHECK("should not be called"); };

            auto one
                = lexy::validate<prod_b>(lexy::zstring_input("(abc)"), lexy::callback(callback));
            CHECK(one);
            CHECK(one.is_success());
            CHECK(!one.is_error());
            CHECK(!one.is_recovered_error());
            CHECK(!one.is_fatal_error());
            CHECK(one.error_count() == 0);

            auto two
                = lexy::validate<prod_b>(lexy::zstring_input("(abcabc)"), lexy::callback(callback));
            CHECK(two);
        }
        SUBCASE("missing abc")
        {
            constexpr auto callback = [](auto ctx, auto error) {
                CHECK(ctx.production() == lexy::_detail::string_view("prod_a"));
                CHECK(*error.position() == ')');
                CHECK(error.string() == lexy::_detail::string_view("abc"));
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("()"), lexy::callback(callback));
            check_error(result);
        }
        SUBCASE("invalid abc")
        {
            constexpr auto callback = [](auto ctx, auto error) {
                CHECK(ctx.production() == lexy::_detail::string_view("prod_a"));
                CHECK(*error.position() == 'a');
                CHECK(error.string() == lexy::_detail::string_view("abc"));
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("(adc)"), lexy::callback(callback));
            check_error(result);
        }
        SUBCASE("missing )")
        {
            constexpr auto callback = [](auto ctx, auto error) {
                CHECK(ctx.production() == lexy::_detail::string_view("prod_b"));
                CHECK(*error.position() == ']');
                CHECK(error.character() == ')');
            };

            auto result
                = lexy::validate<prod_b>(lexy::zstring_input("(abc]"), lexy::callback(callback));
            check_error(result);
        }
    }
    SUBCASE("non-void callback")
    {
        constexpr auto prod_a_error = [](lexy::string_error_context<prod_a>,
                                         lexy::string_error<lexy::expected_literal> error) {
            if (error.string() != lexy::_detail::string_view("abc"))
                throw 0;
            return -1;
        };
        constexpr auto prod_b_error = [](lexy::string_error_context<prod_b>,
                                         lexy::string_error<lexy::expected_literal> error) {
            if (error.character() == '(')
                return -2;
            else if (error.character() == ')')
                return -3;
            else
                return -4;
        };
        constexpr auto callback
            = lexy::collect<std::vector<int>>(lexy::callback<int>(prod_a_error, prod_b_error));

        auto success = lexy::validate<prod_b>(lexy::zstring_input("(abc)"), callback);
        CHECK(success);

        auto missing_abc = lexy::validate<prod_b>(lexy::zstring_input("()"), callback);
        CHECK(!missing_abc);
        CHECK(missing_abc.errors() == std::vector{-1});

        auto empty = lexy::validate<prod_b>(lexy::zstring_input(""), callback);
        CHECK(!empty);
        CHECK(empty.errors() == std::vector{-2});
        auto bad_paren = lexy::validate<prod_b>(lexy::zstring_input("[abc]"), callback);
        CHECK(!bad_paren);
        CHECK(bad_paren.errors() == std::vector{-2});

        auto missing_paren = lexy::validate<prod_b>(lexy::zstring_input("(abc"), callback);
        CHECK(!missing_paren);
        CHECK(missing_paren.errors() == std::vector{-3});
    }
}

