// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/_detail/swar.hpp>

#include <doctest/doctest.h>

using namespace lexy::_detail;

TEST_CASE("swar_fill")
{
    REQUIRE(sizeof(swar_int) == 8);
    CHECK(swar_fill(char(0)) == 0);
    CHECK(swar_fill(char(0x30)) == 0x30303030'30303030);
    CHECK(swar_fill(char(0x80)) == 0x80808080'80808080);
    CHECK(swar_fill(char(0xAA)) == 0xAAAAAAAA'AAAAAAAA);
    CHECK(swar_fill(char16_t(0x30)) == 0x00300030'00300030);
    CHECK(swar_fill(char16_t(0xAA)) == 0x00AA00AA'00AA00AA);
}

TEST_CASE("swar_fill_compl")
{
    REQUIRE(sizeof(swar_int) == 8);
    CHECK(swar_fill_compl(char(0)) == 0xFFFFFFFF'FFFFFFFF);
    CHECK(swar_fill_compl(char(0xF)) == 0xF0F0F0F0'F0F0F0F0);
    CHECK(swar_fill_compl(char16_t(0xF)) == 0xFFF0FFF0'FFF0FFF0);
}

TEST_CASE("swar_pack")
{
    REQUIRE(sizeof(swar_int) == 8);

    constexpr auto single_char = swar_pack(char(0x11));
    CHECK(single_char.value == 0x11);
    CHECK(single_char.mask == 0xFF);
    CHECK(single_char.count == 1);

    constexpr auto multiple_chars
        = swar_pack(char(0x00), char(0x11), char(0x22), char(0x33), char(0x44));
    CHECK(multiple_chars.value == 0x4433221100);
    CHECK(multiple_chars.mask == 0xFFFFFFFFFF);
    CHECK(multiple_chars.count == 5);

    constexpr auto full = swar_pack(char32_t(0x11), char32_t(0x22));
    CHECK(full.value == 0x00000022'00000011);
    CHECK(full.mask == 0xFFFFFFFF'FFFFFFFF);
    CHECK(full.count == 2);

    constexpr auto overflow = swar_pack(char32_t(0x11), char32_t(0x22), char32_t(0x33));
    CHECK(overflow.value == 0x00000022'00000011);
    CHECK(overflow.mask == 0xFFFFFFFF'FFFFFFFF);
    CHECK(overflow.count == 2);

    constexpr auto offset
        = swar_pack<2>(char(0x00), char(0x11), char(0x22), char(0x33), char(0x44));
    CHECK(offset.value == 0x443322);
    CHECK(offset.mask == 0xFFFFFF);
    CHECK(offset.count == 3);

    constexpr auto overflow_offset = swar_pack<1>(char32_t(0x11), char32_t(0x22), char32_t(0x33));
    CHECK(overflow_offset.value == 0x00000033'00000022);
    CHECK(overflow_offset.mask == 0xFFFFFFFF'FFFFFFFF);
    CHECK(overflow_offset.count == 2);
}

TEST_CASE("swar_find_difference")
{
    REQUIRE(sizeof(swar_int) == 8);
    constexpr auto a   = swar_pack(char('a')).value;
    constexpr auto A   = swar_pack(char('A')).value;
    constexpr auto abc = swar_pack(char('a'), char('b'), char('c')).value;
    constexpr auto aBc = swar_pack(char('a'), char('B'), char('c')).value;

    CHECK(swar_find_difference<char>(a, a) == 8);
    CHECK(swar_find_difference<char>(a, A) == 0);
    CHECK(swar_find_difference<char>(abc, aBc) == 1);
}

TEST_CASE("swar_has_zero")
{
    SUBCASE("char")
    {
        constexpr auto all_zero = swar_fill(char(0));
        CHECK(swar_has_zero<char>(all_zero));

        constexpr auto all_one = swar_fill(char(1));
        CHECK(!swar_has_zero<char>(all_one));

        constexpr auto all_high = swar_fill(char(0xAB));
        CHECK(!swar_has_zero<char>(all_high));

        constexpr auto contains_zero = swar_pack(char('a'), char('b'), char('c'), char(0),
                                                 char('d'), char('e'), char('f'), char('g'))
                                           .value;
        CHECK(swar_has_zero<char>(contains_zero));
    }
    SUBCASE("char32_t")
    {
        constexpr auto all_zero = swar_fill(char32_t(0));
        CHECK(swar_has_zero<char32_t>(all_zero));

        constexpr auto all_one = swar_fill(char32_t(1));
        CHECK(!swar_has_zero<char32_t>(all_one));

        constexpr auto all_high = swar_fill(char32_t(0xAB));
        CHECK(!swar_has_zero<char32_t>(all_high));

        constexpr auto contains_zero = swar_pack(char32_t('a'), char32_t(0)).value;
        CHECK(swar_has_zero<char32_t>(contains_zero));
    }
}

TEST_CASE("swar_has_char")
{
    SUBCASE("char")
    {
        constexpr auto all_zero = swar_fill(char(0));
        CHECK(!swar_has_char<char, 1>(all_zero));

        constexpr auto all_one = swar_fill(char(1));
        CHECK(swar_has_char<char, 1>(all_one));

        constexpr auto all_high = swar_fill(char(0xAB));
        CHECK(!swar_has_char<char, 1>(all_high));

        constexpr auto contains_one = swar_pack(char('a'), char('b'), char('c'), char(1), char('d'),
                                                char('e'), char('f'), char('g'))
                                          .value;
        CHECK(swar_has_char<char, 1>(contains_one));
    }
    SUBCASE("char32_t")
    {
        constexpr auto all_zero = swar_fill(char32_t(0));
        CHECK(!swar_has_char<char32_t, 1>(all_zero));

        constexpr auto all_one = swar_fill(char32_t(1));
        CHECK(swar_has_char<char32_t, 1>(all_one));

        constexpr auto all_high = swar_fill(char32_t(0xAB));
        CHECK(!swar_has_char<char32_t, 1>(all_high));

        constexpr auto contains_one = swar_pack(char32_t('a'), char32_t(1)).value;
        CHECK(swar_has_char<char32_t, 1>(contains_one));
    }
}

