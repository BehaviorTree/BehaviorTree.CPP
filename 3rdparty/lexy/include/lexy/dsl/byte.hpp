// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_BYTE_HPP_INCLUDED
#define LEXY_DSL_BYTE_HPP_INCLUDED

#include <cstdint>
#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/token.hpp>

//=== byte ===//
namespace lexyd
{
template <std::size_t N, typename Predicate>
struct _b : token_base<_b<N, Predicate>>
{
    static_assert(N > 0);

    static constexpr bool _match(lexy::byte_encoding::int_type cur)
    {
        if (cur == lexy::byte_encoding::eof())
            return false;

        if constexpr (!std::is_void_v<Predicate>)
        {
            constexpr auto predicate = Predicate{};
            return predicate(static_cast<lexy::byte_encoding::char_type>(cur));
        }
        else
        {
            return true;
        }
    }

    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<N>>
    struct tp;

    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            // Bump N times.
            auto result
                = ((_match(reader.peek()) ? (reader.bump(), true) : ((void)Idx, false)) && ...);
            end = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader&)
        {
            constexpr auto name
                = std::is_void_v<Predicate> ? "byte" : lexy::_detail::type_name<Predicate>();
            auto err = lexy::error<Reader, lexy::expected_char_class>(end.position(), name);
            context.on(_ev::error{}, err);
        }
    };

    //=== dsl ===//
    template <typename P>
    constexpr auto if_() const
    {
        static_assert(std::is_void_v<Predicate>);
        return _b<N, P>{};
    }

    template <unsigned char Low, unsigned char High>
    constexpr auto range() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "byte.range";
            }

            constexpr bool operator()(unsigned char byte) const
            {
                return Low <= byte && byte <= High;
            }
        };

        return if_<predicate>();
    }

    template <unsigned char... Bytes>
    constexpr auto set() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "byte.set";
            }

            constexpr bool operator()(unsigned char byte) const
            {
                return ((byte == Bytes) || ...);
            }
        };

        return if_<predicate>();
    }

    constexpr auto ascii() const
    {
        struct predicate
        {
            static LEXY_CONSTEVAL auto name()
            {
                return "byte.ASCII";
            }

            constexpr bool operator()(unsigned char byte) const
            {
                return byte <= 0x7F;
            }
        };

        return if_<predicate>();
    }
};

/// Matches an arbitrary byte.
constexpr auto byte = _b<1, void>{};

/// Matches N arbitrary bytes.
template <std::size_t N>
constexpr auto bytes = _b<N, void>{};
} // namespace lexyd

namespace lexy
{
template <std::size_t N>
constexpr auto token_kind_of<lexy::dsl::_b<N, void>> = lexy::any_token_kind;
} // namespace lexy

//=== padding bytes ===//
namespace lexyd
{
template <std::size_t N, unsigned char Padding = 0>
struct _pb : branch_base
{
    template <typename Context, typename Reader, typename Iterator>
    static constexpr void _validate(Context& context, const Reader&, Iterator begin, Iterator end)
    {
        constexpr unsigned char str[] = {Padding, 0};
        for (auto iter = begin; iter != end; ++iter)
        {
            if (*iter != Padding)
            {
                auto err = lexy::error<Reader, lexy::expected_literal>(iter, str, 0, 1);
                context.on(_ev::error{}, err);
            }
        }
    }

    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<_b<N, void>, Reader> parser(reader);
            auto                                        result = parser.try_parse(reader);
            end                                                = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::token{}, lexy::any_token_kind, begin, end.position());
            reader.reset(end);

            _validate(context, reader, begin, end.position());
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
            auto begin = reader.position();
            if (!_b<N, void>::token_parse(context, reader))
                return false;
            auto end = reader.position();

            _validate(context, reader, begin, end);
            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)...);
        }
    };
};

/// Matches N bytes set to the padding value.
/// It is a recoverable error if the byte doesn't have that value.
template <std::size_t N, unsigned char Padding = 0>
constexpr auto padding_bytes = _pb<N, Padding>{};
} // namespace lexyd

//=== bint ===//
namespace lexy::_detail
{
enum bint_endianness
{
    bint_little,
    bint_big,

#if LEXY_IS_LITTLE_ENDIAN
    bint_native = bint_little,
#else
    bint_native = bint_big,
#endif
};

template <std::size_t N>
auto _bint()
{
    if constexpr (N == 1)
        return std::uint_least8_t(0);
    else if constexpr (N == 2)
        return std::uint_least16_t(0);
    else if constexpr (N == 4)
        return std::uint_least32_t(0);
    else if constexpr (N == 8)
        return std::uint_least64_t(0);
    else
    {
        static_assert(N == 1, "invalid byte integer size");
        return 0;
    }
}

template <std::size_t N>
using bint = decltype(_bint<N>());
} // namespace lexy::_detail

