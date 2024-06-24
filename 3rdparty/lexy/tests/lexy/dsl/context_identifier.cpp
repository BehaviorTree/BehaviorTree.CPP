// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/context_identifier.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};
} // namespace

TEST_CASE("dsl::context_identifier")
{
    // Note: runtime checks only here due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89074.
    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    constexpr auto var   = dsl::context_identifier<struct id>(dsl::identifier(dsl::ascii::alpha));
    constexpr auto setup = var.create() + var.capture() + LEXY_LIT("-");

    constexpr auto callback = [](const char*, lexy::string_lexeme<>) { return 0; };

    SUBCASE("as rule")
    {
        constexpr auto rule = setup + var.rematch();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());
        auto zero = LEXY_VERIFY_RUNTIME("-");
        CHECK(zero.status == test_result::fatal_error);
        CHECK(zero.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());

        auto one = LEXY_VERIFY_RUNTIME("a-a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace
              == test_trace().token("identifier", "a").literal("-").token("identifier", "a"));
        auto two = LEXY_VERIFY_RUNTIME("ab-ab");
        CHECK(two.status == test_result::success);
        CHECK(two.trace
              == test_trace().token("identifier", "ab").literal("-").token("identifier", "ab"));
        auto three = LEXY_VERIFY_RUNTIME("abc-abc");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace().token("identifier", "abc").literal("-").token("identifier", "abc"));

        auto mismatch = LEXY_VERIFY_RUNTIME("abc-abd");
        CHECK(mismatch.status == test_result::recovered_error);
        CHECK(mismatch.trace
              == test_trace()
                     .token("identifier", "abc")
                     .literal("-")
                     .token("identifier", "abd")
                     .error(4, 7, "different identifier"));
        auto mismatch_length = LEXY_VERIFY_RUNTIME("abc-abcd");
        CHECK(mismatch_length.status == test_result::recovered_error);
        CHECK(mismatch_length.trace
              == test_trace()
                     .token("identifier", "abc")
                     .literal("-")
                     .token("identifier", "abcd")
                     .error(4, 8, "different identifier"));

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto whitespace = LEXY_VERIFY_RUNTIME_P(production, "abc.-.abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.trace
              == test_trace()
                     .token("identifier", "abc")
                     .whitespace(".")
                     .literal("-")
                     .whitespace(".")
                     .token("identifier", "abc")
                     .whitespace("..."));
    }
    SUBCASE("as rule with .error")
    {
        constexpr auto rule = setup + var.rematch().error<my_error>;

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());
        auto zero = LEXY_VERIFY_RUNTIME("-");
        CHECK(zero.status == test_result::fatal_error);
        CHECK(zero.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());

        auto one = LEXY_VERIFY_RUNTIME("a-a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace
              == test_trace().token("identifier", "a").literal("-").token("identifier", "a"));
        auto two = LEXY_VERIFY_RUNTIME("ab-ab");
        CHECK(two.status == test_result::success);
        CHECK(two.trace
              == test_trace().token("identifier", "ab").literal("-").token("identifier", "ab"));
        auto three = LEXY_VERIFY_RUNTIME("abc-abc");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace().token("identifier", "abc").literal("-").token("identifier", "abc"));

        auto mismatch = LEXY_VERIFY_RUNTIME("abc-abd");
        CHECK(mismatch.status == test_result::recovered_error);
        CHECK(mismatch.trace
              == test_trace()
                     .token("identifier", "abc")
                     .literal("-")
                     .token("identifier", "abd")
                     .error(4, 7, "my error"));
        auto mismatch_length = LEXY_VERIFY_RUNTIME("abc-abcd");
        CHECK(mismatch_length.status == test_result::recovered_error);
        CHECK(mismatch_length.trace
              == test_trace()
                     .token("identifier", "abc")
                     .literal("-")
                     .token("identifier", "abcd")
                     .error(4, 8, "my error"));
    }

    SUBCASE("as branch")
    {
        constexpr auto rule = setup + dsl::must(var.rematch()).error<my_error>;

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());
        auto zero = LEXY_VERIFY_RUNTIME("-");
        CHECK(zero.status == test_result::fatal_error);
        CHECK(zero.trace == test_trace().expected_char_class(0, "ASCII.alpha").cancel());

        auto one = LEXY_VERIFY_RUNTIME("a-a");
        CHECK(one.status == test_result::success);
        CHECK(one.trace
              == test_trace().token("identifier", "a").literal("-").token("identifier", "a"));
        auto two = LEXY_VERIFY_RUNTIME("ab-ab");
        CHECK(two.status == test_result::success);
        CHECK(two.trace
              == test_trace().token("identifier", "ab").literal("-").token("identifier", "ab"));
        auto three = LEXY_VERIFY_RUNTIME("abc-abc");
        CHECK(three.status == test_result::success);
        CHECK(three.trace
              == test_trace().token("identifier", "abc").literal("-").token("identifier", "abc"));

        auto mismatch = LEXY_VERIFY_RUNTIME("abc-abd");
        CHECK(mismatch.status == test_result::fatal_error);
        CHECK(mismatch.trace
              == test_trace()
                     .token("identifier", "abc")
                     .literal("-")
                     .error(4, 4, "my error")
                     .cancel());
        auto mismatch_length = LEXY_VERIFY_RUNTIME("abc-abcd");
        CHECK(mismatch_length.status == test_result::fatal_error);
        CHECK(mismatch_length.trace
              == test_trace()
                     .token("identifier", "abc")
                     .literal("-")
                     .error(4, 4, "my error")
                     .cancel());
    }
}

