// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/times.hpp>

#include "verify.hpp"
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/separator.hpp>

TEST_CASE("dsl::times<N>(rule)")
{
    constexpr auto rule = dsl::times<3>(dsl::capture(LEXY_LIT("abc")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = [](const char* begin, lexy::string_lexeme<> a,
                                 lexy::string_lexeme<> b, lexy::string_lexeme<> c) {
        CHECK(a.begin() == begin);
        CHECK(a.size() == 3);
        CHECK(a[0] == 'a');
        CHECK(a[1] == 'b');
        CHECK(a[2] == 'c');

        CHECK(b.begin() == begin + 3);
        CHECK(b.size() == 3);
        CHECK(b[0] == 'a');
        CHECK(b[1] == 'b');
        CHECK(b[2] == 'c');

        CHECK(c.begin() == begin + 6);
        CHECK(c.size() == 3);
        CHECK(c[0] == 'a');
        CHECK(c[1] == 'b');
        CHECK(c[2] == 'c');

        return 0;
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());
    auto one = LEXY_VERIFY("abc");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace == test_trace().literal("abc").expected_literal(3, "abc", 0).cancel());
    auto two = LEXY_VERIFY("abcabc");
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace
          == test_trace().literal("abc").literal("abc").expected_literal(6, "abc", 0).cancel());

    auto three = LEXY_VERIFY("abcabcabc");
    CHECK(three.status == test_result::success);
    CHECK(three.trace == test_trace().literal("abc").literal("abc").literal("abc"));
    auto four = LEXY_VERIFY("abcabcabcabc");
    CHECK(four.status == test_result::success);
    CHECK(four.trace == test_trace().literal("abc").literal("abc").literal("abc"));
}

TEST_CASE("dsl::times<N>(rule, dsl::sep())")
{
    constexpr auto rule = dsl::times<3>(dsl::capture(LEXY_LIT("abc")), dsl::sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = [](const char* begin, lexy::string_lexeme<> a,
                                 lexy::string_lexeme<> b, lexy::string_lexeme<> c) {
        CHECK(a.begin() == begin);
        CHECK(a.size() == 3);
        CHECK(a[0] == 'a');
        CHECK(a[1] == 'b');
        CHECK(a[2] == 'c');

        CHECK(b.begin() == begin + 4);
        CHECK(b.size() == 3);
        CHECK(b[0] == 'a');
        CHECK(b[1] == 'b');
        CHECK(b[2] == 'c');

        CHECK(c.begin() == begin + 8);
        CHECK(c.size() == 3);
        CHECK(c[0] == 'a');
        CHECK(c[1] == 'b');
        CHECK(c[2] == 'c');

        return 0;
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

    auto one = LEXY_VERIFY("abc");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace == test_trace().literal("abc").expected_literal(3, ",", 0).cancel());
    auto one_c = LEXY_VERIFY("abc,");
    CHECK(one_c.status == test_result::fatal_error);
    CHECK(one_c.trace
          == test_trace().literal("abc").literal(",").expected_literal(4, "abc", 0).cancel());

    auto two       = LEXY_VERIFY("abc,abc");
    auto two_trace = test_trace()
                         .literal("abc")
                         .literal(",")
                         .literal("abc")
                         .expected_literal(7, ",", 0)
                         .cancel();
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace == two_trace);
    auto two_c       = LEXY_VERIFY("abc,abc,");
    auto two_c_trace = test_trace()
                           .literal("abc")
                           .literal(",")
                           .literal("abc")
                           .literal(",")
                           .expected_literal(8, "abc", 0)
                           .cancel();
    CHECK(two_c.status == test_result::fatal_error);
    CHECK(two_c.trace == two_c_trace);

    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three.status == test_result::success);
    CHECK(three.trace
          == test_trace().literal("abc").literal(",").literal("abc").literal(",").literal("abc"));

    auto three_c       = LEXY_VERIFY("abc,abc,abc,");
    auto three_c_trace = test_trace()
                             .literal("abc")
                             .literal(",")
                             .literal("abc")
                             .literal(",")
                             .literal("abc")
                             .literal(",")
                             .error(11, 12, "unexpected trailing separator");
    CHECK(three_c.status == test_result::recovered_error);
    CHECK(three_c.trace == three_c_trace);
}