namespace lexy
{
struct mismatched_byte_count
{
    static LEXY_CONSTEVAL auto name()
    {
        return "mismatched byte count";
    }
};
} // namespace lexy

namespace lexyd
{
template <std::size_t N, int Endianness, typename Rule = void>
struct _bint : branch_base
{
    using _rule = lexy::_detail::type_or<Rule, _b<N, void>>;

    template <typename NextParser, typename Indices = lexy::_detail::make_index_sequence<N>>
    struct _pc;

    template <typename NextParser, std::size_t... Idx>
    struct _pc<NextParser, lexy::_detail::index_sequence<Idx...>>
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader,
                                           typename Reader::iterator begin,
                                           typename Reader::iterator end, Args&&... args)
        {
            if (lexy::_detail::range_size(begin, end) != N)
            {
                auto err = lexy::error<Reader, lexy::mismatched_byte_count>(begin, end);
                context.on(_ev::error{}, err);
                return false;
            }

            lexy::_detail::bint<N> result = 0;
            if constexpr (N == 1)
            {
                // For a single byte, endianness doesn't matter.
                result = static_cast<unsigned char>(*begin);
            }
            else if constexpr (Endianness == lexy::_detail::bint_big)
            {
                // In big endian, the first byte is shifted the most,
                // so read in order.
                ((result = static_cast<decltype(result)>(result << 8),
                  result = static_cast<decltype(result)>(result | *begin++), (void)Idx),
                 ...);
            }
            else
            {
                // In little endian, we need to reverse the order,
                // so copy into a buffer to do that.
                unsigned char buffer[N] = {((void)Idx, *begin++)...};
                ((result = static_cast<decltype(result)>(result << 8),
                  result = static_cast<decltype(result)>(result | buffer[N - Idx - 1])),
                 ...);
            }

            return lexy::whitespace_parser<Context, NextParser>::parse(context, reader,
                                                                       LEXY_FWD(args)..., result);
        }
    };

    template <typename Reader>
    struct bp
    {
        static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
        typename Reader::marker end;

        constexpr auto try_parse(const void*, const Reader& reader)
        {
            lexy::token_parser_for<_rule, Reader> parser(reader);
            auto                                  result = parser.try_parse(reader);
            end                                          = parser.end;
            return result;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC auto finish(Context& context, Reader& reader, Args&&... args)
        {
            auto begin = reader.position();
            context.on(_ev::token{}, _rule{}, begin, end.position());
            reader.reset(end);

            return _pc<NextParser>::parse(context, reader, begin, end.position(),
                                          LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(lexy::is_byte_encoding<typename Reader::encoding>);
            auto begin = reader.position();
            if (!_rule::token_parse(context, reader))
                return false;
            auto end = reader.position();
            return _pc<NextParser>::parse(context, reader, begin, end, LEXY_FWD(args)...);
        }
    };

    //=== dsl ===//
    /// Matches a specific token rule instead of arbitrary bytes.
    template <typename Token>
    constexpr auto operator()(Token) const
    {
        static_assert(lexy::is_token_rule<Token>);
        static_assert(std::is_void_v<Rule>);
        return _bint<N, Endianness, Token>{};
    }
};

/// Matches one byte and converts it into an 8-bit integer.
inline constexpr auto bint8 = _bint<1, lexy::_detail::bint_native>{};

/// Matches two bytes and converts it into an 16-bit integer.
inline constexpr auto bint16        = _bint<2, lexy::_detail::bint_native>{};
inline constexpr auto little_bint16 = _bint<2, lexy::_detail::bint_little>{};
inline constexpr auto big_bint16    = _bint<2, lexy::_detail::bint_big>{};

/// Matches four bytes and converts it into an 32-bit integer.
inline constexpr auto bint32        = _bint<4, lexy::_detail::bint_native>{};
inline constexpr auto little_bint32 = _bint<4, lexy::_detail::bint_little>{};
inline constexpr auto big_bint32    = _bint<4, lexy::_detail::bint_big>{};

/// Matches eight bytes and converts it into an 64-bit integer.
inline constexpr auto bint64        = _bint<8, lexy::_detail::bint_native>{};
inline constexpr auto little_bint64 = _bint<8, lexy::_detail::bint_little>{};
inline constexpr auto big_bint64    = _bint<8, lexy::_detail::bint_big>{};
} // namespace lexyd

#endif // LEXY_DSL_BYTE_HPP_INCLUDED
