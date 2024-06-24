// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/effect.hpp>

#include "verify.hpp"

namespace
{
bool called = false;

void fn_no_void()
{
    called = true;
}
int fn_no_int()
{
    called = true;
    return 0;
}

constexpr auto fn_state_void = [](auto&) { called = true; };
constexpr auto fn_state_int  = [](auto&) {
    called = true;
    return 0;
};
} // namespace

TEST_CASE("dsl::effect")
{
    SUBCASE("no parse state, void")
    {
        constexpr auto rule = dsl::effect<fn_no_void>;
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        called     = false;
        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());
        CHECK(called == true);

        called   = false;
        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace());
        CHECK(called == true);
    }
    SUBCASE("parse state, void")
    {
        constexpr auto rule = dsl::effect<fn_state_void>;
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = token_callback;

        called     = false;
        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());
        CHECK(called == true);

        called   = false;
        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace());
        CHECK(called == true);
    }
    SUBCASE("no parse state, non-void")
    {
        constexpr auto rule = dsl::effect<fn_no_int>;
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = [](const char*, int i) { return i; };

        called     = false;
        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());
        CHECK(called == true);

        called   = false;
        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
        CHECK(called == true);
    }
    SUBCASE("parse state, non-void")
    {
        constexpr auto rule = dsl::effect<fn_state_int>;
        CHECK(lexy::is_rule<decltype(rule)>);

        constexpr auto callback = [](const char*, int i) { return i; };

        called     = false;
        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());
        CHECK(called == true);

        called   = false;
        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
        CHECK(called == true);
    }
}

