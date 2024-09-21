// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_STRING_INPUT_HPP_INCLUDED
#define LEXY_INPUT_STRING_INPUT_HPP_INCLUDED

#include <lexy/error.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
template <typename View>
using _string_view_char_type = LEXY_DECAY_DECLTYPE(*LEXY_DECLVAL(View).data());

/// An input that refers to a string.
template <typename Encoding = default_encoding>
class string_input
{
    static_assert(lexy::is_char_encoding<Encoding>);

public:
    using encoding  = Encoding;
    using char_type = typename encoding::char_type;

    //=== constructors ===//
    constexpr string_input() noexcept : _data(nullptr), _size(0u) {}

    constexpr string_input(const char_type* data, std::size_t size) noexcept
    : _data(data), _size(size)
    {}
    constexpr string_input(const char_type* begin, const char_type* end) noexcept
    : string_input(begin, std::size_t(end - begin))
    {}

    template <typename CharT, typename = _detail::require_secondary_char_type<Encoding, CharT>>
    string_input(const CharT* data, std::size_t size) noexcept
    : _data(reinterpret_cast<const char_type*>(data)), _size(size)
    {}
    template <typename CharT, typename = _detail::require_secondary_char_type<Encoding, CharT>>
    string_input(const CharT* begin, const CharT* end) noexcept
    : string_input(begin, std::size_t(end - begin))
    {}

    template <typename View, typename CharT = _string_view_char_type<View>>
    constexpr explicit string_input(const View& view) noexcept : _size(view.size())
    {
        if constexpr (std::is_same_v<CharT, char_type>)
        {
            _data = view.data();
        }
        else
        {
            static_assert(Encoding::template is_secondary_char_type<CharT>());
            _data = reinterpret_cast<const char_type*>(view.data());
        }
    }

    //=== access ===//
    constexpr const char_type* data() const noexcept
    {
        return _data;
    }

    constexpr std::size_t size() const noexcept
    {
        return _size;
    }

    //=== reader ===//
    constexpr auto reader() const& noexcept
    {
        return _range_reader<encoding>(_data, _data + _size);
    }

private:
    const char_type* _data;
    std::size_t      _size;
};

template <typename CharT>
string_input(const CharT* begin, const CharT* end) -> string_input<deduce_encoding<CharT>>;
template <typename CharT>
string_input(const CharT* data, std::size_t size) -> string_input<deduce_encoding<CharT>>;
template <typename View>
string_input(const View&) -> string_input<deduce_encoding<_string_view_char_type<View>>>;

template <typename Encoding, typename CharT>
constexpr string_input<Encoding> zstring_input(const CharT* str) noexcept
{
    auto end = str;
    while (*end != CharT())
        ++end;

    return string_input<Encoding>(str, end);
}
template <typename CharT>
constexpr auto zstring_input(const CharT* str) noexcept
{
    return zstring_input<deduce_encoding<CharT>>(str);
}

//=== convenience typedefs ===//
template <typename Encoding = default_encoding>
using string_lexeme = lexeme_for<string_input<Encoding>>;

template <typename Tag, typename Encoding = default_encoding>
using string_error = error_for<string_input<Encoding>, Tag>;

template <typename Encoding = default_encoding>
using string_error_context = error_context<string_input<Encoding>>;
} // namespace lexy

#endif // LEXY_INPUT_STRING_INPUT_HPP_INCLUDED

