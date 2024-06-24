// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/scan.hpp>

#include "verify.hpp"
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/integer.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/production.hpp>

namespace
{
struct simple_scan : lexy::scan_production<int>, test_production
{
    // We need to ensure that the input is actually advanced.
    static constexpr auto rule = dsl::scan + dsl::eof;

    struct integer
    {
        static constexpr auto name  = "integer";
        static constexpr auto rule  = dsl::integer<int>;
        static constexpr auto value = lexy::forward<int>;
    };

    struct invalid_integer
    {
        static constexpr auto name = "invalid-integer";
    };

    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        if (scanner.branch(LEXY_LIT("abc")) || scanner.peek(dsl::digit<>))
        {
            auto begin   = scanner.position();
            auto integer = scanner.parse(simple_scan::integer{});
            auto end     = scanner.position();
            if (!scanner)
                return {};

            if (integer.value() < 10)
                scanner.error(invalid_integer{}, begin, end);
            return integer.value();
        }
        else
        {
            // Note that we put the scanner in a failed state, but return a value nonetheless.
            // This means that parsing does not fail.
            scanner.fatal_error(lexy::expected_char_class{}, scanner.begin(), "digit");
            return 0;
        }
    }
};

struct no_value_scan : lexy::scan_production<void>, test_production
{
    struct literal
    {
        static constexpr auto name = "literal";
        static constexpr auto rule = LEXY_LIT("abc");
        // note: no value
    };

    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        scanner.parse(dsl::p<literal>);
        return true;
    }
};

struct state_scan : lexy::scan_production<const char*>, test_production
{
    // Overload is required as we also call lexy::match without a state.
    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>&)
    {
        return nullptr;
    }

    template <typename Reader, typename Context, typename State>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>&, const State& state)
    {
        // The parse state is the test handler itself.
        return state.begin();
    }
};

struct branch_scan : lexy::scan_production<const char*>, test_production
{
    static constexpr auto name = "branch_scan";
    static constexpr auto rule = dsl::capture(LEXY_LIT("abc")) >> dsl::scan;

    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner,
                                      lexy::lexeme<Reader>                 lexeme)
    {
        scanner.parse(LEXY_LIT("def"));
        return lexeme.end();
    }
};

struct recursive_scan : lexy::scan_production<int>, test_production
{
    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        if (scan_result result;
            scanner.branch(result, LEXY_LIT("(") >> dsl::recurse<recursive_scan> + LEXY_LIT(")")))
            return result.value() + 1;
        else
        {
            scanner.parse(dsl::lit_c<'x'>);
            return 0;
        }
    }
};

struct token : lexy::token_production
{
    static constexpr auto name = "token";

    static constexpr auto whitespace = dsl::lit_c<'-'>;
    static constexpr auto rule       = LEXY_LIT("abc");
    static constexpr auto value      = lexy::forward<void>;
};

struct whitespace_scan : lexy::scan_production<void>, test_production
{
    static constexpr auto whitespace = dsl::lit_c<'+'>;

    template <typename Reader, typename Context>
    static constexpr scan_result scan(lexy::rule_scanner<Context, Reader>& scanner)
    {
        // The scanner needs to skip whitespace here!
        return scanner.parse(token{});
    }
};
} // namespace

