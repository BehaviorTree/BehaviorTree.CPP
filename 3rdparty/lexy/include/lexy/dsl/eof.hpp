// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_EOF_HPP_INCLUDED
#define LEXY_DSL_EOF_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct expected_eof
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected EOF";
    }
};
} // namespace lexy

namespace lexyd
{
struct _eof : branch_base
{
    template <typename Reader>
    struct bp
    {
        constexpr bool try_parse(const void*, const Reader& reader)
        {
            return reader.peek() == Reader::encoding::eof();
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            auto pos = reader.position();
            context.on(_ev::token{}, lexy::eof_token_kind, pos, pos);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            if (reader.peek() != Reader::encoding::eof())
            {
                // Report that we've failed.
                auto err = lexy::error<Reader, lexy::expected_eof>(reader.position());
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }
            else
            {
                auto pos = reader.position();
                context.on(_ev::token{}, lexy::eof_token_kind, pos, pos);
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Matches EOF.
constexpr auto eof = _eof{};
} // namespace lexyd

#endif // LEXY_DSL_EOF_HPP_INCLUDED

