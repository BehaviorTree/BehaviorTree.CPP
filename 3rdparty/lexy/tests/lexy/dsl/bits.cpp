// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/bits.hpp>

#include "verify.hpp"

TEST_CASE("dsl::bits")
{
    constexpr auto callback = token_callback;

    auto verify = [&](auto rule, int byte, bool expected) {
        auto result = LEXY_VERIFY_RUNTIME(lexy::byte_encoding{}, byte);

        if (expected)
        {
            char format[16];
            std::sprintf(format, "\\%02X", static_cast<unsigned char>(byte));

            CHECK(result.status == test_result::success);
            CHECK(result.trace == test_trace().token(format));
        }
        else
        {
            CHECK(result.status == test_result::fatal_error);
            CHECK(result.trace == test_trace().expected_char_class(0, "bits").cancel());
        }
    };

    SUBCASE("MSB one")
    {
        constexpr auto rule = dsl::bits(dsl::bit::_1, dsl::bit::any<7>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "bits").cancel());

        for (auto byte = 0; byte < 256; ++byte)
            verify(rule, byte, (byte & 0b1000'0000) != 0);
    }
    SUBCASE("MSB zero")
    {
        constexpr auto rule = dsl::bits(dsl::bit::_0, dsl::bit::any<7>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "bits").cancel());

        for (auto byte = 0; byte < 256; ++byte)
            verify(rule, byte, (byte & 0b1000'0000) == 0);
    }
    SUBCASE("MSB any")
    {
        constexpr auto rule = dsl::bits(dsl::bit::_, dsl::bit::any<7>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "bits").cancel());

        for (auto byte = 0; byte < 256; ++byte)
            verify(rule, byte, true);
    }

    SUBCASE("nibble")
    {
        constexpr auto rule = dsl::bits(dsl::bit::nibble<0xA>, dsl::bit::any<4>);
        CHECK(lexy::is_token_rule<decltype(rule)>);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "bits").cancel());

        for (auto byte = 0; byte < 256; ++byte)
            verify(rule, byte, (byte & 0b1111'0000) == 0xA0);
    }
}

