// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/punctuator.hpp>

#include "verify.hpp"

TEST_CASE("dsl::period")
{
    constexpr auto rule = dsl::period;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'.'>));
}

TEST_CASE("dsl::comma")
{
    constexpr auto rule = dsl::comma;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<','>));
}

TEST_CASE("dsl::colon")
{
    constexpr auto rule = dsl::colon;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<':'>));
}

TEST_CASE("dsl::double_colon")
{
    constexpr auto rule = dsl::double_colon;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, LEXY_LIT("::")));
}

TEST_CASE("dsl::semicolon")
{
    constexpr auto rule = dsl::semicolon;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<';'>));
}

TEST_CASE("dsl::exclamation_mark")
{
    constexpr auto rule = dsl::exclamation_mark;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'!'>));
}

TEST_CASE("dsl::question_mark")
{
    constexpr auto rule = dsl::question_mark;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'?'>));
}

TEST_CASE("dsl::hyphen")
{
    constexpr auto rule = dsl::hyphen;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'-'>));
}

TEST_CASE("dsl::slash")
{
    constexpr auto rule = dsl::slash;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'/'>));
}

TEST_CASE("dsl::backslash")
{
    constexpr auto rule = dsl::backslash;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'\\'>));
}

TEST_CASE("dsl::apostrophe")
{
    constexpr auto rule = dsl::apostrophe;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'\''>));
}

TEST_CASE("dsl::ampersand")
{
    constexpr auto rule = dsl::ampersand;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'&'>));
}

TEST_CASE("dsl::caret")
{
    constexpr auto rule = dsl::caret;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'^'>));
}

TEST_CASE("dsl::asterisk")
{
    constexpr auto rule = dsl::asterisk;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'*'>));
}

TEST_CASE("dsl::tilde")
{
    constexpr auto rule = dsl::tilde;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'~'>));
}

TEST_CASE("dsl::vbar")
{
    constexpr auto rule = dsl::vbar;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'|'>));
}

TEST_CASE("dsl::hash_sign")
{
    constexpr auto rule = dsl::hash_sign;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'#'>));
}

TEST_CASE("dsl::dollar_sign")
{
    constexpr auto rule = dsl::dollar_sign;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'$'>));
}

TEST_CASE("dsl::at_sign")
{
    constexpr auto rule = dsl::at_sign;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'@'>));
}

TEST_CASE("dsl::percent_sign")
{
    constexpr auto rule = dsl::percent_sign;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'%'>));
}

TEST_CASE("dsl::equal_sign")
{
    constexpr auto rule = dsl::equal_sign;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    CHECK(equivalent_rules(rule, dsl::lit_c<'='>));
}

