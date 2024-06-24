// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/input/argv_input.hpp>

#include "verify.hpp"

TEST_CASE("dsl::argv_separator")
{
    constexpr auto rule = dsl::argv_separator;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "argv-separator").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace == test_trace().expected_char_class(0, "argv-separator").cancel());

    static constexpr char* argv[]     = {const_cast<char*>("IGNORED"), const_cast<char*>("abc"),
                                     const_cast<char*>("de"), const_cast<char*>("f"), nullptr};
    constexpr auto         argv_begin = lexy::argv_begin(4, const_cast<char**>(argv));
    constexpr auto         argv_end   = lexy::argv_end(4, const_cast<char**>(argv));

    auto argv_at_arg = LEXY_VERIFY(lexy::argv_input(argv_begin, argv_end));
    CHECK(argv_at_arg.status == test_result::fatal_error);
    CHECK(argv_at_arg.trace == test_trace().expected_char_class(0, "argv-separator").cancel());

    auto argv_in_arg = LEXY_VERIFY(lexy::argv_input(lexy::_detail::next(argv_begin, 2), argv_end));
    CHECK(argv_at_arg.status == test_result::fatal_error);
    CHECK(argv_in_arg.trace == test_trace().expected_char_class(0, "argv-separator").cancel());

    auto argv_sep = LEXY_VERIFY(lexy::argv_input(lexy::_detail::next(argv_begin, 3), argv_end));
    CHECK(argv_sep.status == test_result::success);
    CHECK(argv_sep.trace == test_trace().literal("\\0"));
}

