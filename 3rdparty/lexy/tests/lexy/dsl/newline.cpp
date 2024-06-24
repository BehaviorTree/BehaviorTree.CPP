// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/newline.hpp>

#include "verify.hpp"
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::newline")
{
    constexpr auto rule = dsl::newline;
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(lexy::is_literal_set_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY("");
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().error(0, 0, "expected newline").cancel());

    auto abc = LEXY_VERIFY("abc");
    CHECK(abc.status == test_result::fatal_error);
    CHECK(abc.trace == test_trace().error(0, 0, "expected newline").cancel());

    auto n = LEXY_VERIFY("\n");
    CHECK(n.status == test_result::success);
    CHECK(n.trace == test_trace().literal("\\n"));
    auto rn = LEXY_VERIFY("\r\n");
    CHECK(rn.status == test_result::success);
    CHECK(rn.trace == test_trace().literal("\\r\\n"));

    auto r = LEXY_VERIFY("\r");
    CHECK(r.status == test_result::fatal_error);
    CHECK(r.trace == test_trace().error(0, 0, "expected newline").cancel());

    auto nr = LEXY_VERIFY("\n\r");
    CHECK(nr.status == test_result::success);
    CHECK(nr.trace == test_trace().literal("\\n"));
    auto nn = LEXY_VERIFY("\n\n");
    CHECK(nn.status == test_result::success);
    CHECK(nn.trace == test_trace().literal("\\n"));
    auto nrn = LEXY_VERIFY("\n\r\n");
    CHECK(nrn.status == test_result::success);
    CHECK(nrn.trace == test_trace().literal("\\n"));

    auto utf16 = LEXY_VERIFY(u"\r\n");
    CHECK(utf16.status == test_result::success);
    CHECK(utf16.trace == test_trace().literal("\\r\\n"));
}

TEST_CASE("dsl::eol")
{
    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = dsl::eol;
        CHECK(lexy::is_branch_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace().eof());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::fatal_error);
        CHECK(abc.trace == test_trace().error(0, 0, "expected newline").cancel());

        auto n = LEXY_VERIFY("\n");
        CHECK(n.status == test_result::success);
        CHECK(n.trace == test_trace().literal("\\n"));
        auto rn = LEXY_VERIFY("\r\n");
        CHECK(rn.status == test_result::success);
        CHECK(rn.trace == test_trace().literal("\\r\\n"));

        auto r = LEXY_VERIFY("\r");
        CHECK(r.status == test_result::fatal_error);
        CHECK(r.trace == test_trace().error(0, 0, "expected newline").cancel());

        auto nr = LEXY_VERIFY("\n\r");
        CHECK(nr.status == test_result::success);
        CHECK(nr.trace == test_trace().literal("\\n"));
        auto nn = LEXY_VERIFY("\n\n");
        CHECK(nn.status == test_result::success);
        CHECK(nn.trace == test_trace().literal("\\n"));
        auto nrn = LEXY_VERIFY("\n\r\n");
        CHECK(nrn.status == test_result::success);
        CHECK(nrn.trace == test_trace().literal("\\n"));

        auto utf16 = LEXY_VERIFY(u"\r\n");
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().literal("\\r\\n"));
    }
    SUBCASE("as branch")
    {
        constexpr auto rule = dsl::if_(dsl::eol);
        CHECK(lexy::is_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY("");
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace().eof());

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace());

        auto n = LEXY_VERIFY("\n");
        CHECK(n.status == test_result::success);
        CHECK(n.trace == test_trace().literal("\\n"));
        auto rn = LEXY_VERIFY("\r\n");
        CHECK(rn.status == test_result::success);
        CHECK(rn.trace == test_trace().literal("\\r\\n"));

        auto r = LEXY_VERIFY("\r");
        CHECK(r.status == test_result::success);
        CHECK(r.trace == test_trace());

        auto nr = LEXY_VERIFY("\n\r");
        CHECK(nr.status == test_result::success);
        CHECK(nr.trace == test_trace().literal("\\n"));
        auto nn = LEXY_VERIFY("\n\n");
        CHECK(nn.status == test_result::success);
        CHECK(nn.trace == test_trace().literal("\\n"));
        auto nrn = LEXY_VERIFY("\n\r\n");
        CHECK(nrn.status == test_result::success);
        CHECK(nrn.trace == test_trace().literal("\\n"));

        auto utf16 = LEXY_VERIFY(u"\r\n");
        CHECK(utf16.status == test_result::success);
        CHECK(utf16.trace == test_trace().literal("\\r\\n"));
    }
}

