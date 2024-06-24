// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/action/scan.hpp>

#include <doctest/doctest.h>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/fold.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/whitespace.hpp>
#include <lexy/input/string_input.hpp>

namespace
{
struct production
{
    static constexpr auto rule = lexy::dsl::capture(LEXY_LIT("abc"));
    static constexpr auto value
        = lexy::callback<int>([](auto lex) { return static_cast<int>(lex.size()); });
};

struct control_production
{
    static constexpr auto whitespace = LEXY_LIT(" ");
};
} // namespace

TEST_CASE("lexy::scan")
{
    auto check_position = [](const auto& scanner, bool eof, auto pos) {
        CHECK(scanner.is_at_eof() == eof);
        CHECK(scanner.position() == pos);
        CHECK(scanner.remaining_input().reader().position() == pos);
    };

    SUBCASE("empty input")
    {
        auto input   = lexy::string_input<lexy::default_encoding>();
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, true, input.data());

        scanner.parse(lexy::dsl::eof);
        CHECK(scanner);
        check_position(scanner, true, input.data());

        scanner.parse(LEXY_LIT("abc"));
        CHECK(!scanner);
        check_position(scanner, true, input.data());
    }

    SUBCASE("parse w/o value")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.parse(LEXY_LIT("abc"));
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);

        scanner.parse(lexy::dsl::eof);
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);

        scanner.parse(LEXY_LIT("abc"));
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 3);
    }
    SUBCASE("parse with value")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto first = scanner.parse<lexy::string_lexeme<>>(lexy::dsl::capture(LEXY_LIT("abc")));
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(first);
        CHECK(first.value().begin() == input.data());
        CHECK(first.value().end() == input.data() + 3);

        auto second = scanner.parse<lexy::string_lexeme<>>(lexy::dsl::capture(LEXY_LIT("abc")));
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(!second);
    }
    SUBCASE("parse production")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto first = scanner.parse(production{});
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(first);
        CHECK(first.value() == 3);

        auto second = scanner.parse<production>();
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(!second);
    }

    SUBCASE("branch w/o value")
    {
        auto input   = lexy::zstring_input("abcdefa");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto first = scanner.branch(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(scanner);
        check_position(scanner, false, input.data() + 3);
        CHECK(first);

        auto second = scanner.branch(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(scanner);
        check_position(scanner, false, input.data() + 3);
        CHECK(!second);

        auto third = scanner.branch(LEXY_LIT("d") >> LEXY_LIT("ef"));
        CHECK(scanner);
        check_position(scanner, false, input.data() + 6);
        CHECK(third);

        auto fourth = scanner.branch(LEXY_LIT("a") >> LEXY_LIT("bc"));
        CHECK(!scanner);
        check_position(scanner, true, input.data() + 7);
        CHECK(fourth);
    }
    SUBCASE("branch with value")
    {
        auto input   = lexy::zstring_input("abcdefa");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, LEXY_LIT("a") >> lexy::dsl::position + LEXY_LIT("bc"));
            CHECK(scanner);
            check_position(scanner, false, input.data() + 3);
            CHECK(taken);
            CHECK(result);
            CHECK(result.value() == input.data() + 1);
        }
        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, LEXY_LIT("a") >> lexy::dsl::position + LEXY_LIT("bc"));
            CHECK(scanner);
            check_position(scanner, false, input.data() + 3);
            CHECK(!taken);
            CHECK(!result);
        }
        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, LEXY_LIT("d") >> lexy::dsl::position + LEXY_LIT("ef"));
            CHECK(scanner);
            check_position(scanner, false, input.data() + 6);
            CHECK(taken);
            CHECK(result);
            CHECK(result.value() == input.data() + 4);
        }
        {
            lexy::scan_result<const char*> result;

            auto taken
                = scanner.branch(result, LEXY_LIT("a") >> lexy::dsl::position + LEXY_LIT("bc"));
            CHECK(!scanner);
            check_position(scanner, true, input.data() + 7);
            CHECK(taken);
            CHECK(!result);
        }
    }
    SUBCASE("branch production")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        {
            lexy::scan_result<int> result;

            auto taken = scanner.branch(result, lexy::dsl::p<production>);
            CHECK(scanner);
            check_position(scanner, true, input.data() + 3);
            CHECK(taken);
            CHECK(result);
            CHECK(result.value() == 3);
        }
        {
            lexy::scan_result<int> result;

            auto taken = scanner.branch<production>(result);
            CHECK(scanner);
            check_position(scanner, true, input.data() + 3);
            CHECK(!taken);
            CHECK(!result);
        }
    }

    SUBCASE("error recovery")
    {
        auto input   = lexy::zstring_input("123-abc");
        auto scanner = lexy::scan(input, lexy::noop);
        scanner.parse(LEXY_LIT("abc"));
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        // Parsing is a no-op in failed state.
        scanner.parse(LEXY_LIT("123"));
        CHECK(!scanner);
        check_position(scanner, false, input.data());
        // Branch parsing is a no-op in failed state.
        auto taken = scanner.branch(LEXY_LIT("123"));
        CHECK(!scanner);
        check_position(scanner, false, input.data());
        CHECK(!taken);

        auto recovery = scanner.error_recovery();

        // Parsing does something in recovery.
        scanner.parse(LEXY_LIT("123"));
        CHECK(!scanner);
        check_position(scanner, false, input.data() + 3);
        // Branch parsing does something in recovery.
        taken = scanner.branch(LEXY_LIT("-"));
        CHECK(!scanner);
        check_position(scanner, false, input.data() + 4);
        CHECK(taken);

        SUBCASE("finish")
        {
            LEXY_MOV(recovery).finish();
            CHECK(scanner);
            check_position(scanner, false, input.data() + 4);

            scanner.parse(LEXY_LIT("abc"));
            CHECK(scanner);
            check_position(scanner, true, input.data() + 7);
        }
        SUBCASE("cancel")
        {
            LEXY_MOV(recovery).cancel();
            CHECK(!scanner);
            check_position(scanner, false, input.data() + 4);

            scanner.parse(LEXY_LIT("abc"));
            CHECK(!scanner);
            check_position(scanner, false, input.data() + 4);

            auto taken = scanner.branch(LEXY_LIT("abc"));
            CHECK(!scanner);
            check_position(scanner, false, input.data() + 4);
            CHECK(!taken);
        }
    }
    SUBCASE("discard")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto result = scanner.discard(LEXY_LIT("abcd"));
        CHECK(scanner);
        check_position(scanner, false, input.data());
        CHECK(!result);

        result = scanner.discard(LEXY_LIT("abc"));
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(result);

        result = scanner.discard(LEXY_LIT("abc"));
        CHECK(scanner);
        check_position(scanner, true, input.data() + 3);
        CHECK(!result);
    }
    SUBCASE("error")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::count);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.error(lexy::expected_char_class{}, scanner.position(), "foo");
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.parse(LEXY_LIT("123"));
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        scanner.error(lexy::expected_char_class{}, scanner.position(), "foo");
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        auto result = LEXY_MOV(scanner).finish();
        CHECK(result.error_count() == 3);
    }
    SUBCASE("fatal error")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::count);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.fatal_error(lexy::expected_char_class{}, scanner.position(), "foo");
        CHECK(!scanner);
        check_position(scanner, false, input.data());

        auto result = LEXY_MOV(scanner).finish();
        CHECK(result.error_count() == 1);
    }

    SUBCASE("peek")
    {
        auto input   = lexy::zstring_input("abc");
        auto scanner = lexy::scan(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        auto result = scanner.peek(LEXY_LIT("abc"));
        CHECK(scanner);
        check_position(scanner, false, input.data());
        CHECK(result);

        result = scanner.peek(LEXY_LIT("123"));
        CHECK(scanner);
        check_position(scanner, false, input.data());
        CHECK(!result);
    }

    SUBCASE("control production")
    {
        auto input   = lexy::zstring_input("abc abc");
        auto scanner = lexy::scan<control_production>(input, lexy::noop);
        CHECK(scanner);
        check_position(scanner, false, input.data());

        scanner.parse(LEXY_LIT("abc"));
        CHECK(scanner);
        check_position(scanner, false, input.data() + 4);

        scanner.parse(LEXY_LIT("abc"));
        CHECK(scanner);
        check_position(scanner, true, input.data() + 7);
    }
}