TEST_CASE("dsl::times<N>(rule, dsl::trailing_sep())")
{
    constexpr auto rule
        = dsl::times<3>(dsl::capture(LEXY_LIT("abc")), dsl::trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = [](const char* begin, lexy::string_lexeme<> a,
                                 lexy::string_lexeme<> b, lexy::string_lexeme<> c) {
        CHECK(a.begin() == begin);
        CHECK(a.size() == 3);
        CHECK(a[0] == 'a');
        CHECK(a[1] == 'b');
        CHECK(a[2] == 'c');

        CHECK(b.begin() == begin + 4);
        CHECK(b.size() == 3);
        CHECK(b[0] == 'a');
        CHECK(b[1] == 'b');
        CHECK(b[2] == 'c');

        CHECK(c.begin() == begin + 8);
        CHECK(c.size() == 3);
        CHECK(c[0] == 'a');
        CHECK(c[1] == 'b');
        CHECK(c[2] == 'c');

        return 0;
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

    auto one = LEXY_VERIFY("abc");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace == test_trace().literal("abc").expected_literal(3, ",", 0).cancel());
    auto one_c = LEXY_VERIFY("abc,");
    CHECK(one_c.status == test_result::fatal_error);
    CHECK(one_c.trace
          == test_trace().literal("abc").literal(",").expected_literal(4, "abc", 0).cancel());

    auto two       = LEXY_VERIFY("abc,abc");
    auto two_trace = test_trace()
                         .literal("abc")
                         .literal(",")
                         .literal("abc")
                         .expected_literal(7, ",", 0)
                         .cancel();
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace == two_trace);
    auto two_c       = LEXY_VERIFY("abc,abc,");
    auto two_c_trace = test_trace()
                           .literal("abc")
                           .literal(",")
                           .literal("abc")
                           .literal(",")
                           .expected_literal(8, "abc", 0)
                           .cancel();
    CHECK(two_c.status == test_result::fatal_error);
    CHECK(two_c.trace == two_c_trace);

    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three.status == test_result::success);
    CHECK(three.trace
          == test_trace().literal("abc").literal(",").literal("abc").literal(",").literal("abc"));

    auto three_c       = LEXY_VERIFY("abc,abc,abc,");
    auto three_c_trace = test_trace()
                             .literal("abc")
                             .literal(",")
                             .literal("abc")
                             .literal(",")
                             .literal("abc")
                             .literal(",");
    CHECK(three_c.status == test_result::success);
    CHECK(three_c.trace == three_c_trace);
}

TEST_CASE("dsl::times<N>(rule, dsl::ignore_trailing_sep())")
{
    constexpr auto rule
        = dsl::times<3>(dsl::capture(LEXY_LIT("abc")), dsl::ignore_trailing_sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(rule)>);

    constexpr auto callback = [](const char* begin, lexy::string_lexeme<> a,
                                 lexy::string_lexeme<> b, lexy::string_lexeme<> c) {
        CHECK(a.begin() == begin);
        CHECK(a.size() == 3);
        CHECK(a[0] == 'a');
        CHECK(a[1] == 'b');
        CHECK(a[2] == 'c');

        CHECK(b.begin() == begin + 4);
        CHECK(b.size() == 3);
        CHECK(b[0] == 'a');
        CHECK(b[1] == 'b');
        CHECK(b[2] == 'c');

        CHECK(c.begin() == begin + 8);
        CHECK(c.size() == 3);
        CHECK(c[0] == 'a');
        CHECK(c[1] == 'b');
        CHECK(c[2] == 'c');

        return 0;
    };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

    auto one = LEXY_VERIFY("abc");
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace == test_trace().literal("abc").expected_literal(3, ",", 0).cancel());
    auto one_c = LEXY_VERIFY("abc,");
    CHECK(one_c.status == test_result::fatal_error);
    CHECK(one_c.trace
          == test_trace().literal("abc").literal(",").expected_literal(4, "abc", 0).cancel());

    auto two       = LEXY_VERIFY("abc,abc");
    auto two_trace = test_trace()
                         .literal("abc")
                         .literal(",")
                         .literal("abc")
                         .expected_literal(7, ",", 0)
                         .cancel();
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace == two_trace);
    auto two_c       = LEXY_VERIFY("abc,abc,");
    auto two_c_trace = test_trace()
                           .literal("abc")
                           .literal(",")
                           .literal("abc")
                           .literal(",")
                           .expected_literal(8, "abc", 0)
                           .cancel();
    CHECK(two_c.status == test_result::fatal_error);
    CHECK(two_c.trace == two_c_trace);

    auto three = LEXY_VERIFY("abc,abc,abc");
    CHECK(three.status == test_result::success);
    CHECK(three.trace
          == test_trace().literal("abc").literal(",").literal("abc").literal(",").literal("abc"));

    auto three_c = LEXY_VERIFY("abc,abc,abc,");
    auto three_c_trace
        = test_trace().literal("abc").literal(",").literal("abc").literal(",").literal("abc");
    CHECK(three_c.status == test_result::success);
    CHECK(three_c.trace == three_c_trace);
}

TEST_CASE("dsl::twice()")
{
    constexpr auto no_sep = dsl::twice(LEXY_LIT("abc"));
    CHECK(lexy::is_rule<decltype(no_sep)>);
    CHECK(equivalent_rules(no_sep, dsl::times<2>(LEXY_LIT("abc"))));

    constexpr auto sep = dsl::twice(LEXY_LIT("abc"), dsl::sep(LEXY_LIT(",")));
    CHECK(lexy::is_rule<decltype(no_sep)>);
    CHECK(equivalent_rules(sep, dsl::times<2>(LEXY_LIT("abc"), dsl::sep(LEXY_LIT(",")))));
}

