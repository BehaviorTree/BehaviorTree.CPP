// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/digit.hpp>

#include "verify.hpp"

TEST_CASE("dsl::zero")
{
    constexpr auto rule = dsl::zero;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.zero").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto zerozero = LEXY_VERIFY("00");
    CHECK(zerozero.status == test_result::success);
    CHECK(zerozero.trace == test_trace().token("digits", "0"));

    auto nine = LEXY_VERIFY("9");
    CHECK(nine.status == test_result::fatal_error);
    CHECK(nine.trace == test_trace().expected_char_class(0, "digit.zero").cancel());

    auto utf16 = LEXY_VERIFY(u"0");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "0"));
}

TEST_CASE("dsl::digit")
{
    constexpr auto callback = token_callback;

    auto check_valid = [&](auto rule, auto... _digits) {
        char digits[] = {_digits...};
        auto value    = 0;
        for (auto digit : digits)
        {
            auto result = LEXY_VERIFY_RUNTIME(lexy::ascii_encoding{}, digit, digit, digit);
            CHECK(result.status == test_result::success);
            CHECK(result.trace == test_trace().token("digits", doctest::String(&digit, 1).c_str()));
            CHECK(rule.digit_value(digit) == value);
            ++value;
        }
    };
    auto check_invalid = [&](auto rule, const char* name, auto... _digits) {
        char digits[] = {_digits...};
        for (auto digit : digits)
        {
            auto result = LEXY_VERIFY_RUNTIME(lexy::ascii_encoding{}, digit, digit, digit);
            CHECK(result.status == test_result::fatal_error);
            CHECK(result.trace == test_trace().expected_char_class(0, name).cancel());
            CHECK(rule.digit_value(digit) >= rule.digit_radix);
        }
    };

    SUBCASE("binary")
    {
        constexpr auto rule = dsl::digit<dsl::binary>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(rule.digit_radix == 2);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.binary").cancel());

        check_valid(rule, '0', '1');
        check_invalid(rule, "digit.binary", '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c',
                      'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F');
    }
    SUBCASE("octal")
    {
        constexpr auto rule = dsl::digit<dsl::octal>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(rule.digit_radix == 8);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.octal").cancel());

        check_valid(rule, '0', '1', '2', '3', '4', '5', '6', '7');
        check_invalid(rule, "digit.octal", '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C',
                      'D', 'E', 'F');
    }
    SUBCASE("decimal")
    {
        constexpr auto rule = dsl::digit<dsl::decimal>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(rule.digit_radix == 10);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

        check_valid(rule, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
        check_invalid(rule, "digit.decimal", 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E',
                      'F');
    }
    SUBCASE("hex_lower")
    {
        constexpr auto rule = dsl::digit<dsl::hex_lower>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(rule.digit_radix == 16);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.hex-lower").cancel());

        check_valid(rule, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
                    'f');
        check_invalid(rule, "digit.hex-lower", 'A', 'B', 'C', 'D', 'E', 'F');
    }
    SUBCASE("hex_upper")
    {
        constexpr auto rule = dsl::digit<dsl::hex_upper>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(rule.digit_radix == 16);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.hex-upper").cancel());

        check_valid(rule, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                    'F');
        check_invalid(rule, "digit.hex-upper", 'a', 'b', 'c', 'd', 'e', 'f');
    }
    SUBCASE("hex")
    {
        constexpr auto rule = dsl::digit<dsl::hex>;
        CHECK(lexy::is_token_rule<decltype(rule)>);
        CHECK(rule.digit_radix == 16);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit.hex").cancel());

        check_valid(rule, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
                    'f');
        check_valid(rule, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                    'F');
        check_invalid(rule, "digit.hex", 'g', 'x', 'y', 'z');
    }
}

TEST_CASE("dsl::digits<>")
{
    constexpr auto rule = dsl::digits<>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::success);
    CHECK(zero_zero_seven.trace == test_trace().token("digits", "007"));

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("dsl::digits<>.no_leading_zero()")
{
    constexpr auto rule = dsl::digits<>.no_leading_zero();
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::fatal_error);
    CHECK(zero_zero_seven.trace
          == test_trace().error_token("0").error(0, 1, "forbidden leading zero").cancel());

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("dsl::digits<>.sep()")
{
    constexpr auto rule = dsl::digits<>.sep(LEXY_LIT("_"));
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::success);
    CHECK(zero_zero_seven.trace == test_trace().token("digits", "007"));

    auto with_sep = LEXY_VERIFY("1_2_3");
    CHECK(with_sep.status == test_result::success);
    CHECK(with_sep.trace == test_trace().token("digits", "1_2_3"));

    auto leading_sep = LEXY_VERIFY("_1");
    CHECK(leading_sep.status == test_result::fatal_error);
    CHECK(leading_sep.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());
    auto trailing_sep = LEXY_VERIFY("1_");
    CHECK(trailing_sep.status == test_result::fatal_error);
    CHECK(trailing_sep.trace
          == test_trace().error_token("1_").expected_char_class(2, "digit.decimal").cancel());

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("dsl::digits<>.sep().no_leading_zero")
{
    constexpr auto rule = dsl::digits<>.sep(LEXY_LIT("_")).no_leading_zero();
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, dsl::digits<>.no_leading_zero().sep(LEXY_LIT("_"))));

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto zero = LEXY_VERIFY("0");
    CHECK(zero.status == test_result::success);
    CHECK(zero.trace == test_trace().token("digits", "0"));
    auto six = LEXY_VERIFY("6");
    CHECK(six.status == test_result::success);
    CHECK(six.trace == test_trace().token("digits", "6"));
    auto three_seven = LEXY_VERIFY("37");
    CHECK(three_seven.status == test_result::success);
    CHECK(three_seven.trace == test_trace().token("digits", "37"));
    auto one_two_three = LEXY_VERIFY("123");
    CHECK(one_two_three.status == test_result::success);
    CHECK(one_two_three.trace == test_trace().token("digits", "123"));

    auto zero_zero_seven = LEXY_VERIFY("007");
    CHECK(zero_zero_seven.status == test_result::fatal_error);
    CHECK(zero_zero_seven.trace
          == test_trace().error_token("0").error(0, 1, "forbidden leading zero").cancel());
    auto zero_sep_zero_seven = LEXY_VERIFY("0_07");
    CHECK(zero_sep_zero_seven.status == test_result::fatal_error);
    CHECK(zero_sep_zero_seven.trace
          == test_trace().error_token("0").error(0, 1, "forbidden leading zero").cancel());

    auto with_sep = LEXY_VERIFY("1_2_3");
    CHECK(with_sep.status == test_result::success);
    CHECK(with_sep.trace == test_trace().token("digits", "1_2_3"));

    auto leading_sep = LEXY_VERIFY("_1");
    CHECK(leading_sep.status == test_result::fatal_error);
    CHECK(leading_sep.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());
    auto trailing_sep = LEXY_VERIFY("1_");
    CHECK(trailing_sep.status == test_result::fatal_error);
    CHECK(trailing_sep.trace
          == test_trace().error_token("1_").expected_char_class(2, "digit.decimal").cancel());

    auto utf16 = LEXY_VERIFY(u"11");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().token("digits", "11"));
}

