// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef BENCHMARKS_SWAR_SWAR_HPP_INCLUDED
#define BENCHMARKS_SWAR_SWAR_HPP_INCLUDED

#include <lexy/dsl/any.hpp>
#include <lexy/input/buffer.hpp>
#include <nanobench.h>

#if defined(__GNUC__)
#    define LEXY_NOINLINE [[gnu::noinline]]
#else
#    define LEXY_NOINLINE
#endif

template <typename Encoding>
class swar_disabled_reader
{
public:
    using encoding = Encoding;
    using iterator = const typename Encoding::char_type*;

    struct marker
    {
        iterator _it;

        constexpr iterator position() const noexcept
        {
            return _it;
        }
    };

    explicit swar_disabled_reader(iterator begin) noexcept : _cur(begin) {}

    auto peek() const noexcept
    {
        // The last one will be EOF.
        return *_cur;
    }

    void bump() noexcept
    {
        ++_cur;
    }

    iterator position() const noexcept
    {
        return _cur;
    }

    marker current() const noexcept
    {
        return {_cur};
    }
    void reset(marker m) noexcept
    {
        _cur = m._it;
    }

private:
    iterator _cur;
};

template <typename Encoding>
constexpr auto disable_swar(lexy::_br<Encoding> reader)
{
    return swar_disabled_reader<Encoding>(reader.position());
}

lexy::buffer<lexy::utf8_encoding> random_buffer(std::size_t size, float unicode_ratio);

lexy::buffer<lexy::utf8_encoding> repeat_buffer_padded(std::size_t size, const char* str);

#endif // BENCHMARKS_SWAR_SWAR_HPP_INCLUDED

