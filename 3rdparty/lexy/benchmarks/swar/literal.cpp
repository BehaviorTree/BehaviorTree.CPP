// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "swar.hpp"

#include <lexy/dsl/literal.hpp>

namespace
{
template <typename Reader>
LEXY_NOINLINE std::size_t bm_a(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::lit_c<'a'>, reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}

template <typename Reader>
LEXY_NOINLINE std::size_t bm_abcd(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(LEXY_LIT("abcd"), reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}

template <typename Reader>
LEXY_NOINLINE std::size_t bm_alphabet(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(LEXY_LIT("abcdefghijklmnopqrstuvwxyz"), reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}
} // namespace

std::size_t bm_lit(ankerl::nanobench::Bench& b)
{
    auto count = std::size_t(0);

    auto a = repeat_buffer_padded(10 * 1024ull, "a");
    b.minEpochIterations(10 * 1000ull);
    b.unit("byte").batch(a.size());
    b.run("lit/manual/a", [&] { return count += bm_a(disable_swar(a.reader())); });
    b.run("lit/swar/a", [&] { return count += bm_a(a.reader()); });

    auto abcd = repeat_buffer_padded(10 * 1024ull, "abcd");
    b.minEpochIterations(10 * 1000ull);
    b.unit("byte").batch(a.size());
    b.run("lit/manual/abcd", [&] { return count += bm_abcd(disable_swar(a.reader())); });
    b.run("lit/swar/abcd", [&] { return count += bm_abcd(a.reader()); });

    auto alphabet = repeat_buffer_padded(10 * 1024ull, "abcdefghijklmnopqrstuvwxyz");
    b.minEpochIterations(10 * 1000ull);
    b.unit("byte").batch(a.size());
    b.run("lit/manual/alphabet", [&] { return count += bm_alphabet(disable_swar(a.reader())); });
    b.run("lit/swar/alphabet", [&] { return count += bm_alphabet(a.reader()); });

    return count;
}

