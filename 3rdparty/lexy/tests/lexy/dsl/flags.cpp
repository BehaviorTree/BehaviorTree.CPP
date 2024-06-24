// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/flags.hpp>

#include "verify.hpp"
#include <lexy/dsl/symbol.hpp>

namespace
{
struct my_error
{
    static constexpr auto name()
    {
        return "my error";
    }
};

enum class flags
{
    none = 0,
    a    = 1 << 0,
    b    = 1 << 1,
    c    = 1 << 2,
};

constexpr auto flag_symbols
    = lexy::symbol_table<flags>.map<'a'>(flags::a).map<'b'>(flags::b).map<'c'>(flags::c);
} // namespace

TEST_CASE("dsl::flags")
{
    constexpr auto rule = dsl::flags(dsl::symbol<flag_symbols>);
    CHECK(equivalent_rules(rule, dsl::flags<flags::none>(dsl::symbol<flag_symbols>)));

    constexpr auto callback = [](const char*, flags value) { return int(value); };

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::success);
    CHECK(empty.value == int(flags::none));
    CHECK(empty.trace == test_trace());

    auto a = LEXY_VERIFY("a");
    CHECK(a.status == test_result::success);
    CHECK(a.value == int(flags::a));
    CHECK(a.trace == test_trace().token("identifier", "a"));
    auto b = LEXY_VERIFY("b");
    CHECK(b.status == test_result::success);
    CHECK(b.value == int(flags::b));
    CHECK(b.trace == test_trace().token("identifier", "b"));
    auto c = LEXY_VERIFY("c");
    CHECK(c.status == test_result::success);
    CHECK(c.value == int(flags::c));
    CHECK(c.trace == test_trace().token("identifier", "c"));

    auto ab = LEXY_VERIFY("ab");
    CHECK(ab.status == test_result::success);
    CHECK(ab.value == (int(flags::a) | int(flags::b)));
    CHECK(ab.trace == test_trace().token("identifier", "a").token("identifier", "b"));
    auto cb = LEXY_VERIFY("cb");
    CHECK(cb.status == test_result::success);
    CHECK(cb.value == (int(flags::c) | int(flags::b)));
    CHECK(cb.trace == test_trace().token("identifier", "c").token("identifier", "b"));
    auto cab = LEXY_VERIFY("cab");
    CHECK(cab.status == test_result::success);
    CHECK(cab.value == (int(flags::c) | int(flags::a) | int(flags::b)));
    CHECK(
        cab.trace
        == test_trace().token("identifier", "c").token("identifier", "a").token("identifier", "b"));

    auto aba = LEXY_VERIFY("aba");
    CHECK(aba.status == test_result::recovered_error);
    CHECK(aba.value == (int(flags::a) | int(flags::b)));
    CHECK(aba.trace
          == test_trace()
                 .token("identifier", "a")
                 .token("identifier", "b")
                 .token("identifier", "a")
                 .error(2, 3, "duplicate flag"));

    SUBCASE(".duplicate_error")
    {
        constexpr auto rule = dsl::flags(dsl::symbol<flag_symbols>).error<my_error>;

        auto aba = LEXY_VERIFY("aba");
        CHECK(aba.status == test_result::recovered_error);
        CHECK(aba.value == (int(flags::a) | int(flags::b)));
        CHECK(aba.trace
              == test_trace()
                     .token("identifier", "a")
                     .token("identifier", "b")
                     .token("identifier", "a")
                     .error(2, 3, "my error"));
    }
}

TEST_CASE("dsl::flag")
{
    SUBCASE("explicit value")
    {
        constexpr auto rule = dsl::flag<flags::a>(LEXY_LIT("a"));
        CHECK(equivalent_rules(rule, dsl::flag<flags::a, flags::none>(LEXY_LIT("a"))));

        constexpr auto callback = [](const char*, flags value) { return int(value); };

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == int(flags::none));
        CHECK(empty.trace == test_trace());

        auto a = LEXY_VERIFY("a");
        CHECK(a.status == test_result::success);
        CHECK(a.value == int(flags::a));
        CHECK(a.trace == test_trace().literal("a"));

        auto aa = LEXY_VERIFY("aa");
        CHECK(aa.status == test_result::success);
        CHECK(aa.value == int(flags::a));
        CHECK(aa.trace == test_trace().literal("a"));
    }
    SUBCASE("boolean")
    {
        // This has to be after the above version, otherwise GCC 7 gets really confused about
        // overload resolution or something.
        constexpr auto rule = dsl::flag(LEXY_LIT("a"));
        CHECK(equivalent_rules(rule, dsl::flag<true, false>(LEXY_LIT("a"))));
    }
}

