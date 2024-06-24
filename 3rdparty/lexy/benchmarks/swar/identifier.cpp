// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "swar.hpp"

#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/unicode.hpp>

namespace
{
template <typename Reader>
LEXY_NOINLINE std::size_t bm_ascii(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::identifier(lexy::dsl::ascii::word).pattern(), reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}

template <typename Reader>
LEXY_NOINLINE std::size_t bm_unicode(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::identifier(lexy::dsl::unicode::xid_start,
                                                        lexy::dsl::unicode::xid_continue)
                                      .pattern(),
                                  reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}
} // namespace

std::size_t bm_identifier(ankerl::nanobench::Bench& b)
{
    auto count = std::size_t(0);

    auto words = repeat_buffer_padded(
        10 * 1024ull,
        "Hello, World, how, are, you, lexy, ankerl, nanobench, Bench, bm_identifier, std::stringstream, rotate, antidisestablishmentarianism, Kurzfristenergieversorgungssicherungsmaßnahmenverordnung");
    auto ascii        = random_buffer(10 * 1024ull, 0);
    auto few_unicode  = random_buffer(10 * 1024ull, 0.1f);
    auto much_unicode = random_buffer(10 * 1024ull, 0.5f);

    b.minEpochIterations(100);

    b.unit("byte").batch(words.size());
    b.run("identifier-ascii/manual/words",
          [&] { return count += bm_ascii(disable_swar(words.reader())); });
    b.run("identifier-ascii/swar/words", [&] { return count += bm_ascii(words.reader()); });
    b.run("identifier-unicode/manual/words",
          [&] { return count += bm_unicode(disable_swar(words.reader())); });
    b.run("identifier-unicode/swar/words", [&] { return count += bm_unicode(words.reader()); });

    b.unit("byte").batch(ascii.size());
    b.run("identifier-ascii/manual/ascii",
          [&] { return count += bm_ascii(disable_swar(ascii.reader())); });
    b.run("identifier-ascii/swar/ascii", [&] { return count += bm_ascii(ascii.reader()); });
    b.run("identifier-unicode/manual/ascii",
          [&] { return count += bm_unicode(disable_swar(ascii.reader())); });
    b.run("identifier-unicode/swar/ascii", [&] { return count += bm_ascii(ascii.reader()); });

    b.unit("byte").batch(few_unicode.size());
    b.run("identifier-ascii/manual/few_unicode",
          [&] { return count += bm_ascii(disable_swar(few_unicode.reader())); });
    b.run("identifier-ascii/swar/few_unicode",
          [&] { return count += bm_ascii(few_unicode.reader()); });
    b.run("identifier-unicode/manual/few_unicode",
          [&] { return count += bm_unicode(disable_swar(few_unicode.reader())); });
    b.run("identifier-unicode/swar/few_unicode",
          [&] { return count += bm_unicode(few_unicode.reader()); });

    b.unit("byte").batch(much_unicode.size());
    b.run("identifier-ascii/manual/much_unicode",
          [&] { return count += bm_ascii(disable_swar(much_unicode.reader())); });
    b.run("identifier-ascii/swar/much_unicode",
          [&] { return count += bm_ascii(much_unicode.reader()); });
    b.run("identifier-unicode/manual/much_unicode",
          [&] { return count += bm_unicode(disable_swar(much_unicode.reader())); });
    b.run("identifier-unicode/swar/much_unicode",
          [&] { return count += bm_unicode(much_unicode.reader()); });

    return count;
}

