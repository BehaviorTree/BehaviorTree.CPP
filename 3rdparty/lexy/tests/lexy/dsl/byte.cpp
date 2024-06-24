// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/dsl/byte.hpp>

#include "verify.hpp"
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/if.hpp>

TEST_CASE("dsl::byte")
{
    constexpr auto rule = dsl::byte;
    CHECK(lexy::is_token_rule<decltype(rule)>);
    CHECK(equivalent_rules(rule, dsl::bytes<1>));
}

TEST_CASE("dsl::bytes")
{
    constexpr auto rule = dsl::bytes<4>;
    CHECK(lexy::is_token_rule<decltype(rule)>);

    constexpr auto callback = token_callback;

    auto empty = LEXY_VERIFY(lexy::byte_encoding{});
    CHECK(empty.status == test_result::fatal_error);
    CHECK(empty.trace == test_trace().expected_char_class(0, "byte").cancel());

    auto one = LEXY_VERIFY(lexy::byte_encoding{}, 42);
    CHECK(one.status == test_result::fatal_error);
    CHECK(one.trace == test_trace().error_token("\\2A").expected_char_class(1, "byte").cancel());
    auto two = LEXY_VERIFY(lexy::byte_encoding{}, 42, 11);
    CHECK(two.status == test_result::fatal_error);
    CHECK(two.trace
          == test_trace().error_token("\\2A\\0B").expected_char_class(2, "byte").cancel());
    auto three = LEXY_VERIFY(lexy::byte_encoding{}, 42, 11, 0x42);
    CHECK(three.status == test_result::fatal_error);
    CHECK(three.trace
          == test_trace().error_token("\\2A\\0B\\42").expected_char_class(3, "byte").cancel());

    auto four = LEXY_VERIFY(lexy::byte_encoding{}, 42, 11, 0x42, 0x11);
    CHECK(four.status == test_result::success);
    CHECK(four.trace == test_trace().token("any", "\\2A\\0B\\42\\11"));
    auto five = LEXY_VERIFY(lexy::byte_encoding{}, 42, 11, 0x42, 0x11, 0);
    CHECK(five.status == test_result::success);
    CHECK(five.trace == test_trace().token("any", "\\2A\\0B\\42\\11"));
}

TEST_CASE("dsl::padding_bytes")
{
    constexpr auto pb = dsl::padding_bytes<2, 0xAA>;
    CHECK(lexy::is_branch_rule<decltype(pb)>);

    constexpr auto callback = token_callback;

    SUBCASE("as rule")
    {
        constexpr auto rule = pb;

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::fatal_error);
        CHECK(empty.trace == test_trace().expected_char_class(0, "byte").cancel());

        auto one = LEXY_VERIFY(lexy::byte_encoding{}, 0xAA);
        CHECK(one.status == test_result::fatal_error);
        CHECK(one.trace
              == test_trace().error_token("\\AA").expected_char_class(1, "byte").cancel());

        auto two = LEXY_VERIFY(lexy::byte_encoding{}, 0xAA, 0xAA);
        CHECK(two.status == test_result::success);
        CHECK(two.trace == test_trace().token("any", "\\AA\\AA"));
        auto three = LEXY_VERIFY(lexy::byte_encoding{}, 0xAA, 0xAA, 0xAA);
        CHECK(three.status == test_result::success);
        CHECK(three.trace == test_trace().token("any", "\\AA\\AA"));

        auto bad = LEXY_VERIFY(lexy::byte_encoding{}, 0xBB, 0xBB);
        CHECK(bad.status == test_result::recovered_error);
        CHECK(bad.trace
              == test_trace()
                     .token("any", "\\BB\\BB")
                     .expected_literal(0, "\\AA", 0)
                     .expected_literal(1, "\\AA", 0));
    }
    SUBCASE("as branch rule")
    {
        constexpr auto rule = dsl::if_(pb);

        auto empty = LEXY_VERIFY(lexy::byte_encoding{});
        CHECK(empty.status == test_result::success);
        CHECK(empty.trace == test_trace());

        auto one = LEXY_VERIFY(lexy::byte_encoding{}, 0xAA);
        CHECK(one.status == test_result::success);
        CHECK(one.trace == test_trace());

        auto two = LEXY_VERIFY(lexy::byte_encoding{}, 0xAA, 0xAA);
        CHECK(two.status == test_result::success);
        CHECK(two.trace == test_trace().token("any", "\\AA\\AA"));
        auto three = LEXY_VERIFY(lexy::byte_encoding{}, 0xAA, 0xAA, 0xAA);
        CHECK(three.status == test_result::success);
        CHECK(three.trace == test_trace().token("any", "\\AA\\AA"));

        auto bad = LEXY_VERIFY(lexy::byte_encoding{}, 0xBB, 0xBB);
        CHECK(bad.status == test_result::recovered_error);
        CHECK(bad.trace
              == test_trace()
                     .token("any", "\\BB\\BB")
                     .expected_literal(0, "\\AA", 0)
                     .expected_literal(1, "\\AA", 0));
    }
}

