// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/combination.hpp>

#include "verify.hpp"
#include <lexy/dsl/position.hpp>

TEST_CASE("dsl::combination()")
{
    constexpr auto callback = [](const char*, std::size_t n) { return static_cast<int>(n); };

    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    auto make_trace = [](const char* first, const char* second, const char* third) {
        return test_trace()
            .literal(first)
            .literal(first)
            .position()
            .literal(second)
            .literal(second)
            .position()
            .literal(third)
            .literal(third)
            .position();
    };

    SUBCASE("default")
    {
        constexpr auto rule = dsl::combination(LEXY_LIT("a") >> LEXY_LIT("a") + dsl::position,
                                               LEXY_LIT("b") >> LEXY_LIT("b") + dsl::position,
                                               LEXY_LIT("c") >> LEXY_LIT("c") + dsl::position);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted choice").cancel());

        auto abc = LEXY_VERIFY("aabbcc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 3);
        CHECK(abc.trace == make_trace("a", "b", "c"));
        auto acb = LEXY_VERIFY("aaccbb");
        CHECK(acb.status == test_result::success);
        CHECK(acb.value == 3);
        CHECK(acb.trace == make_trace("a", "c", "b"));
        auto bac = LEXY_VERIFY("bbaacc");
        CHECK(bac.status == test_result::success);
        CHECK(bac.value == 3);
        CHECK(bac.trace == make_trace("b", "a", "c"));
        auto bca = LEXY_VERIFY("bbccaa");
        CHECK(bca.status == test_result::success);
        CHECK(bca.value == 3);
        CHECK(bca.trace == make_trace("b", "c", "a"));
        auto cab = LEXY_VERIFY("ccaabb");
        CHECK(cab.status == test_result::success);
        CHECK(cab.value == 3);
        CHECK(cab.trace == make_trace("c", "a", "b"));
        auto cba = LEXY_VERIFY("ccbbaa");
        CHECK(cba.status == test_result::success);
        CHECK(cba.value == 3);
        CHECK(cba.trace == make_trace("c", "b", "a"));

        auto abca = LEXY_VERIFY("aabbccaa");
        CHECK(abca.status == test_result::success);
        CHECK(abca.value == 3);
        CHECK(abca.trace == make_trace("a", "b", "c"));

        auto branch_error = LEXY_VERIFY("abbcc");
        CHECK(branch_error.status == test_result::fatal_error);
        CHECK(branch_error.trace == test_trace().literal("a").expected_literal(1, "a", 0).cancel());

        auto ab       = LEXY_VERIFY("aabb");
        auto ab_trace = test_trace()
                            .literal("a")
                            .literal("a")
                            .position()
                            .literal("b")
                            .literal("b")
                            .position()
                            .error(4, 4, "exhausted choice")
                            .cancel();
        CHECK(ab.status == test_result::fatal_error);
        CHECK(ab.trace == ab_trace);

        auto aabc       = LEXY_VERIFY("aaaabbcc");
        auto aabc_trace = test_trace()
                              .literal("a")
                              .literal("a")
                              .position()
                              .literal("a")
                              .literal("a")
                              .position()
                              .error(2, 4, "combination duplicate")
                              .literal("b")
                              .literal("b")
                              .position()
                              .literal("c")
                              .literal("c")
                              .position();
        CHECK(aabc.status == test_result::recovered_error);
        CHECK(aabc.value == 3);
        CHECK(aabc.trace == aabc_trace);
        auto abac       = LEXY_VERIFY("aabbaacc");
        auto abac_trace = test_trace()
                              .literal("a")
                              .literal("a")
                              .position()
                              .literal("b")
                              .literal("b")
                              .position()
                              .literal("a")
                              .literal("a")
                              .position()
                              .error(4, 6, "combination duplicate")
                              .literal("c")
                              .literal("c")
                              .position();
        CHECK(abac.status == test_result::recovered_error);
        CHECK(abac.value == 3);
        CHECK(abac.trace == abac_trace);
    }
    SUBCASE("missing_error")
    {
        constexpr auto rule = dsl::combination(LEXY_LIT("a") >> LEXY_LIT("a") + dsl::position,
                                               LEXY_LIT("b") >> LEXY_LIT("b") + dsl::position,
                                               LEXY_LIT("c") >> LEXY_LIT("c") + dsl::position)
                                  .missing_error<my_error>;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "my error").cancel());

        auto abc = LEXY_VERIFY("aabbcc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 3);
        CHECK(abc.trace == make_trace("a", "b", "c"));
    }
    SUBCASE("duplicate_error")
    {
        constexpr auto rule = dsl::combination(LEXY_LIT("a") >> LEXY_LIT("a") + dsl::position,
                                               LEXY_LIT("b") >> LEXY_LIT("b") + dsl::position,
                                               LEXY_LIT("c") >> LEXY_LIT("c") + dsl::position)
                                  .duplicate_error<my_error>;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().error(0, 0, "exhausted choice").cancel());

        auto abc = LEXY_VERIFY("aabbcc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 3);
        CHECK(abc.trace == make_trace("a", "b", "c"));

        auto aabc       = LEXY_VERIFY("aaaabbcc");
        auto aabc_trace = test_trace()
                              .literal("a")
                              .literal("a")
                              .position()
                              .literal("a")
                              .literal("a")
                              .position()
                              .error(2, 4, "my error")
                              .literal("b")
                              .literal("b")
                              .position()
                              .literal("c")
                              .literal("c")
                              .position();
        CHECK(aabc.status == test_result::recovered_error);
        CHECK(aabc.value == 3);
        CHECK(aabc.trace == aabc_trace);
    }
}

