// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_LOCATION_HPP_INCLUDED
#define LEXY_INPUT_LOCATION_HPP_INCLUDED

#include <lexy/dsl/code_point.hpp>
#include <lexy/dsl/newline.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

//=== input_location_anchor ===//
namespace lexy
{
/// Anchor for the location search.
template <typename Input>
struct input_location_anchor
{
    using marker = typename lexy::input_reader<Input>::marker;

    constexpr explicit input_location_anchor(const Input& input)
    : _line_begin(input.reader().current()), _line_nr(1)
    {}

    // implementation detail
    constexpr explicit input_location_anchor(marker line_begin, unsigned line_nr)
    : _line_begin(line_begin), _line_nr(line_nr)
    {}

    marker   _line_begin;
    unsigned _line_nr;
};
} // namespace lexy

//=== counting strategies ===//
namespace lexy
{
/// Counts code units for columns, newlines for lines.
class code_unit_location_counting
{
public:
    template <typename Reader>
    constexpr bool try_match_newline(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        return lexy::try_match_token(lexy::dsl::newline, reader);
    }

    template <typename Reader>
    constexpr void match_column(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        reader.bump();
    }
};

/// Counts code points for columns, newlines for lines.
class code_point_location_counting
{
public:
    template <typename Reader>
    constexpr bool try_match_newline(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        return lexy::try_match_token(lexy::dsl::newline, reader);
    }

    template <typename Reader>
    constexpr void match_column(Reader& reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        if (!lexy::try_match_token(lexy::dsl::code_point, reader))
            reader.bump();
    }
};

/// Counts bytes for columns, lines end after LineWidth bytes.
template <std::size_t LineWidth = 16>
class byte_location_counting
{
public:
    template <typename Reader>
    constexpr bool try_match_newline(Reader& reader)
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        LEXY_PRECONDITION(_cur_index <= LineWidth - 1);
        if (_cur_index == LineWidth - 1)
        {
            // Consider the last byte to be the "newline".
            // We need to consume something if possible;
            // the logic in the function breaks otherwise.
            if (reader.peek() != Reader::encoding::eof())
                reader.bump();
            _cur_index = 0;
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename Reader>
    constexpr void match_column(Reader& reader)
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);

        reader.bump();
        ++_cur_index;
    }

private:
    std::size_t _cur_index = 0;
};

template <typename Input>
auto _compute_default_location_counting()
{
    using encoding = typename lexy::input_reader<Input>::encoding;
    if constexpr (lexy::is_byte_encoding<encoding>)
        return byte_location_counting{};
    else if constexpr (lexy::is_char_encoding<encoding>)
        return code_unit_location_counting{};
    else
        static_assert(_detail::error<Input>,
                      "input encoding does not have a default location counting policy");
}

template <typename Input>
using _default_location_counting = decltype(_compute_default_location_counting<Input>());
} // namespace lexy

//=== input_location ===//
namespace lexy
{
/// A location in the input.
template <typename Input, typename Counting = _default_location_counting<Input>>
class input_location
{
    using iterator = typename lexy::input_reader<Input>::iterator;
    using marker   = typename lexy::input_reader<Input>::marker;

public:
    constexpr explicit input_location(const Input& input)
    : _line_begin(input.reader().current()), _column_begin(_line_begin.position()), _line_nr(1),
      _column_nr(1)
    {}

    /// The closest previous anchor.
    constexpr input_location_anchor<Input> anchor() const
    {
        return input_location_anchor<Input>(_line_begin, _line_nr);
    }

    constexpr unsigned line_nr() const
    {
        return _line_nr;
    }
    constexpr unsigned column_nr() const
    {
        return _column_nr;
    }

    /// The corresponding position, rounded down to the previous column start.
    constexpr iterator position() const
    {
        return _column_begin;
    }

    friend constexpr bool operator==(const input_location& lhs, const input_location& rhs)
    {
        return lhs._line_nr == rhs._line_nr && lhs._column_nr == rhs._column_nr;
    }
    friend constexpr bool operator!=(const input_location& lhs, const input_location& rhs)
    {
        return !(lhs == rhs);
    }

