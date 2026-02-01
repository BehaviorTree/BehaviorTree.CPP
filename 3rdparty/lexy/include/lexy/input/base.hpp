// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_BASE_HPP_INCLUDED
#define LEXY_INPUT_BASE_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/encoding.hpp>

namespace lexy
{
// A generic reader from an iterator range.
template <typename Encoding, typename Iterator, typename Sentinel = Iterator>
class _rr
{
public:
    using encoding = Encoding;
    using iterator = Iterator;

    struct marker
    {
        iterator _it;

        constexpr iterator position() const noexcept
        {
            return _it;
        }
    };

    constexpr explicit _rr(Iterator begin, Sentinel end) noexcept : _cur(begin), _end(end)
    {
        LEXY_PRECONDITION(lexy::_detail::precedes(begin, end));
    }

    constexpr auto peek() const noexcept
    {
        if (_cur == _end)
            return encoding::eof();
        else
            return encoding::to_int_type(static_cast<typename encoding::char_type>(*_cur));
    }

    constexpr void bump() noexcept
    {
        LEXY_PRECONDITION(_cur != _end);
        ++_cur;
    }

    constexpr iterator position() const noexcept
    {
        return _cur;
    }

    constexpr marker current() const noexcept
    {
        return {_cur};
    }
    constexpr void reset(marker m) noexcept
    {
        LEXY_PRECONDITION(lexy::_detail::precedes(m._it, _end));
        _cur = m._it;
    }

private:
    Iterator                   _cur;
    LEXY_EMPTY_MEMBER Sentinel _end;
};

// A special version where the iterators are pointers.
template <typename Encoding>
LEXY_INSTANTIATION_NEWTYPE(_pr, _rr, Encoding, const typename Encoding::char_type*);

// Aliases for the most common encodings.
LEXY_INSTANTIATION_NEWTYPE(_prd, _pr, lexy::default_encoding);
LEXY_INSTANTIATION_NEWTYPE(_pr8, _pr, lexy::utf8_encoding);
LEXY_INSTANTIATION_NEWTYPE(_prc, _pr, lexy::utf8_char_encoding);
LEXY_INSTANTIATION_NEWTYPE(_prb, _pr, lexy::byte_encoding);

template <typename Encoding, typename Iterator, typename Sentinel>
constexpr auto _range_reader(Iterator begin, Sentinel end)
{
    if constexpr (std::is_pointer_v<Iterator>)
    {
        if constexpr (std::is_same_v<Encoding, lexy::default_encoding>)
            return _prd(begin, end);
        else if constexpr (std::is_same_v<Encoding, lexy::utf8_encoding>)
            return _pr8(begin, end);
        else if constexpr (std::is_same_v<Encoding, lexy::utf8_char_encoding>)
            return _prc(begin, end);
        else if constexpr (std::is_same_v<Encoding, lexy::byte_encoding>)
            return _prb(begin, end);
        else
            return _pr<Encoding>(begin, end);
    }
    else
    {
        return _rr<Encoding, Iterator, Sentinel>(begin, end);
    }
}
} // namespace lexy

namespace lexy
{
template <typename Input>
using input_reader = decltype(LEXY_DECLVAL(Input).reader());

template <typename Input>
constexpr bool input_is_view = std::is_trivially_copyable_v<Input>;

template <typename Reader, typename CharT>
constexpr bool char_type_compatible_with_reader
    = (std::is_same_v<CharT, typename Reader::encoding::char_type>)
      || Reader::encoding::template is_secondary_char_type<CharT>();
} // namespace lexy

namespace lexy
{
template <typename Reader>
struct _partial_input
{
    Reader _reader;

    constexpr explicit _partial_input(Reader r) : _reader(r) {}

    constexpr Reader reader() const&
    {
        return _reader;
    }
};

template <typename Reader>
constexpr auto partial_input(const Reader&, typename Reader::iterator begin,
                             typename Reader::iterator end)
{
    return _partial_input(_range_reader<typename Reader::encoding>(begin, end));
}

/// Creates an input that only reads until the given end.
template <typename Reader>
constexpr auto partial_input(const Reader& reader, typename Reader::iterator end)
{
    return partial_input(reader, reader.position(), end);
}
} // namespace lexy

#endif // LEXY_INPUT_BASE_HPP_INCLUDED

