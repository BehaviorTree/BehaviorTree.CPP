// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/choice.hpp>

#include "verify.hpp"
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/recover.hpp>

namespace
{
template <int Id>
struct label
{
    static constexpr auto id   = Id;
    static constexpr auto name = "label";

    static constexpr auto rule = dsl::try_(LEXY_LIT("!"));
};
} // namespace

TEST_CASE("dsl::operator|")
{
    constexpr auto callback = lexy::callback<int>([](const char*) { return 42; },
                                                  [](const char*, auto p) { return p.id; });

    SUBCASE("simple")
    {
        constexpr auto rule
            = LEXY_LIT("abc") >> dsl::p<label<0>> | LEXY_LIT("def") >> dsl::p<label<1>>;
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted choice").cancel());

        auto abc = LEXY_VERIFY("abc!");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("abc").production("label").literal("!"));

        auto def = LEXY_VERIFY("def!");
        CHECK(def.status == test_result::success);
        CHECK(def.value == 1);
        CHECK(def.trace == test_trace().literal("def").production("label").literal("!"));

        auto branch_error = LEXY_VERIFY("abc");
        CHECK(branch_error.status == test_result::recovered_error);
        CHECK(branch_error.value == 0);
        CHECK(branch_error.trace
              == test_trace().literal("abc").production("label").expected_literal(3, "!", 0));
    }
    SUBCASE("branches are ordered")
    {
        constexpr auto rule
            = LEXY_LIT("a") >> dsl::p<label<0>> | LEXY_LIT("abc") >> dsl::p<label<1>>;
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted choice").cancel());

        auto a = LEXY_VERIFY("a!");
        CHECK(a.status == test_result::success);
        CHECK(a.value == 0);
        CHECK(a.trace == test_trace().literal("a").production("label").literal("!"));

        auto abc = LEXY_VERIFY("abc!");
        CHECK(abc.status == test_result::recovered_error);
        CHECK(abc.value == 0);
        CHECK(abc.trace
              == test_trace().literal("a").production("label").expected_literal(1, "!", 0));
    }
    SUBCASE("with else")
    {
        constexpr auto rule = LEXY_LIT("abc") >> dsl::p<label<0>>   //
                              | LEXY_LIT("def") >> dsl::p<label<1>> //
                              | dsl::else_ >> dsl::p<label<2>>;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::recovered_error);
        CHECK(empty.value == 2);
        CHECK(empty.trace == test_trace().production("label").expected_literal(0, "!", 0));

        auto abc = LEXY_VERIFY("abc!");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("abc").production("label").literal("!"));

        auto def = LEXY_VERIFY("def!");
        CHECK(def.status == test_result::success);
        CHECK(def.value == 1);
        CHECK(def.trace == test_trace().literal("def").production("label").literal("!"));

        auto branch_error = LEXY_VERIFY("abc");
        CHECK(branch_error.status == test_result::recovered_error);
        CHECK(branch_error.value == 0);
        CHECK(branch_error.trace
              == test_trace().literal("abc").production("label").expected_literal(3, "!", 0));
    }
    SUBCASE("with error")
    {
        struct my_error
        {
            static constexpr auto name()
            {
                return "my error";
            }
        };

        constexpr auto rule = LEXY_LIT("abc") >> dsl::p<label<0>>   //
                              | LEXY_LIT("def") >> dsl::p<label<1>> //
                              | dsl::error<my_error>;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto abc = LEXY_VERIFY("abc!");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("abc").production("label").literal("!"));

        auto def = LEXY_VERIFY("def!");
        CHECK(def.status == test_result::success);
        CHECK(def.value == 1);
        CHECK(def.trace == test_trace().literal("def").production("label").literal("!"));

        auto branch_error = LEXY_VERIFY("abc");
        CHECK(branch_error.status == test_result::recovered_error);
        CHECK(branch_error.value == 0);
        CHECK(branch_error.trace
              == test_trace().literal("abc").production("label").expected_literal(3, "!", 0));
    }

    SUBCASE("as branch")
    {
        constexpr auto rule
            = dsl::if_(LEXY_LIT("abc") >> dsl::p<label<0>> | LEXY_LIT("def") >> dsl::p<label<1>>);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 42);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY("abc!");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 0);
        CHECK(abc.trace == test_trace().literal("abc").production("label").literal("!"));

        auto def = LEXY_VERIFY("def!");
        CHECK(def.status == test_result::success);
        CHECK(def.value == 1);
        CHECK(def.trace == test_trace().literal("def").production("label").literal("!"));

        auto branch_error = LEXY_VERIFY("abc");
        CHECK(branch_error.status == test_result::recovered_error);
        CHECK(branch_error.value == 0);
        CHECK(branch_error.trace
              == test_trace().literal("abc").production("label").expected_literal(3, "!", 0));
    }
}

