// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ANY_HPP_INCLUDED
#define LEXY_DSL_ANY_HPP_INCLUDED

#include <lexy/_detail/swar.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
struct _any : token_base<_any, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr std::true_type try_parse(Reader reader)
        {
            using encoding = typename Reader::encoding;
            if constexpr (lexy::_detail::is_swar_reader<Reader>)
            {
                while (!lexy::_detail::swar_has_char<typename encoding::char_type, encoding::eof()>(
                    reader.peek_swar()))
                    reader.bump_swar();
            }

            while (reader.peek() != encoding::eof())
                reader.bump();

            end = reader.current();
            return {};
        }
    };
};

/// Matches anything and consumes all remaining characters.
constexpr auto any = _any{};
} // namespace lexyd

namespace lexy
{
template <>
inline constexpr auto token_kind_of<lexy::dsl::_any> = lexy::any_token_kind;
}

#endif // LEXY_DSL_ANY_HPP_INCLUDED