    friend constexpr bool operator<(const input_location& lhs, const input_location& rhs)
    {
        if (lhs._line_nr != rhs._line_nr)
            return lhs._line_nr < rhs._line_nr;
        return lhs._column_nr < rhs._column_nr;
    }
    friend constexpr bool operator<=(const input_location& lhs, const input_location& rhs)
    {
        return !(rhs < lhs);
    }
    friend constexpr bool operator>(const input_location& lhs, const input_location& rhs)
    {
        return rhs < lhs;
    }
    friend constexpr bool operator>=(const input_location& lhs, const input_location& rhs)
    {
        return !(rhs > lhs);
    }

private:
    constexpr input_location(marker line_begin, unsigned line_nr, iterator column_begin,
                             unsigned column_nr)
    : _line_begin(line_begin), _column_begin(column_begin), _line_nr(line_nr), _column_nr(column_nr)
    {}

    marker   _line_begin;
    iterator _column_begin;
    unsigned _line_nr, _column_nr;

    template <typename C, typename I>
    friend constexpr auto get_input_location(const I&                                 input,
                                             typename lexy::input_reader<I>::iterator position,
                                             input_location_anchor<I>                 anchor)
        -> input_location<I, C>;
};

/// The location for a position in the input; search starts at the anchor.
template <typename Counting, typename Input>
constexpr auto get_input_location(const Input&                                 input,
                                  typename lexy::input_reader<Input>::iterator position,
                                  input_location_anchor<Input>                 anchor)
    -> input_location<Input, Counting>
{
    auto reader = input.reader();
    reader.reset(anchor._line_begin);

    auto line_begin   = anchor._line_begin;
    auto line_nr      = anchor._line_nr;
    auto column_begin = line_begin;
    auto column_nr    = 1u;

    Counting counting;
    while (true)
    {
        if (reader.position() == position)
        {
            // We've already found the position; it's at the beginning of a colum nor newline.
            // No need to do the expensive checks.
            //
            // This also allows `lexy_ext::shell` to work properly, if position is at EOF,
            // the reader.peek() call will ask for more input.
            break;
        }
        else if (reader.peek() == lexy::input_reader<Input>::encoding::eof())
        {
            LEXY_ASSERT(false, "invalid position + anchor combination");
        }
        else if (counting.try_match_newline(reader))
        {
            // [column_begin, newline_end) covers the newline.
            auto newline_end = reader.current();
            if (lexy::_detail::min_range_end(column_begin.position(), newline_end.position(),
                                             position)
                != newline_end.position())
                break;

            // Advance to the next line.
            ++line_nr;
            line_begin   = newline_end;
            column_nr    = 1;
            column_begin = line_begin;
        }
        else
        {
            counting.match_column(reader);

            // [column_begin, column_end) covers the column.
            auto column_end = reader.current();
            if (lexy::_detail::min_range_end(column_begin.position(), column_end.position(),
                                             position)
                != column_end.position())
                break;

            // Advance to the next column.
            ++column_nr;
            column_begin = column_end;
        }
    }

    return {line_begin, line_nr, column_begin.position(), column_nr};
}

template <typename Counting, typename Input>
constexpr auto get_input_location(const Input&                                 input,
                                  typename lexy::input_reader<Input>::iterator position)
{
    return get_input_location<Counting>(input, position, input_location_anchor(input));
}
template <typename Input>
constexpr auto get_input_location(const Input&                                 input,
                                  typename lexy::input_reader<Input>::iterator position,
                                  input_location_anchor<Input>                 anchor)
{
    return get_input_location<_default_location_counting<Input>>(input, position, anchor);
}
template <typename Input>
constexpr auto get_input_location(const Input&                                 input,
                                  typename lexy::input_reader<Input>::iterator position)
{
    return get_input_location<_default_location_counting<Input>>(input, position,
                                                                 input_location_anchor(input));
}
} // namespace lexy

//=== input_line_annotation ===//
namespace lexy::_detail
{
template <typename Counting, typename Input>
constexpr auto get_input_line(const Input&                               input,
                              typename lexy::input_reader<Input>::marker line_begin)
{
    auto reader = input.reader();
    reader.reset(line_begin);

    auto line_end = reader.position();
    for (Counting counting;
         reader.peek() != decltype(reader)::encoding::eof() && !counting.try_match_newline(reader);
         line_end = reader.position())
    {
        counting.match_column(reader);
    }
    auto newline_end = reader.position();

    struct result_t
    {
        lexy::lexeme_for<Input> line;
        lexy::lexeme_for<Input> newline;
    };
    return result_t{{line_begin.position(), line_end}, {line_end, newline_end}};
}

// Advances the iterator to the beginning of the next code point.
template <typename Encoding, typename Iterator>
constexpr Iterator find_cp_boundary(Iterator cur, Iterator end)
{
    auto is_cp_continuation = [](auto c) {
        if constexpr (std::is_same_v<Encoding,
                                     lexy::utf8_encoding> //
                      || std::is_same_v<Encoding, lexy::utf8_char_encoding>)
            return (c & 0b1100'0000) == (0b10 << 6);
        else if constexpr (std::is_same_v<Encoding, lexy::utf16_encoding>)
            return 0xDC00 <= c && c <= 0xDFFF;
        else
        {
            // This encoding doesn't have continuation code units.
            (void)c;
            return std::false_type{};
        }
    };

    while (cur != end && is_cp_continuation(*cur))
        ++cur;
    return cur;
}
} // namespace lexy::_detail

namespace lexy
{
template <typename Input>
struct input_line_annotation
{
    /// Everything of the line before the range.
    lexy::lexeme_for<Input> before;
    /// The annotated part.
    lexy::lexeme_for<Input> annotated;
    /// Everything of the line after the annotated range.
    lexy::lexeme_for<Input> after;

