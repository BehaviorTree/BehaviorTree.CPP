// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ERROR_HPP_INCLUDED
#define LEXY_ERROR_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/base.hpp>

namespace lexy
{
template <typename Reader, typename Tag>
class error;

/// Type erased generic failure.
template <typename Reader>
class error<Reader, void>
{
public:
    constexpr explicit error(typename Reader::iterator pos, const char* msg) noexcept
    : _pos(pos), _end(pos), _msg(msg)
    {}
    constexpr explicit error(typename Reader::iterator begin, typename Reader::iterator end,
                             const char* msg) noexcept
    : _pos(begin), _end(end), _msg(msg)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, void>() const noexcept
    {
        return error<OtherReader, void>(_pos, _end, _msg);
    }

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* message() const noexcept
    {
        return _msg;
    }
    template <typename Tag>
    constexpr bool is(Tag = {}) const noexcept
    {
        return _detail::string_view(_msg) == _detail::type_name<Tag>();
    }

    constexpr auto begin() const noexcept
    {
        return _pos;
    }
    constexpr auto end() const noexcept
    {
        return _end;
    }

private:
    typename Reader::iterator _pos;
    typename Reader::iterator _end;
    const char*               _msg;
};

/// Generic failure.
template <typename Reader, typename Tag>
class error : public error<Reader, void>
{
public:
    constexpr explicit error(typename Reader::iterator pos) noexcept
    : error<Reader, void>(pos, _detail::type_name<Tag>())
    {}
    constexpr explicit error(typename Reader::iterator begin,
                             typename Reader::iterator end) noexcept
    : error<Reader, void>(begin, end, _detail::type_name<Tag>())
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, Tag>() const noexcept
    {
        return error<OtherReader, Tag>(this->begin(), this->end());
    }
};

/// Expected the literal character sequence.
struct expected_literal
{};
template <typename Reader>
class error<Reader, expected_literal>
{
public:
    constexpr explicit error(typename Reader::iterator                   pos,
                             const typename Reader::encoding::char_type* str, std::size_t index,
                             std::size_t length) noexcept
    : _pos(pos), _str(str), _idx(index), _length(length)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, expected_literal>() const noexcept
    {
        return error<OtherReader, expected_literal>(_pos, _str, _idx, _length);
    }

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr auto string() const noexcept -> const typename Reader::encoding::char_type*
    {
        return _str;
    }

    constexpr std::size_t index() const noexcept
    {
        return _idx;
    }

    constexpr std::size_t length() const noexcept
    {
        return _length;
    }

    constexpr auto character() const noexcept
    {
        return _str[_idx];
    }

private:
    typename Reader::iterator                   _pos;
    const typename Reader::encoding::char_type* _str;
    std::size_t                                 _idx, _length;
};

/// Expected the given keyword.
/// Unlike expected_literal, this one looks at the following characters as well.
struct expected_keyword
{};
template <typename Reader>
class error<Reader, expected_keyword>
{
public:
    constexpr explicit error(typename Reader::iterator begin, typename Reader::iterator end,
                             const typename Reader::encoding::char_type* str, std::size_t length)
    : _begin(begin), _end(end), _str(str), _length(length)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, expected_keyword>() const noexcept
    {
        return error<OtherReader, expected_keyword>(_begin, _end, _str, _length);
    }

    constexpr auto position() const noexcept
    {
        return _begin;
    }

    constexpr auto begin() const noexcept
    {
        return _begin;
    }
    constexpr auto end() const noexcept
    {
        return _end;
    }

    constexpr auto string() const noexcept -> const typename Reader::encoding::char_type*
    {
        return _str;
    }

    constexpr std::size_t length() const noexcept
    {
        return _length;
    }

private:
    typename Reader::iterator                   _begin;
    typename Reader::iterator                   _end;
    const typename Reader::encoding::char_type* _str;
    std::size_t                                 _length;
};

/// Expected a character of the specified character class.
struct expected_char_class
{};
template <typename Reader>
class error<Reader, expected_char_class>
{
public:
    constexpr explicit error(typename Reader::iterator pos, const char* name) noexcept
    : _pos(pos), _name(name)
    {}

    template <typename OtherReader, typename = std::enable_if_t<std::is_same_v<
                                        typename Reader::iterator, typename OtherReader::iterator>>>
    constexpr operator error<OtherReader, expected_char_class>() const noexcept
    {
        return error<OtherReader, expected_char_class>(_pos, _name);
    }

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* name() const noexcept
    {
        return _name;
    }

private:
    typename Reader::iterator _pos;
    const char*               _name;
};

template <typename Input, typename Tag>
using error_for = error<input_reader<Input>, Tag>;
} // namespace lexy

namespace lexy
{
template <typename Input>
using _detect_parent_input = decltype(LEXY_DECLVAL(Input).parent_input());

/// Contains information about the context of an error, production is type-erased.
template <typename Input>
class error_context
{
public:
    constexpr explicit error_context(lexy::production_info production, const Input& input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : _input(&input), _pos(pos), _production(production.name)
    {}

    /// The input.
    constexpr const auto& input() const noexcept
    {
        if constexpr (_detail::is_detected<_detect_parent_input, Input>)
            return _input->parent_input();
        else
            return *_input;
    }

    /// The name of the production where the error occurred.
    const char* production() const noexcept
    {
        return _production;
    }

    /// The starting position of the production.
    constexpr auto position() const noexcept
    {
        return _pos;
    }

private:
    const Input*                           _input;
    typename input_reader<Input>::iterator _pos;
    const char*                            _production;
};
} // namespace lexy

#endif // LEXY_ERROR_HPP_INCLUDED

