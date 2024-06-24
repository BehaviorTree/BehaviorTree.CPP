// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/context_flag.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::context_flag")
{
    // Note: runtime checks only here due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89074.
    constexpr auto flag = dsl::context_flag<struct id>;

    constexpr auto callback = lexy::callback<int>([](const char*) { return 2; },
                                                  [](const char*, bool value) { return value; });

    SUBCASE(".create()")
    {
        constexpr auto rule = flag.create() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".create<true>()")
    {
        constexpr auto rule = flag.create<true>() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 1);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace());
    }

    SUBCASE(".set() false flag")
    {
        constexpr auto rule = flag.create() + flag.set() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 1);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".set() true flag")
    {
        constexpr auto rule = flag.create<true>() + flag.set() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 1);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".reset() false flag")
    {
        constexpr auto rule = flag.create() + flag.reset() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".reset() true flag")
    {
        constexpr auto rule = flag.create<true>() + flag.reset() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".toggle() false flag")
    {
        constexpr auto rule = flag.create() + flag.toggle() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 1);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".toggle() true flag")
    {
        constexpr auto rule = flag.create<true>() + flag.toggle() + flag.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
    }

    SUBCASE(".is_set() false flag")
    {
        constexpr auto rule = flag.create() + dsl::if_(flag.is_set() >> flag.value());

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 2);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 2);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".is_set() true flag")
    {
        constexpr auto rule = flag.create<true>() + dsl::if_(flag.is_set() >> flag.value());

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 1);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".is_reset() false flag")
    {
        constexpr auto rule = flag.create() + dsl::if_(flag.is_reset() >> flag.value());

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".is_reset() true flag")
    {
        constexpr auto rule = flag.create<true>() + dsl::if_(flag.is_reset() >> flag.value());

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 2);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 2);
        CHECK(abc.trace == test_trace());
    }
}