TEST_CASE("dsl::partial_combination()")
{
    constexpr auto callback = [](const char*, std::size_t n) { return static_cast<int>(n); };

    struct my_error
    {
        static constexpr auto name()
        {
            return "my error";
        }
    };

    auto make_trace = [](const char* first, const char* second, const char* third) {
        return test_trace()
            .literal(first)
            .literal(first)
            .position()
            .literal(second)
            .literal(second)
            .position()
            .literal(third)
            .literal(third)
            .position();
    };

    SUBCASE("default")
    {
        constexpr auto rule
            = dsl::partial_combination(LEXY_LIT("a") >> LEXY_LIT("a") + dsl::position,
                                       LEXY_LIT("b") >> LEXY_LIT("b") + dsl::position,
                                       LEXY_LIT("c") >> LEXY_LIT("c") + dsl::position);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY("aabbcc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 3);
        CHECK(abc.trace == make_trace("a", "b", "c"));
        auto acb = LEXY_VERIFY("aaccbb");
        CHECK(acb.status == test_result::success);
        CHECK(acb.value == 3);
        CHECK(acb.trace == make_trace("a", "c", "b"));
        auto bac = LEXY_VERIFY("bbaacc");
        CHECK(bac.status == test_result::success);
        CHECK(bac.value == 3);
        CHECK(bac.trace == make_trace("b", "a", "c"));
        auto bca = LEXY_VERIFY("bbccaa");
        CHECK(bca.status == test_result::success);
        CHECK(bca.value == 3);
        CHECK(bca.trace == make_trace("b", "c", "a"));
        auto cab = LEXY_VERIFY("ccaabb");
        CHECK(cab.status == test_result::success);
        CHECK(cab.value == 3);
        CHECK(cab.trace == make_trace("c", "a", "b"));
        auto cba = LEXY_VERIFY("ccbbaa");
        CHECK(cba.status == test_result::success);
        CHECK(cba.value == 3);
        CHECK(cba.trace == make_trace("c", "b", "a"));

        auto abca = LEXY_VERIFY("aabbccaa");
        CHECK(abca.status == test_result::success);
        CHECK(abca.value == 3);
        CHECK(abca.trace == make_trace("a", "b", "c"));

        auto branch_error = LEXY_VERIFY("abbcc");
        CHECK(branch_error.status == test_result::fatal_error);
        CHECK(branch_error.trace == test_trace().literal("a").expected_literal(1, "a", 0).cancel());

        auto ab       = LEXY_VERIFY("aabb");
        auto ab_trace = test_trace()
                            .literal("a")
                            .literal("a")
                            .position()
                            .literal("b")
                            .literal("b")
                            .position();
        CHECK(ab.status == test_result::success);
        CHECK(ab.value == 2);
        CHECK(ab.trace == ab_trace);

        auto aabc       = LEXY_VERIFY("aaaabbcc");
        auto aabc_trace = test_trace()
                              .literal("a")
                              .literal("a")
                              .position()
                              .literal("a")
                              .literal("a")
                              .position()
                              .error(2, 4, "combination duplicate")
                              .literal("b")
                              .literal("b")
                              .position()
                              .literal("c")
                              .literal("c")
                              .position();
        CHECK(aabc.status == test_result::recovered_error);
        CHECK(aabc.value == 3);
        CHECK(aabc.trace == aabc_trace);
        auto abac       = LEXY_VERIFY("aabbaacc");
        auto abac_trace = test_trace()
                              .literal("a")
                              .literal("a")
                              .position()
                              .literal("b")
                              .literal("b")
                              .position()
                              .literal("a")
                              .literal("a")
                              .position()
                              .error(4, 6, "combination duplicate")
                              .literal("c")
                              .literal("c")
                              .position();
        CHECK(abac.status == test_result::recovered_error);
        CHECK(abac.value == 3);
        CHECK(abac.trace == abac_trace);
    }
    SUBCASE("duplicate_error")
    {
        constexpr auto rule
            = dsl::partial_combination(LEXY_LIT("a") >> LEXY_LIT("a") + dsl::position,
                                       LEXY_LIT("b") >> LEXY_LIT("b") + dsl::position,
                                       LEXY_LIT("c") >> LEXY_LIT("c") + dsl::position)
                  .duplicate_error<my_error>;
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.value == 0);
        CHECK(empty.trace == test_trace());

        auto abc = LEXY_VERIFY("aabbcc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.value == 3);
        CHECK(abc.trace == make_trace("a", "b", "c"));

        auto aabc       = LEXY_VERIFY("aaaabbcc");
        auto aabc_trace = test_trace()
                              .literal("a")
                              .literal("a")
                              .position()
                              .literal("a")
                              .literal("a")
                              .position()
                              .error(2, 4, "my error")
                              .literal("b")
                              .literal("b")
                              .position()
                              .literal("c")
                              .literal("c")
                              .position();
        CHECK(aabc.status == test_result::recovered_error);
        CHECK(aabc.value == 3);
        CHECK(aabc.trace == aabc_trace);
    }
}