TEST_CASE("digit separators")
{
    CHECK(equivalent_rules(dsl::digit_sep_tick, LEXY_LIT("'")));
    CHECK(equivalent_rules(dsl::digit_sep_underscore, LEXY_LIT("_")));
}

TEST_CASE("dsl::n_digits")
{
    constexpr auto rule = dsl::n_digits<3>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto one = LEXY_VERIFY("1");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace
          == test_trace().error_token("1").expected_char_class(1, "digit.decimal").cancel());
    auto two = LEXY_VERIFY("12");
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace
          == test_trace().error_token("12").expected_char_class(2, "digit.decimal").cancel());

    auto three = LEXY_VERIFY("123");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().token("digits", "123"));
    auto four = LEXY_VERIFY("1234");
    CHECK(four.status == test_result::success);
    CHECK(four.trace == test_trace().token("digits", "123"));
}

TEST_CASE("dsl::n_digits.sep()")
{
    constexpr auto rule = dsl::n_digits<3>.sep(LEXY_LIT("_"));
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());

    auto one = LEXY_VERIFY("1");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace
          == test_trace().error_token("1").expected_char_class(1, "digit.decimal").cancel());
    auto two = LEXY_VERIFY("12");
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace
          == test_trace().error_token("12").expected_char_class(2, "digit.decimal").cancel());

    auto three = LEXY_VERIFY("123");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().token("digits", "123"));
    auto four = LEXY_VERIFY("1234");
    CHECK(four.status == test_result::success);
    CHECK(four.trace == test_trace().token("digits", "123"));

    auto with_sep = LEXY_VERIFY("1_2_3");
    CHECK(with_sep.status == test_result::success);
    CHECK(with_sep.trace == test_trace().token("digits", "1_2_3"));

    auto leading_sep = LEXY_VERIFY("_1");
    CHECK(leading_sep.status == test_result::fatal_error);
    CHECK(leading_sep.trace == test_trace().expected_char_class(0, "digit.decimal").cancel());
    auto trailing_sep = LEXY_VERIFY("1_");
    CHECK(trailing_sep.status == test_result::fatal_error);
    CHECK(trailing_sep.trace
          == test_trace().error_token("1_").expected_char_class(2, "digit.decimal").cancel());
}

