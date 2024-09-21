// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BOM_HPP_INCLUDED
#define LEXY_DSL_BOM_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Encoding, lexy::encoding_endianness Endianness>
struct _bom : _lit<unsigned char>
{};
template <lexy::encoding_endianness DontCare>
struct _bom<lexy::utf8_encoding, DontCare> //
: _lit<unsigned char, 0xEF, 0xBB, 0xBF>
{};
template <lexy::encoding_endianness DontCare>
struct _bom<lexy::utf8_char_encoding, DontCare> //
: _lit<unsigned char, 0xEF, 0xBB, 0xBF>
{};
template <>
struct _bom<lexy::utf16_encoding, lexy::encoding_endianness::little>
: _lit<unsigned char, 0xFF, 0xFE>
{};
template <>
struct _bom<lexy::utf16_encoding, lexy::encoding_endianness::big> //
: _lit<unsigned char, 0xFE, 0xFF>
{};
template <>
struct _bom<lexy::utf32_encoding, lexy::encoding_endianness::little>
: _lit<unsigned char, 0xFF, 0xFE, 0x00, 0x00>
{};
template <>
struct _bom<lexy::utf32_encoding, lexy::encoding_endianness::big>
: _lit<unsigned char, 0x00, 0x00, 0xFE, 0xFF>
{};

/// The BOM for that particular encoding.
template <typename Encoding, lexy::encoding_endianness Endianness>
inline constexpr auto bom = _bom<Encoding, Endianness>{};
} // namespace lexyd

#endif // LEXY_DSL_BOM_HPP_INCLUDED

