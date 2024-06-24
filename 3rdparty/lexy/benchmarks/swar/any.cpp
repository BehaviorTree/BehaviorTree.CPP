// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "swar.hpp"

#include <lexy/dsl/any.hpp>

namespace
{
template <typename Reader>
LEXY_NOINLINE std::size_t bm_any(Reader reader)
{
    auto begin = reader.position();
    lexy::try_match_token(lexy::dsl::any, reader);
    auto end = reader.position();

    return lexy::_detail::range_size(begin, end);
}
} // namespace

std::size_t bm_any(ankerl::nanobench::Bench& b)
{
    auto small        = random_buffer(10, 0);
    auto ascii        = random_buffer(1031, 0);
    auto few_unicode  = random_buffer(1031, 0.1f);
    auto much_unicode = random_buffer(1031, 0.5f);

    auto count = std::size_t(0);

    b.minEpochIterations(1000 * 1000ull);
    b.unit("byte").batch(small.size());
    b.run("any/manual/small", [&] { return count += bm_any(disable_swar(small.reader())); });
    b.run("any/swar/small", [&] { return count += bm_any(small.reader()); });

    b.minEpochIterations(100 * 1000ull);
    b.unit("byte").batch(ascii.size());
    b.run("any/manual/ascii", [&] { return count += bm_any(disable_swar(ascii.reader())); });
    b.run("any/swar/ascii", [&] { return count += bm_any(ascii.reader()); });

    b.minEpochIterations(100 * 1000ull);
    b.unit("byte").batch(few_unicode.size());
    b.run("any/manual/few_unicode",
          [&] { return count += bm_any(disable_swar(few_unicode.reader())); });
    b.run("any/swar/few_unicode", [&] { return count += bm_any(few_unicode.reader()); });

    b.minEpochIterations(100 * 1000ull);
    b.unit("byte").batch(much_unicode.size());
    b.run("any/manual/much_unicode",
          [&] { return count += bm_any(disable_swar(much_unicode.reader())); });
    b.run("any/swar/much_unicode", [&] { return count += bm_any(much_unicode.reader()); });

    return count;
}

