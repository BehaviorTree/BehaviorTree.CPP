// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_INPUT_LEXEME_INPUT_HPP_INCLUDED
#define LEXY_INPUT_LEXEME_INPUT_HPP_INCLUDED

#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
template <typename ParentInput>
class lexeme_input
{
    using _lexeme_type = lexeme_for<ParentInput>;
    using _input_ptr
        = std::conditional_t<input_is_view<ParentInput>, ParentInput, const ParentInput*>;

public:
    using encoding  = typename _lexeme_type::encoding;
    using char_type = typename _lexeme_type::char_type;
    using iterator  = typename _lexeme_type::iterator;

    //=== constructor ===//
    explicit lexeme_input(const ParentInput& input, _lexeme_type lexeme)
    : _input([&] {
          if constexpr (input_is_view<ParentInput>)
              return input;
          else
              return &input;
      }()),
      _lexeme(lexeme)
    {}

    explicit lexeme_input(const ParentInput& input, iterator begin, iterator end)
    : lexeme_input(input, _lexeme_type(begin, end))
    {}

    //=== access ===//
    const ParentInput& parent_input() const
    {
        if constexpr (input_is_view<ParentInput>)
            return _input;
        else
            return *_input;
    }

    _lexeme_type lexeme() const
    {
        return _lexeme;
    }

    //=== input ===//
    auto reader() const& noexcept
    {
        return _range_reader<encoding>(_lexeme.begin(), _lexeme.end());
    }

private:
    _input_ptr   _input;
    _lexeme_type _lexeme;
};
} // namespace lexy

#endif // LEXY_INPUT_LEXEME_INPUT_HPP_INCLUDED

