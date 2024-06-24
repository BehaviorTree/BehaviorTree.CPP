// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LOOKAHEAD_HPP_INCLUDED
#define LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/error.hpp>

namespace lexy
{
/// We've failed to match a lookahead.
struct lookahead_failure
{
    static LEXY_CONSTEVAL auto name()
    {
        return "lookahead failure";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Encoding, typename... Needle, typename... End>
LEXY_CONSTEVAL auto _build_look_trie(_lset<Needle...>, _lset<End...>)
{
    auto result     = lexy::_detail::make_empty_trie<Encoding, Needle..., End...>();
    auto char_class = std::size_t(0);

    // We insert all needles with value 0.
    ((result.node_value[Needle::lit_insert(result, 0, char_class)] = 0,
      char_class += Needle::lit_char_classes.size),
     ...);

    // And all ends with value 1.
    ((result.node_value[End::lit_insert(result, 0, char_class)] = 1,
      char_class += End::lit_char_classes.size),
     ...);

    return result;
}
template <typename Encoding, typename Needle, typename End>
static constexpr auto _look_trie
    = _build_look_trie<Encoding>(typename Needle::as_lset{}, typename End::as_lset{});

template <typename Needle, typename End, typename Tag>
struct _look : branch_base
{
    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);

        typename Reader::iterator begin;
        typename Reader::iterator end;

        constexpr bool try_parse(const void*, Reader reader)
        {
            begin = reader.position();

            auto result = [&] {
                using matcher = lexy::_detail::lit_trie_matcher<
                    _look_trie<typename Reader::encoding, Needle, End>, 0>;

                while (true)
                {
                    auto result = matcher::try_match(reader);
                    if (result == 0)
                        // We've found the needle.
                        return true;
                    else if (result == 1 || reader.peek() == Reader::encoding::eof())
                        // We've failed.
                        return false;
                    else
                        // Try again.
                        reader.bump();
                }

                return false; // unreachable
            }();

            end = reader.position();
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            context.on(_ev::backtracked{}, begin, end);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            context.on(_ev::backtracked{}, begin, end);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_char_encoding<typename Reader::encoding>);
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                // Report that we've failed.
                using tag = lexy::_detail::type_or<Tag, lexy::lookahead_failure>;
                auto err  = lexy::error<Reader, tag>(impl.begin, impl.end);
                context.on(_ev::error{}, err);

                // But recover immediately, as we wouldn't have consumed anything either way.
            }

            context.on(_ev::backtracked{}, impl.begin, impl.end);
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Error>
    static constexpr _look<Needle, End, Error> error = {};
};

/// Looks for the Needle before End.
/// Used as condition to implement arbitrary lookahead.
template <typename Needle, typename End>
constexpr auto lookahead(Needle _needle, End _end)
{
    auto needle = literal_set() / _needle;
    auto end    = literal_set() / _end;
    return _look<decltype(needle), decltype(end), void>{};
}
} // namespace lexyd

#endif // LEXY_DSL_LOOKAHEAD_HPP_INCLUDED

