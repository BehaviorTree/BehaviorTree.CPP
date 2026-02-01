// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED
#define LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>

namespace lexy::_detail
{
template <typename T, T... Indices>
struct integer_sequence
{
    using type = integer_sequence<T, Indices...>;
};
template <std::size_t... Indices>
using index_sequence = integer_sequence<std::size_t, Indices...>;

#if defined(__clang__)
template <std::size_t Size>
using make_index_sequence = __make_integer_seq<integer_sequence, std::size_t, Size>;
#elif defined(__GNUC__) && __GNUC__ >= 8
template <std::size_t Size>
using make_index_sequence = index_sequence<__integer_pack(Size)...>;
#elif defined(_MSC_VER)
template <std::size_t Size>
using make_index_sequence = __make_integer_seq<integer_sequence, std::size_t, Size>;
#else

// Adapted from https://stackoverflow.com/a/32223343.
template <class Sequence1, class Sequence2>
struct concat_seq;
template <std::size_t... I1, std::size_t... I2>
struct concat_seq<index_sequence<I1...>, index_sequence<I2...>>
{
    using type = index_sequence<I1..., (sizeof...(I1) + I2)...>;
};

template <size_t N>
struct _make_index_sequence : concat_seq<typename _make_index_sequence<N / 2>::type,
                                         typename _make_index_sequence<N - N / 2>::type>
{};
template <>
struct _make_index_sequence<0>
{
    using type = index_sequence<>;
};
template <>
struct _make_index_sequence<1>
{
    using type = index_sequence<0>;
};

template <std::size_t Size>
using make_index_sequence = typename _make_index_sequence<Size>::type;

#endif

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
} // namespace lexy::_detail

#endif // LEXY_DETAIL_INTEGER_SEQUENCE_HPP_INCLUDED