TEST_CASE("dsl::bint conversion")
{
    auto convert = [](auto rule, auto... bytes) {
        unsigned char buffer[] = {static_cast<unsigned char>(bytes)...};
        auto          input    = lexy::string_input(buffer, sizeof...(bytes));

        auto callback = [](const unsigned char*, auto result) { return static_cast<int>(result); };
        return lexy_test::verify(rule, input, callback).value;
    };

    CHECK(convert(dsl::bint8, 0) == 0);
    CHECK(convert(dsl::bint8, 0xFF) == 0xFF);

    CHECK(convert(dsl::big_bint16, 0, 0) == 0);
    CHECK(convert(dsl::big_bint16, 1, 2) == 0x0102);
    CHECK(convert(dsl::big_bint16, 0x00, 0x99) == 0x0099);
    CHECK(convert(dsl::big_bint16, 0x99, 0x00) == 0x9900);
    CHECK(convert(dsl::big_bint16, 0xAA, 0xBB) == 0xAABB);
    CHECK(convert(dsl::big_bint16, 0xFF, 0xFF) == 0xFFFF);

    CHECK(convert(dsl::little_bint16, 0, 0) == 0);
    CHECK(convert(dsl::little_bint16, 1, 2) == 0x0201);
    CHECK(convert(dsl::little_bint16, 0x00, 0x99) == 0x9900);
    CHECK(convert(dsl::little_bint16, 0x99, 0x00) == 0x0099);
    CHECK(convert(dsl::little_bint16, 0xAA, 0xBB) == 0xBBAA);
    CHECK(convert(dsl::little_bint16, 0xFF, 0xFF) == 0xFFFF);

    // other bitwidths use the same code
}

