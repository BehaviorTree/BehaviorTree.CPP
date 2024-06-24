// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BITS_HPP_INCLUDED
#define LEXY_DSL_BITS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

//=== bit rules ===//
namespace lexyd::bit
{
struct _bit_pattern
{
    unsigned mask;
    unsigned value;
};

struct _bit_rule
{};

struct _b0 : _bit_rule
{
    static constexpr auto size = 1u;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= 1;
        p.value <<= 1;

        p.mask |= 1;
    }
};

/// Matches a 0 bit.
inline constexpr auto _0 = _b0{};

struct _b1 : _bit_rule
{
    static constexpr auto size = 1u;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= 1;
        p.value <<= 1;

        p.mask |= 1;
        p.value |= 1;
    }
};

/// Matches a 1 bit.
inline constexpr auto _1 = _b1{};

template <unsigned Value>
struct _n : _bit_rule
{
    static_assert(Value <= 0xF);

    static constexpr auto size = 4u;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= 4;
        p.value <<= 4;

        p.mask |= 0b1111;
        p.value |= Value;
    }
};

/// Matches a specific nibble, i.e. four bits.
template <unsigned Value>
constexpr auto nibble = _n<Value>{};

template <unsigned N>
struct _b : _bit_rule
{
    static_assert(N > 0);

    static constexpr auto size = N;

    static constexpr void apply(_bit_pattern& p)
    {
        p.mask <<= N;
        p.value <<= N;
    }
};

/// Matches any bit.
inline constexpr auto _ = _b<1>{};

/// Matches N arbitrary bits.
template <unsigned N>
constexpr auto any = _b<N>{};
} // namespace lexyd::bit

//=== bits ===//
namespace lexyd
{
template <unsigned Mask, unsigned Value>
struct _bits : token_base<_bits<Mask, Value>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            static_assert(lexy::is_byte_encoding<typename Reader::encoding>);

            auto byte = reader.peek();
            if (byte == Reader::encoding::eof()
                || ((static_cast<unsigned char>(byte) & Mask) != Value))
                return false;

            reader.bump();
            end = reader.current();
            return true;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(), "bits");
            context.on(_ev::error{}, err);
        }
    };
};

/// Matches the specific bit pattern.
template <typename... Bits>
constexpr auto bits(Bits...)
{
    static_assert((std::is_base_of_v<bit::_bit_rule, Bits> && ...), "bits() requires bit rules");
    static_assert((0 + ... + Bits::size) == 8, "must specify 8 bit at a time");

    constexpr auto pattern = [] {
        bit::_bit_pattern result{0, 0};
        (Bits::apply(result), ...);
        return result;
    }();

    return _bits<pattern.mask, pattern.value>{};
}
} // namespace lexyd

#endif // LEXY_DSL_BITS_HPP_INCLUDED

