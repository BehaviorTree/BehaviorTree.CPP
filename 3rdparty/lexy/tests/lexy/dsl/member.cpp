// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/member.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/position.hpp>

namespace
{
template <typename Fn, typename Obj, typename T>
void apply(lexy::member<Fn>, Obj& obj, T t)
{
    Fn()(obj, t);
}
} // namespace

TEST_CASE("dsl::member")
{
    struct test_type
    {
        int member;
    };

    constexpr auto callback = lexy::callback<int>([](const char*) { return 0; },
                                                  [](const char*, auto member, const char*) {
                                                      test_type test{};
                                                      apply(member, test, 42);
                                                      CHECK(test.member == 42);

                                                      return 1;
                                                  });

    SUBCASE("non-macro")
    {
        constexpr auto rule = (dsl::member<& test_type::member> = LEXY_LIT("abc")) + dsl::position;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc").position());
    }
    SUBCASE("macro")
    {
        // Note: not constexpr in C++17 due to the use of reinterpret_cast in stateless lambda.
        constexpr auto member = LEXY_MEM(member) = LEXY_LIT("abc");
        constexpr auto rule                      = member + dsl::position;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = verify(rule, lexy::zstring_input(""), callback);
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, "abc", 0).cancel());

        auto abc = verify(rule, lexy::zstring_input("abc"), callback);
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc").position());
    }

    SUBCASE("as branch")
    {
        constexpr auto rule
            = dsl::if_(dsl::member<& test_type::member> = LEXY_LIT("abc") >> dsl::position);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 1);
        CHECK(abc.trace == test_trace().literal("abc").position());
    }
}

