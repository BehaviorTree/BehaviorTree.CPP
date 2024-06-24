// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "swar.hpp"

#include <lexy/dsl/digit.hpp>

namespace
{
template <typename Reader>
LEXY_NOINLINE std::size_t bm_decimal(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::digits<>, reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}
template <typename Reader>
LEXY_NOINLINE std::size_t bm_hex(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::digits<lexy::dsl::hex>, reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}

template <typename Reader>
LEXY_NOINLINE std::size_t bm_no_leading(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::digits<>.no_leading_zero(), reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}

template <typename Reader>
LEXY_NOINLINE std::size_t bm_sep(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::digits<>.sep(lexy::dsl::digit_sep_tick), reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}
} // namespace

std::size_t bm_digits(ankerl::nanobench::Bench& b)
{
    auto count = std::size_t(0);

    // Totally scientific and legit benchmark here.
    auto decimal = repeat_buffer_padded(
        10 * 1024ull,
        "0,1,2,3,4,5,6,7,8,9,11,42,100,1024,16401561405,132512476845576,43626725672,145626,4096,14315612436,14362543625473,");
    auto hex = repeat_buffer_padded(
        10 * 1024ull,
        "0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F,a,b,c,d,e,f,11,42,FF,10AB,1024DEF,41626275A1462BCaaD,234567890ABDeA2345ABcc45");

    b.minEpochIterations(50 * 1000ull);
    b.unit("byte").batch(decimal.size());
    b.run("digits/manual/decimal",
          [&] { return count += bm_decimal(disable_swar(decimal.reader())); });
    b.run("digits/swar/decimal", [&] { return count += bm_decimal(decimal.reader()); });

    b.unit("byte").batch(hex.size());
    b.run("digits/manual/hex", [&] { return count += bm_hex(disable_swar(hex.reader())); });
    b.run("digits/swar/hex", [&] { return count += bm_hex(hex.reader()); });

    b.unit("byte").batch(decimal.size());
    b.run("digits/manual/no_leading_zero",
          [&] { return count += bm_no_leading(disable_swar(decimal.reader())); });
    b.run("digits/swar/no_leading_zero", [&] { return count += bm_no_leading(decimal.reader()); });

    b.unit("byte").batch(decimal.size());
    b.run("digits/manual/sep", [&] { return count += bm_sep(disable_swar(decimal.reader())); });
    b.run("digits/swar/sep", [&] { return count += bm_sep(decimal.reader()); });

    return count;
}

