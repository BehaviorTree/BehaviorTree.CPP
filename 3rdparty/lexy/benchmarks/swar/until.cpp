#include "swar.hpp"

#include <lexy/dsl/newline.hpp>
#include <lexy/dsl/until.hpp>

namespace
{
template <typename Reader>
LEXY_NOINLINE std::size_t bm_until(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::until(lexy::dsl::newline), reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}

template <typename Reader>
LEXY_NOINLINE std::size_t bm_until_eof(Reader reader)
{
    auto count = 0u;
    while (reader.peek() != Reader::encoding::eof())
    {
        if (lexy::try_match_token(lexy::dsl::until(lexy::dsl::newline).or_eof(), reader))
            ++count;
        else
            reader.bump();
    }
    return count;
}
} // namespace

std::size_t bm_until(ankerl::nanobench::Bench& b)
{
    auto small = repeat_buffer_padded(
        1031, "abc\nabcdefghijkl\r\nabcdefghijklmnopqrstuvwxyz\nabcdfghijkl\rmnopqrstuvwxyz\n");
    auto ascii        = random_buffer(1031, 0);
    auto few_unicode  = random_buffer(1031, 0.1f);
    auto much_unicode = random_buffer(1031, 0.5f);

    auto count = std::size_t(0);

    b.minEpochIterations(100 * 1000ull);
    b.unit("byte").batch(small.size());
    b.run("until/manual/small", [&] { return count += bm_until(disable_swar(small.reader())); });
    b.run("until/swar/small", [&] { return count += bm_until(small.reader()); });

    b.unit("byte").batch(ascii.size());
    b.run("until/manual/ascii", [&] { return count += bm_until(disable_swar(ascii.reader())); });
    b.run("until/swar/ascii", [&] { return count += bm_until(ascii.reader()); });

    b.unit("byte").batch(few_unicode.size());
    b.run("until/manual/few_unicode",
          [&] { return count += bm_until(disable_swar(few_unicode.reader())); });
    b.run("until/swar/few_unicode", [&] { return count += bm_until(few_unicode.reader()); });

    b.unit("byte").batch(much_unicode.size());
    b.run("until/manual/much_unicode",
          [&] { return count += bm_until(disable_swar(much_unicode.reader())); });
    b.run("until/swar/much_unicode", [&] { return count += bm_until(much_unicode.reader()); });

    b.unit("byte").batch(small.size());
    b.run("until_eof/manual/small",
          [&] { return count += bm_until_eof(disable_swar(small.reader())); });
    b.run("until_eof/swar/small", [&] { return count += bm_until_eof(small.reader()); });

    b.unit("byte").batch(ascii.size());
    b.run("until_eof/manual/ascii",
          [&] { return count += bm_until_eof(disable_swar(ascii.reader())); });
    b.run("until_eof/swar/ascii", [&] { return count += bm_until_eof(ascii.reader()); });

    b.unit("byte").batch(few_unicode.size());
    b.run("until_eof/manual/few_unicode",
          [&] { return count += bm_until_eof(disable_swar(few_unicode.reader())); });
    b.run("until_eof/swar/few_unicode",
          [&] { return count += bm_until_eof(few_unicode.reader()); });

    b.unit("byte").batch(much_unicode.size());
    b.run("until_eof/manual/much_unicode",
          [&] { return count += bm_until_eof(disable_swar(much_unicode.reader())); });
    b.run("until_eof/swar/much_unicode",
          [&] { return count += bm_until_eof(much_unicode.reader()); });

    return count;
}

