// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_UNTIL_HPP_INCLUDED
#define LEXY_DSL_UNTIL_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Condition>
struct _until_eof : token_base<_until_eof<Condition>, unconditional_branch_base>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr std::true_type try_parse(Reader reader)
        {
            while (true)
            {
                // Check whether we've reached the end of the input or the condition.
                // Note that we're checking for EOF before the condition.
                // This is a potential optimization: as we're accepting EOF anyway, we don't need to
                // enter Condition's parsing logic.
                if (reader.peek() == Reader::encoding::eof()
                    || lexy::try_match_token(Condition{}, reader))
                {
                    // It did, so we're done.
                    break;
                }

                // It did not match, consume one code unit and try again.
                reader.bump();
            }

            end = reader.position();
            return {};
        }
    };
};

template <typename Condition>
struct _until : token_base<_until<Condition>>
{
    template <typename Reader>
    struct tp
    {
        typename Reader::iterator end;

        constexpr explicit tp(const Reader& reader) : end(reader.position()) {}

        constexpr bool try_parse(Reader reader)
        {
            while (true)
            {
                // Try to parse the condition.
                if (lexy::try_match_token(Condition{}, reader))
                {
                    // It did match, we're done at that end.
                    end = reader.position();
                    return true;
                }

                // Check whether we've reached the end of the input.
                // We need to do it after checking for condition, as the condition might just accept
                // EOF.
                if (reader.peek() == Reader::encoding::eof())
                {
                    // It did, so we did not succeed.
                    end = reader.position();
                    return false;
                }

                // It did not match, consume one code unit and try again.
                reader.bump();
            }

            return false; // unreachable
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            // We need to trigger the error `Condition` would.
            // As such, we try parsing it, which will report an error.

            reader.set_position(end);
            LEXY_ASSERT(reader.peek() == Reader::encoding::eof(),
                        "forgot to set end in try_parse()");

            lexy::token_parser_for<Condition, Reader> parser(reader);
            auto                                      result = parser.try_parse(reader);
            LEXY_ASSERT(!result, "condition shouldn't have matched?!");
            parser.report_error(context, reader);
        }
    };

    /// Also accepts EOF as the closing condition.
    constexpr auto or_eof() const
    {
        return _until_eof<Condition>{};
    }
};

/// Matches anything until Condition matches.
/// Then matches Condition.
template <typename Condition>
constexpr auto until(Condition)
{
    static_assert(lexy::is_token_rule<Condition>);
    return _until<Condition>{};
}
} // namespace lexyd

namespace lexy
{
template <typename Condition>
constexpr auto token_kind_of<lexy::dsl::_until_eof<Condition>> = lexy::any_token_kind;
template <typename Condition>
constexpr auto token_kind_of<lexy::dsl::_until<Condition>> = lexy::any_token_kind;
} // namespace lexy

#endif // LEXY_DSL_UNTIL_HPP_INCLUDED

