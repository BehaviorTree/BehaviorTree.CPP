// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#include "swar.hpp"

#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/delimited.hpp>

namespace
{
template <typename Reader>
LEXY_NOINLINE std::size_t bm_quoted(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::token(lexy::dsl::quoted(lexy::dsl::ascii::print)),
                                  reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}

template <typename Reader>
LEXY_NOINLINE std::size_t bm_quoted_escape(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::token(
                                      lexy::dsl::quoted(lexy::dsl::ascii::print,
                                                        lexy::dsl::backslash_escape.rule(
                                                            lexy::dsl::lit_c<'"'>))),
                                  reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}
} // namespace

std::size_t bm_delimited(ankerl::nanobench::Bench& b)
{
    auto count = std::size_t(0);

    auto strs = repeat_buffer_padded(
        10 * 1024ull,
        R"("", "a", "b", "c", "d", "abc", "hello world", "this is a string literal", "lexy is a C++ parsing DSL library.", "I don't really know many strings I could put here.", "I'm guessing strings are usually about this large, right?", "This is a string literal with \" escapes\n")");
    auto ascii        = random_buffer(10 * 1024ull, 0);
    auto few_unicode  = random_buffer(10 * 1024ull, 0.1f);
    auto much_unicode = random_buffer(10 * 1024ull, 0.5f);

    b.minEpochIterations(10000);

    b.unit("byte").batch(strs.size());
    b.run("quoted/manual/strs", [&] { return count += bm_quoted(disable_swar(strs.reader())); });
    b.run("quoted/swar/strs", [&] { return count += bm_quoted(strs.reader()); });
    b.run("quoted-escape/manual/strs",
          [&] { return count += bm_quoted_escape(disable_swar(strs.reader())); });
    b.run("quoted-escape/swar/strs", [&] { return count += bm_quoted_escape(strs.reader()); });

    b.unit("byte").batch(ascii.size());
    b.run("quoted/manual/ascii", [&] { return count += bm_quoted(disable_swar(ascii.reader())); });
    b.run("quoted/swar/ascii", [&] { return count += bm_quoted(ascii.reader()); });
    b.run("quoted-escape/manual/ascii",
          [&] { return count += bm_quoted_escape(disable_swar(ascii.reader())); });
    b.run("quoted-escape/swar/ascii", [&] { return count += bm_quoted_escape(ascii.reader()); });

    b.unit("byte").batch(few_unicode.size());
    b.run("quoted/manual/few_unicode",
          [&] { return count += bm_quoted(disable_swar(few_unicode.reader())); });
    b.run("quoted/swar/few_unicode", [&] { return count += bm_quoted(few_unicode.reader()); });
    b.run("quoted-escape/manual/few_unicode",
          [&] { return count += bm_quoted_escape(disable_swar(few_unicode.reader())); });
    b.run("quoted-escape/swar/few_unicode",
          [&] { return count += bm_quoted_escape(few_unicode.reader()); });

    b.unit("byte").batch(much_unicode.size());
    b.run("quoted/manual/much_unicode",
          [&] { return count += bm_quoted(disable_swar(much_unicode.reader())); });
    b.run("quoted/swar/much_unicode", [&] { return count += bm_quoted(much_unicode.reader()); });
    b.run("quoted-escape/manual/much_unicode",
          [&] { return count += bm_quoted_escape(disable_swar(much_unicode.reader())); });
    b.run("quoted-escape/swar/much_unicode",
          [&] { return count += bm_quoted_escape(much_unicode.reader()); });

    return count;
}