    /// true if the the range was spanning multiple line and needed to be truncated.
    bool truncated_multiline;
    /// true if annotated includes the newline (this implies after.empty())
    bool annotated_newline;
    /// true if end needed to be moved to a code point boundary.
    bool rounded_end;
};

template <typename Input>
constexpr void _get_input_line_annotation(input_line_annotation<Input>&                result,
                                          lexy::lexeme_for<Input>                      line,
                                          lexy::lexeme_for<Input>                      newline,
                                          typename lexy::input_reader<Input>::iterator begin,
                                          typename lexy::input_reader<Input>::iterator end)
{
    // At this point there are two cases:
    // Either line.begin() <= begin < end <= newline.end()),
    // or line.begin() <= begin == end == newline.end().

    // We then round end to the code point boundary.
    // Note that we don't round begin.
    {
        auto old_end = end;

        using encoding = typename lexy::input_reader<Input>::encoding;
        end            = _detail::find_cp_boundary<encoding>(end, newline.end());

        result.rounded_end = end != old_end;
    }

    // Now we can compute the annotation.
    if (lexy::_detail::min_range_end(line.begin(), line.end(), end) == end)
    {
        // We have end <= line.end(),
        // so line.end() is the end of after.
        result.before    = {line.begin(), begin};
        result.annotated = {begin, end};
        result.after     = {end, line.end()};
    }
    else
    {
        // We have end > line.end(),
        // so newline.end() is the end of annotated.
        result.before            = {line.begin(), begin};
        result.annotated         = {begin, newline.end()};
        result.after             = {newline.end(), newline.end()};
        result.annotated_newline = true;
    }
}

template <typename Input, typename Counting>
constexpr auto get_input_line_annotation(const Input&                           input,
                                         const input_location<Input, Counting>& begin_location,
                                         typename lexy::input_reader<Input>::iterator end)
    -> input_line_annotation<Input>
{
    input_line_annotation<Input> result{};

    auto [line, newline]
        = _detail::get_input_line<Counting>(input, begin_location.anchor()._line_begin);

    // We first normalize the range.
    auto begin = begin_location.position();
    if (begin == end)
    {
        if (end == newline.begin())
        {
            // Empty range at the newline; make it cover the entire newline.
            end = newline.end();
        }
        else if (end != newline.end())
        {
            // Empty range before end of newline; extend by one code unit.
            ++end;
        }
        else
        {
            // begin == end == newline.end()
        }
    }
    else if (lexy::_detail::min_range_end(begin, end, newline.end()) != end)
    {
        // Truncate a multiline range to a single line.
        // Note that we can't have both an empty range and a multiline range.
        end                        = newline.end();
        result.truncated_multiline = true;
    }

    _get_input_line_annotation(result, line, newline, begin, end);
    return result;
}

/// Computes the annotation for e.g. an error message covering [location, location + size).
template <typename Input, typename Counting>
constexpr auto get_input_line_annotation(const Input&                           input,
                                         const input_location<Input, Counting>& location,
                                         std::size_t                            size)
{
    input_line_annotation<Input> result{};
    auto [line, newline] = _detail::get_input_line<Counting>(input, location.anchor()._line_begin);

    // We don't want an empty annotation.
    auto range_size = size == 0 ? 1 : size;

    auto begin = location.position();
    auto end   = _detail::next_clamped(location.position(), range_size, newline.end());
    if (_detail::range_size(location.position(), end) < size)
    {
        // We didn't have enough of the current line to match the size request.
        // As such, we needed to truncate it.
        result.truncated_multiline = true;
    }

    _get_input_line_annotation(result, line, newline, begin, end);
    return result;
}
} // namespace lexy

#endif // LEXY_INPUT_LOCATION_HPP_INCLUDED

