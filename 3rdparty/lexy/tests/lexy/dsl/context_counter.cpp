// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/context_counter.hpp>

#include "verify.hpp"
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/whitespace.hpp>

namespace
{
struct with_whitespace
{
    static constexpr auto whitespace = LEXY_LIT(".");
};
} // namespace

TEST_CASE("dsl::context_counter")
{
    // Note: runtime checks only here due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89074.
    constexpr auto counter = dsl::context_counter<struct id>;

    constexpr auto callback = lexy::callback<int>([](const char*) { return -11; },
                                                  [](const char*, int value) { return value; });

    SUBCASE(".create()")
    {
        constexpr auto rule = counter.create() + counter.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".create<42>()")
    {
        constexpr auto rule = counter.create<42>() + counter.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 42);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 42);
        CHECK(abc.trace == test_trace());
    }

    SUBCASE(".inc()")
    {
        constexpr auto rule = counter.create<11>() + counter.inc() + counter.inc() + counter.inc()
                              + counter.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 14);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 14);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".dec()")
    {
        constexpr auto rule = counter.create<11>() + counter.dec() + counter.dec() + counter.dec()
                              + counter.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 8);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 8);
        CHECK(abc.trace == test_trace());
    }

    SUBCASE(".push()")
    {
        constexpr auto rule
            = counter.create<11>() + counter.push(LEXY_LIT("abc")) + counter.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 14);
        CHECK(abc.trace == test_trace().literal("abc"));

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto whitespace = LEXY_VERIFY_RUNTIME_P(production, "abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 17);
        CHECK(whitespace.trace == test_trace().literal("abc").whitespace("..."));
    }
    SUBCASE(".pop()")
    {
        constexpr auto rule = counter.create<11>() + counter.pop(LEXY_LIT("abc")) + counter.value();

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 8);
        CHECK(abc.trace == test_trace().literal("abc"));

        struct production : test_production_for<decltype(rule)>, with_whitespace
        {};

        auto whitespace = LEXY_VERIFY_RUNTIME_P(production, "abc...");
        CHECK(whitespace.status == test_result::success);
        CHECK(whitespace.value == 5);
        CHECK(whitespace.trace == test_trace().literal("abc").whitespace("..."));
    }

    SUBCASE(".is<42>() true")
    {
        constexpr auto rule = counter.create<42>() + dsl::if_(counter.is<42>() >> counter.value());

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 42);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 42);
        CHECK(abc.trace == test_trace());
    }
    SUBCASE(".is<42>() false")
    {
        constexpr auto rule = counter.create() + dsl::if_(counter.is<42>() >> counter.value());

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == -11);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == -11);
        CHECK(abc.trace == test_trace());
    }

    SUBCASE(".is_zero()")
    {
        CHECK(equivalent_rules(counter.is_zero(), counter.is<0>()));
    }
}

TEST_CASE("dsl::equal_counts()")
{
    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    struct a_id
    {};
    struct b_id
    {};
    struct c_id
    {};
    constexpr auto setup = [] {
        auto ac = dsl::context_counter<a_id>;
        auto a  = ac.create() + ac.push(dsl::while_(dsl::lit_c<'a'>));

        auto bc = dsl::context_counter<b_id>;
        auto b  = bc.create() + bc.push(dsl::while_(dsl::lit_c<'b'>));

        auto cc = dsl::context_counter<c_id>;
        auto c  = cc.create() + cc.push(dsl::while_(dsl::lit_c<'c'>));

        return a + b + c;
    }();

    constexpr auto equal = dsl::equal_counts(dsl::context_counter<a_id>, dsl::context_counter<b_id>,
                                             dsl::context_counter<c_id>);
    CHECK(lexy::is_branch_rule<decltype(equal)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = setup + equal;

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("a").literal("b").literal("c"));
        auto aabbcc = LEXY_VERIFY_RUNTIME("aabbcc");
        CHECK(aabbcc.status == test_result::success);
        CHECK(aabbcc.trace
              == test_trace()
                     .literal("a")
                     .literal("a")
                     .literal("b")
                     .literal("b")
                     .literal("c")
                     .literal("c"));

        auto aabcc = LEXY_VERIFY_RUNTIME("aabcc");
        CHECK(aabcc.status == test_result::recovered_error);
        CHECK(aabcc.trace
              == test_trace()
                     .literal("a")
                     .literal("a")
                     .literal("b")
                     .literal("c")
                     .literal("c")
                     .error(5, 5, "unequal counts"));
        auto aabbccc = LEXY_VERIFY_RUNTIME("aabbccc");
        CHECK(aabbccc.status == test_result::recovered_error);
        CHECK(aabbccc.trace
              == test_trace()
                     .literal("a")
                     .literal("a")
                     .literal("b")
                     .literal("b")
                     .literal("c")
                     .literal("c")
                     .literal("c")
                     .error(7, 7, "unequal counts"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = setup + dsl::must(equal).error<my_error>;

        auto empty = LEXY_VERIFY_RUNTIME("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY_RUNTIME("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal("a").literal("b").literal("c"));
        auto aabbcc = LEXY_VERIFY_RUNTIME("aabbcc");
        CHECK(aabbcc.status == test_result::success);
        CHECK(aabbcc.trace
              == test_trace()
                     .literal("a")
                     .literal("a")
                     .literal("b")
                     .literal("b")
                     .literal("c")
                     .literal("c"));

        auto aabcc = LEXY_VERIFY_RUNTIME("aabcc");
        CHECK(aabcc.status == test_result::fatal_error);
        CHECK(aabcc.trace
              == test_trace()
                     .literal("a")
                     .literal("a")
                     .literal("b")
                     .literal("c")
                     .literal("c")
                     .error(5, 5, "my error")
                     .cancel());
        auto aabbccc = LEXY_VERIFY_RUNTIME("aabbccc");
        CHECK(aabbccc.status == test_result::fatal_error);
        CHECK(aabbccc.trace
              == test_trace()
                     .literal("a")
                     .literal("a")
                     .literal("b")
                     .literal("b")
                     .literal("c")
                     .literal("c")
                     .literal("c")
                     .error(7, 7, "my error")
                     .cancel());
    }
}

