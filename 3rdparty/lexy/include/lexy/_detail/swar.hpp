// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_SWAR_HPP_INCLUDED
#define LEXY_DETAIL_SWAR_HPP_INCLUDED

#include <climits>
#include <cstdint>
#include <cstring>
#include <lexy/_detail/config.hpp>
#include <lexy/input/base.hpp>

#if defined(_MSC_VER)
#    include <intrin.h>
#endif

namespace lexy::_detail
{
// Contains the chars in little endian order; rightmost bits are first char.
using swar_int = std::uintmax_t;

// The number of chars that can fit into one SWAR.
template <typename CharT>
constexpr auto swar_length = [] {
    static_assert(sizeof(CharT) < sizeof(swar_int) && sizeof(swar_int) % sizeof(CharT) == 0);
    return sizeof(swar_int) / sizeof(CharT);
}();

template <typename CharT>
constexpr auto char_bit_size = sizeof(CharT) * CHAR_BIT;

template <typename CharT>
constexpr auto make_uchar(CharT c)
{
    if constexpr (std::is_same_v<CharT, LEXY_CHAR8_T>)
        // Not all libstdc++ support char8_t and std::make_unsigned_t.
        return c;
    else
        return std::make_unsigned_t<CharT>(c);
}
template <typename CharT>
using uchar_t = decltype(make_uchar(CharT()));

// Returns a swar_int filled with the specific char.
template <typename CharT>
constexpr swar_int swar_fill(CharT _c)
{
    auto c = make_uchar(_c);

    auto result = swar_int(0);
    for (auto i = 0u; i != swar_length<CharT>; ++i)
    {
        result <<= char_bit_size<CharT>;
        result |= c;
    }
    return result;
}

// Returns a swar_int filled with the complement of the specific char.
template <typename CharT>
constexpr swar_int swar_fill_compl(CharT _c)
{
    auto c = uchar_t<CharT>(~uchar_t<CharT>(_c));

    auto result = swar_int(0);
    for (auto i = 0u; i != swar_length<CharT>; ++i)
    {
        result <<= char_bit_size<CharT>;
        result |= c;
    }
    return result;
}

constexpr void _swar_pack(swar_int&, int) {}
template <typename H, typename... T>
constexpr void _swar_pack(swar_int& result, int index, H h, T... t)
{
    if (std::size_t(index) == char_bit_size<swar_int>)
        return;

    if (index >= 0)
        result |= swar_int(make_uchar(h)) << index;

    _swar_pack(result, index + int(char_bit_size<H>), t...);
}

template <typename CharT>
struct swar_pack_result
{
    swar_int    value;
    swar_int    mask;
    std::size_t count;

    constexpr CharT operator[](std::size_t idx) const
    {
        constexpr auto mask = (swar_int(1) << char_bit_size<CharT>)-1;
        return (value >> idx * char_bit_size<CharT>)&mask;
    }
};

// Returns a swar_int containing the specified characters.
// If more are provided than fit, will only take the first couple ones.
template <int SkipFirstNChars = 0, typename... CharT>
constexpr auto swar_pack(CharT... cs)
{
    using char_type = std::common_type_t<CharT...>;
    swar_pack_result<char_type> result{0, 0, 0};

    _swar_pack(result.value, -SkipFirstNChars * int(char_bit_size<char_type>), cs...);

    auto count = int(sizeof...(CharT)) - SkipFirstNChars;
    if (count <= 0)
    {
        result.mask  = 0;
        result.count = 0;
    }
    else if (count >= int(swar_length<char_type>))
    {
        result.mask  = swar_int(-1);
        result.count = swar_length<char_type>;
    }
    else
    {
        result.mask  = swar_int(swar_int(1) << count * int(char_bit_size<char_type>)) - 1;
        result.count = std::size_t(count);
    }

    return result;
}

// Returns the index of the char that is different between lhs and rhs.
template <typename CharT>
constexpr std::size_t swar_find_difference(swar_int lhs, swar_int rhs)
{
    if (lhs == rhs)
        return swar_length<CharT>;

    auto mask = lhs ^ rhs;

#if defined(__GNUC__)
    auto bit_idx = __builtin_ctzll(mask);
#elif defined(_MSC_VER)
    unsigned long bit_idx;
    if (!_BitScanForward64(&bit_idx, mask))
        bit_idx         = 64;
#else
#    error "unsupported compiler; please file an issue"
#endif

    return std::size_t(bit_idx) / char_bit_size<CharT>;
}

// Returns true if v has a char less than N.
template <typename CharT, CharT N>
constexpr bool swar_has_char_less(swar_int v)
{
    // https://graphics.stanford.edu/~seander/bithacks.html#HasLessInWord

    constexpr auto offset      = swar_fill(CharT(N));
    auto           zero_or_msb = v - offset;

    constexpr auto msb_mask = swar_fill(CharT(1 << (char_bit_size<CharT> - 1)));
    auto           not_msb  = ~v & msb_mask;

    return zero_or_msb & not_msb;
}

// Returns true if v has a zero char.
template <typename CharT>
constexpr bool swar_has_zero(swar_int v)
{
    return swar_has_char_less<CharT, 1>(v);
}

// Returns true if v contains the specified char.
template <typename CharT, CharT C>
constexpr bool swar_has_char(swar_int v)
{
    if constexpr (C == 0)
    {
        return swar_has_zero<CharT>(v);
    }
    else
    {
        constexpr auto mask = swar_fill(C);
        return swar_has_zero<CharT>(v ^ mask);
    }
}
} // namespace lexy::_detail

namespace lexy::_detail
{
struct _swar_base
{};
template <typename Reader>
constexpr auto is_swar_reader = std::is_base_of_v<_swar_base, Reader>;

template <typename Derived>
class swar_reader_base : _swar_base
{
public:
    swar_int peek_swar() const
    {
        auto ptr = static_cast<const Derived&>(*this).position();

        swar_int result;
#if LEXY_IS_LITTLE_ENDIAN
        std::memcpy(&result, ptr, sizeof(swar_int));
#else
        using char_type = typename Derived::encoding::char_type;
        auto dst        = reinterpret_cast<char*>(&result);
        auto length     = sizeof(swar_int) / sizeof(char_type);
        for (auto i = 0u; i != length; ++i)
        {
            std::memcpy(dst + i, ptr + length - i - 1, sizeof(char_type));
        }
#endif
        return result;
    }

    void bump_swar()
    {
        auto ptr = static_cast<Derived&>(*this).position();
        ptr += swar_length<typename Derived::encoding::char_type>;
        static_cast<Derived&>(*this).reset({ptr});
    }
    void bump_swar(std::size_t char_count)
    {
        auto ptr = static_cast<Derived&>(*this).position();
        ptr += char_count;
        static_cast<Derived&>(*this).reset({ptr});
    }
};

constexpr std::size_t round_size_for_swar(std::size_t size_in_bytes)
{
    // We round up to the next multiple.
    if (auto remainder = size_in_bytes % sizeof(swar_int); remainder > 0)
        size_in_bytes += sizeof(swar_int) - remainder;
    // Then add one extra space of padding on top.
    size_in_bytes += sizeof(swar_int);
    return size_in_bytes;
}
} // namespace lexy::_detail

#endif // LEXY_DETAIL_SWAR_HPP_INCLUDED