TEST_CASE("dsl::bint")
{
    constexpr auto callback = lexy::callback<int>([](const unsigned char*) { return -11; },
                                                  [](const unsigned char*, std::uint_least16_t i) {
                                                      return static_cast<int>(i);
                                                  },
                                                  [](const unsigned char*, std::uint_least32_t i) {
                                                      return static_cast<int>(i);
                                                  });

    SUBCASE("big")
    {
        constexpr auto bint = dsl::big_bint32;
        CHECK(lexy::is_branch_rule<decltype(bint)>);

        SUBCASE("as rule")
        {
            constexpr auto rule = bint;

            auto empty = LEXY_VERIFY(lexy::byte_encoding{});
            CHECK(empty.status == test_result::fatal_error);
            CHECK(empty.trace == test_trace().expected_char_class(0, "byte").cancel());

            auto not_enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3);
            CHECK(not_enough.status == test_result::fatal_error);
            CHECK(not_enough.trace
                  == test_trace()
                         .error_token("\\01\\02\\03")
                         .expected_char_class(3, "byte")
                         .cancel());

            auto enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3, 4);
            CHECK(enough.status == test_result::success);
            CHECK(enough.value == 0x01020304);
            CHECK(enough.trace == test_trace().token("any", "\\01\\02\\03\\04"));
        }
        SUBCASE("as branch")
        {
            constexpr auto rule = dsl::if_(bint);

            auto empty = LEXY_VERIFY(lexy::byte_encoding{});
            CHECK(empty.status == test_result::success);
            CHECK(empty.value == -11);
            CHECK(empty.trace == test_trace());

            auto not_enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3);
            CHECK(not_enough.status == test_result::success);
            CHECK(not_enough.value == -11);
            CHECK(not_enough.trace == test_trace());

            auto enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3, 4);
            CHECK(enough.status == test_result::success);
            CHECK(enough.value == 0x01020304);
            CHECK(enough.trace == test_trace().token("any", "\\01\\02\\03\\04"));
        }
    }
    SUBCASE("little")
    {
        constexpr auto bint = dsl::little_bint32;
        CHECK(lexy::is_branch_rule<decltype(bint)>);

        SUBCASE("as rule")
        {
            constexpr auto rule = bint;

            auto empty = LEXY_VERIFY(lexy::byte_encoding{});
            CHECK(empty.status == test_result::fatal_error);
            CHECK(empty.trace == test_trace().expected_char_class(0, "byte").cancel());

            auto not_enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3);
            CHECK(not_enough.status == test_result::fatal_error);
            CHECK(not_enough.trace
                  == test_trace()
                         .error_token("\\01\\02\\03")
                         .expected_char_class(3, "byte")
                         .cancel());

            auto enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3, 4);
            CHECK(enough.status == test_result::success);
            CHECK(enough.value == 0x04030201);
            CHECK(enough.trace == test_trace().token("any", "\\01\\02\\03\\04"));
        }
        SUBCASE("as branch")
        {
            constexpr auto rule = dsl::if_(bint);

            auto empty = LEXY_VERIFY(lexy::byte_encoding{});
            CHECK(empty.status == test_result::success);
            CHECK(empty.value == -11);
            CHECK(empty.trace == test_trace());

            auto not_enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3);
            CHECK(not_enough.status == test_result::success);
            CHECK(not_enough.value == -11);
            CHECK(not_enough.trace == test_trace());

            auto enough = LEXY_VERIFY(lexy::byte_encoding{}, 1, 2, 3, 4);
            CHECK(enough.status == test_result::success);
            CHECK(enough.value == 0x04030201);
            CHECK(enough.trace == test_trace().token("any", "\\01\\02\\03\\04"));
        }
    }

    SUBCASE("token rule")
    {
        constexpr auto bint = dsl::big_bint16(dsl::identifier(dsl::ascii::lower).pattern());
        CHECK(lexy::is_branch_rule<decltype(bint)>);

        SUBCASE("as rule")
        {
            constexpr auto rule = bint;

            auto empty = LEXY_VERIFY(lexy::byte_encoding{});
            CHECK(empty.status == test_result::fatal_error);
            CHECK(empty.trace == test_trace().expected_char_class(0, "ASCII.lower").cancel());

            auto a = LEXY_VERIFY(lexy::byte_encoding{}, 0x61);
            CHECK(a.status == test_result::fatal_error);
            CHECK(a.trace
                  == test_trace()
                         .token("identifier", "\\61")
                         .error(0, 1, "mismatched byte count")
                         .cancel());

            auto ab = LEXY_VERIFY(lexy::byte_encoding{}, 0x61, 0x62);
            CHECK(ab.status == test_result::success);
            CHECK(ab.value == 0x6162);
            CHECK(ab.trace == test_trace().token("identifier", "\\61\\62"));

            auto abc = LEXY_VERIFY(lexy::byte_encoding{}, 0x61, 0x62, 0x63);
            CHECK(abc.status == test_result::fatal_error);
            CHECK(abc.trace
                  == test_trace()
                         .token("identifier", "\\61\\62\\63")
                         .error(0, 3, "mismatched byte count")
                         .cancel());
        }
        SUBCASE("as branch")
        {
            constexpr auto rule = dsl::if_(bint);

            auto empty = LEXY_VERIFY(lexy::byte_encoding{});
            CHECK(empty.status == test_result::success);
            CHECK(empty.trace == test_trace());

            auto a = LEXY_VERIFY(lexy::byte_encoding{}, 0x61);
            CHECK(a.status == test_result::fatal_error);
            CHECK(a.trace
                  == test_trace()
                         .token("identifier", "\\61")
                         .error(0, 1, "mismatched byte count")
                         .cancel());

            auto ab = LEXY_VERIFY(lexy::byte_encoding{}, 0x61, 0x62);
            CHECK(ab.status == test_result::success);
            CHECK(ab.value == 0x6162);
            CHECK(ab.trace == test_trace().token("identifier", "\\61\\62"));

            auto abc = LEXY_VERIFY(lexy::byte_encoding{}, 0x61, 0x62, 0x63);
            CHECK(abc.status == test_result::fatal_error);
            CHECK(abc.trace
                  == test_trace()
                         .token("identifier", "\\61\\62\\63")
                         .error(0, 3, "mismatched byte count")
                         .cancel());
        }
    }
}