TEST_CASE("dsl::scan")
{
    // Note: most part of the scanning interface tested by lexy::scan.
    // Focus on interaction with parse events here.
    CHECK(lexy::is_rule<decltype(dsl::scan)>);

    SUBCASE("simple")
    {
        constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                      [](const char*, int value) { return value; });

        auto empty = LEXY_VERIFY_P(simple_scan, "");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_char_class(0, "digit").eof());

        auto abc = LEXY_VERIFY_P(simple_scan, "abc");
        CHECK(abc.status == test_result::fatal_error);
        // clang-format off
        CHECK(abc.trace == test_trace()
                             .literal("abc")
                             .production("integer")
                                 .expected_char_class(3, "digit.decimal")
                                 .cancel()
                             .cancel());
        // clang-format on
        auto abc_small = LEXY_VERIFY_P(simple_scan, "abc4");
        CHECK(abc_small.status == test_result::recovered_error);
        CHECK(abc_small.value == 4);
        CHECK(abc_small.trace
              == test_trace()
                     .literal("abc")
                     .production("integer")
                     .token("digits", "4")
                     .finish()
                     .error(3, 4, "invalid-integer")
                     .eof());
        auto abc_big = LEXY_VERIFY_P(simple_scan, "abc42");
        CHECK(abc_big.status == test_result::success);
        CHECK(abc_big.value == 42);
        CHECK(abc_big.trace
              == test_trace()
                     .literal("abc")
                     .production("integer")
                     .token("digits", "42")
                     .finish()
                     .eof());

        auto small = LEXY_VERIFY_P(simple_scan, "4");
        CHECK(small.status == test_result::recovered_error);
        CHECK(small.value == 4);
        CHECK(small.trace
              == test_trace()
                     .backtracked("4")
                     .production("integer")
                     .token("digits", "4")
                     .finish()
                     .error(0, 1, "invalid-integer")
                     .eof());
        auto big = LEXY_VERIFY_P(simple_scan, "42");
        CHECK(big.status == test_result::success);
        CHECK(big.value == 42);
        CHECK(big.trace
              == test_trace()
                     .backtracked("4")
                     .production("integer")
                     .token("digits", "42")
                     .finish()
                     .eof());
    }
    SUBCASE("void")
    {
        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY_P(no_value_scan, "");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.trace
              == test_trace().production("literal").expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY_P(no_value_scan, "abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().production("literal").literal("abc"));
    }
    SUBCASE("with state")
    {
        constexpr auto callback = [](const char* begin, const char* value) {
            CHECK(begin == value);
            return 0;
        };

        auto empty = LEXY_VERIFY_P(state_scan, "");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());
    }
    SUBCASE("branch scan")
    {
        constexpr auto callback = lexy::callback<int>(
            // branch_scan production
            [](const char*) { return 0; },
            [](const char* begin, const char* value) {
                CHECK(begin + 3 == value);
                return 1;
            },
            // top-level production
            [](const char*, int i) { return i; });

        constexpr auto rule = dsl::if_(dsl::p<branch_scan>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().production("branch_scan").cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::recovered_error);
        CHECK(abc.trace
              == test_trace() //
                     .production("branch_scan")
                     .literal("abc")
                     .expected_literal(3, "def", 0));

        auto abcdef = LEXY_VERIFY("abcdef");
        CHECK(abcdef.status == test_result::success);
        CHECK(abcdef.value == 1);
        CHECK(abcdef.trace == test_trace().production("branch_scan").literal("abc").literal("def"));
    }
    SUBCASE("recursive")
    {
        constexpr auto callback = [](const char*, int value) { return value; };

        auto empty = LEXY_VERIFY_P(recursive_scan, "");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace().expected_literal(0, "x", 0));

        auto zero = LEXY_VERIFY_P(recursive_scan, "x");
        CHECK(zero.status == test_result::success);
        CHECK(zero.value == 0);
        CHECK(zero.trace == test_trace().literal("x"));

        auto one = LEXY_VERIFY_P(recursive_scan, "(x)");
        CHECK(one.status == test_result::success);
        CHECK(one.value == 1);
        CHECK(one.trace
              == test_trace()
                     .literal("(")
                     .production("test_production")
                     .literal("x")
                     .finish()
                     .literal(")"));

        // GCC 8 doesn't want to execute it at compile-time.
        auto two = LEXY_VERIFY_RUNTIME_P(recursive_scan, "((x))");
        CHECK(two.status == test_result::success);
        CHECK(two.value == 2);
        CHECK(two.trace
              == test_trace()
                     .literal("(")
                     .production("test_production")
                     .literal("(")
                     .production("test_production")
                     .literal("x")
                     .finish()
                     .literal(")")
                     .finish()
                     .literal(")"));
    }
    SUBCASE("whitespace")
    {
        constexpr auto callback = token_callback;

        auto empty = LEXY_VERIFY_P(whitespace_scan, "");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace
              == test_trace().production("token").expected_literal(0, "abc", 0).cancel().cancel());

        auto abc = LEXY_VERIFY_P(whitespace_scan, "abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().production("token").literal("abc"));

        auto abc_ws = LEXY_VERIFY_P(whitespace_scan, "--abc++");
        CHECK(abc_ws.status == test_result::success);
        // clang-format off
        CHECK(abc_ws.trace == test_trace()
                .production("token")
                    .whitespace("--")
                    .literal("abc")
                    .finish()
                .whitespace("++"));
        // clang-format on
    }
}

