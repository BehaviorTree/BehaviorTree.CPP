// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include <lexy/code_point.hpp>

#include <cctype>
#include <doctest/doctest.h>

TEST_CASE("code_point - classification")
{
    // Note: we're only testing the BMP.
    auto compare_gc = [](lexy::code_point cp) {
        CHECK(cp.is_control() == (cp.general_category() == lexy::code_point::Cc));
        CHECK(cp.is_surrogate() == (cp.general_category() == lexy::code_point::Cs));
        CHECK(cp.is_private_use() == (cp.general_category() == lexy::code_point::Co));
        CHECK(cp.is_noncharacter() == (cp.general_category() == lexy::code_point::Cn));
    };

    for (auto i = 0u; i <= 0x7F; ++i)
    {
        lexy::code_point cp(i);
        CHECK(cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(cp.is_control() == !!std::iscntrl(static_cast<int>(cp.value())));
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());
        compare_gc(cp);

        CHECK(cp.is_scalar());
    }
    for (auto i = 0x80u; i <= 0x9F; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());
        compare_gc(cp);

        CHECK(cp.is_scalar());
    }
    for (auto i = 0xA0u; i <= 0xFF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());
        compare_gc(cp);

        CHECK(cp.is_scalar());
    }

    // 0x0100 - 0xD7FF are normal

    for (auto i = 0xD800u; i <= 0xDFFF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(!cp.is_noncharacter());
        compare_gc(cp);

        CHECK(!cp.is_scalar());
    }
    for (auto i = 0xE000u; i <= 0xF8FF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(cp.is_private_use());
        CHECK(!cp.is_noncharacter());
        compare_gc(cp);

        CHECK(cp.is_scalar());
    }

    // 0xF900 - 0xFDCF are normal

    for (auto i = 0xFDD0u; i <= 0xFDEF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(cp.is_noncharacter());
        compare_gc(cp);

        CHECK(cp.is_scalar());
    }

    // 0xFDF0 - 0xFFFD are normal

    for (auto i = 0xFFFEu; i <= 0xFFFF; ++i)
    {
        lexy::code_point cp(i);
        CHECK(!cp.is_ascii());
        CHECK(cp.is_bmp());
        CHECK(cp.is_valid());

        CHECK(!cp.is_control());
        CHECK(!cp.is_surrogate());
        CHECK(!cp.is_private_use());
        CHECK(cp.is_noncharacter());
        compare_gc(cp);

        CHECK(cp.is_scalar());
    }
}

namespace
{
std::size_t count_code_points(lexy::code_point::general_category_t gc)
{
    auto count = 0u;
    for (char32_t cp = 0; cp <= 0x10FFFF; ++cp)
        if (lexy::code_point(cp).general_category() == gc)
            ++count;
    return count;
}
} // namespace

TEST_CASE("code_point - general category")
{
    // Update this test case when a new Unicode version comes out.
    // https://en.wikipedia.org/wiki/Unicode_character_property#General_Category
    REQUIRE(LEXY_UNICODE_DATABASE_VERSION == doctest::String("14.0.0"));

    CHECK(count_code_points(lexy::code_point::Lu) == 1831);
    CHECK(count_code_points(lexy::code_point::Ll) == 2227);
    CHECK(count_code_points(lexy::code_point::Lt) == 31);
    CHECK(count_code_points(lexy::code_point::Lm) == 334);
    CHECK(count_code_points(lexy::code_point::Lo) == 127333);

    CHECK(count_code_points(lexy::code_point::Mn) == 1950);
    CHECK(count_code_points(lexy::code_point::Mc) == 445);
    CHECK(count_code_points(lexy::code_point::Me) == 13);

    CHECK(count_code_points(lexy::code_point::Nd) == 660);
    CHECK(count_code_points(lexy::code_point::Nl) == 236);
    CHECK(count_code_points(lexy::code_point::No) == 895);

    CHECK(count_code_points(lexy::code_point::Pc) == 10);
    CHECK(count_code_points(lexy::code_point::Pd) == 26);
    CHECK(count_code_points(lexy::code_point::Ps) == 79);
    CHECK(count_code_points(lexy::code_point::Pe) == 77);
    CHECK(count_code_points(lexy::code_point::Pi) == 12);
    CHECK(count_code_points(lexy::code_point::Pf) == 10);
    CHECK(count_code_points(lexy::code_point::Po) == 605);

    CHECK(count_code_points(lexy::code_point::Sm) == 948);
    CHECK(count_code_points(lexy::code_point::Sc) == 63);
    CHECK(count_code_points(lexy::code_point::Sk) == 125);
    CHECK(count_code_points(lexy::code_point::So) == 6605);

    CHECK(count_code_points(lexy::code_point::Zs) == 17);
    CHECK(count_code_points(lexy::code_point::Zl) == 1);
    CHECK(count_code_points(lexy::code_point::Zp) == 1);

    CHECK(count_code_points(lexy::code_point::Cc) == 65);
    CHECK(count_code_points(lexy::code_point::Cf) == 163);
    CHECK(count_code_points(lexy::code_point::Cs) == 2048);
    CHECK(count_code_points(lexy::code_point::Co) == 137468);

    // Need to include the noncharacters in the count_code_points.
    CHECK(count_code_points(lexy::code_point::Cn) == 829768 + 66);
}

TEST_CASE("lexy::simple_case_fold")
{
    // ASCII
    for (char32_t c = 0; c <= 0x7F; ++c)
    {
        auto cp = lexy::code_point(c);
        if (c >= 'A' && c <= 'Z')
        {
            auto lower = lexy::code_point(c + ('a' - 'A'));
            CHECK(lexy::simple_case_fold(cp) == lower);
        }
        else
            CHECK(lexy::simple_case_fold(cp) == cp);
    }

    // arbitrary other code points (identity)
    CHECK(lexy::simple_case_fold(lexy::code_point(0xFF)) == lexy::code_point(0xFF));
    CHECK(lexy::simple_case_fold(lexy::code_point(0xFFFF)) == lexy::code_point(0xFFFF));
    CHECK(lexy::simple_case_fold(lexy::code_point(0x10FFF)) == lexy::code_point(0x10FFF));
    CHECK(lexy::simple_case_fold(lexy::code_point(0xABCDEF)) == lexy::code_point(0xABCDEF));

    // arbitrary other code points (canonical)
    CHECK(lexy::simple_case_fold(lexy::code_point(0xC4)) == lexy::code_point(0xE4));
    CHECK(lexy::simple_case_fold(lexy::code_point(0x1F1)) == lexy::code_point(0x1F3));
    CHECK(lexy::simple_case_fold(lexy::code_point(0x10A0)) == lexy::code_point(0x2D00));
    CHECK(lexy::simple_case_fold(lexy::code_point(0x1F59)) == lexy::code_point(0x1F51));
    CHECK(lexy::simple_case_fold(lexy::code_point(0x10400)) == lexy::code_point(0x10428));

    // arbitrary simple mappings
    CHECK(lexy::simple_case_fold(lexy::code_point(0x1E9E)) == lexy::code_point(0xDF));
    CHECK(lexy::simple_case_fold(lexy::code_point(0x1FAB)) == lexy::code_point(0x1FA3));
    CHECK(lexy::simple_case_fold(lexy::code_point(0x1FFC)) == lexy::code_point(0x1FF3));
}

