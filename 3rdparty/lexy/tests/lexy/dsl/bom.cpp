// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/bom.hpp>

#include "verify.hpp"

TEST_CASE("dsl::bom")
{
    constexpr auto callback = token_callback;

    SUBCASE("no bom")
    {
        constexpr auto rule = dsl::bom<lexy::byte_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace().literal(""));

        auto abc = LEXY_VERIFY("abc");
        CHECK(abc.status == test_result::success);
        CHECK(abc.trace == test_trace().literal(""));
    }

    SUBCASE("UTF-8")
    {
        constexpr auto rule = dsl::bom<lexy::utf8_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, R"(\EF\BB\BF)", 0).cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xEF, 0xBB, 0xBF);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(\EF\BB\BF)"));
    }

    SUBCASE("UTF-16, little")
    {
        constexpr auto rule = dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, R"(\FF\FE)", 0).cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xFF, 0xFE);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(\FF\FE)"));
    }
    SUBCASE("UTF-16, big")
    {
        constexpr auto rule = dsl::bom<lexy::utf16_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, R"(\FE\FF)", 0).cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xFE, 0xFF);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(\FE\FF)"));
    }

    SUBCASE("UTF-32, little")
    {
        constexpr auto rule = dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::little>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, R"(\FF\FE\00\00)", 0).cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0xFF, 0xFE, 0x00, 0x00);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(\FF\FE\00\00)"));
    }
    SUBCASE("UTF-32, big")
    {
        constexpr auto rule = dsl::bom<lexy::utf32_encoding, lexy::encoding_endianness::big>;
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_literal(0, R"(\00\00\FE\FF)", 0).cancel());

        auto bom = LEXY_VERIFY(lexy::byte_encoding{}, 0x00, 0x00, 0xFE, 0xFF);
        CHECK(bom.status == test_result::success);
        CHECK(bom.trace == test_trace().literal(R"(\00\00\FE\FF)"));
    }
}

