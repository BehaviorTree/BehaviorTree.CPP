// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CASE_FOLDING_HPP_INCLUDED
#define LEXY_DSL_CASE_FOLDING_HPP_INCLUDED

#include <lexy/_detail/code_point.hpp>
#include <lexy/code_point.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>

//=== generic rule impl ===//
namespace lexyd
{
template <template <typename> typename CaseFolding>
struct _cfl_folding
{
    template <typename Reader>
    using reader = CaseFolding<Reader>;
};

template <typename Literal, template <typename> typename CaseFolding>
struct _cfl : token_base<_cfl<Literal, CaseFolding>>, _lit_base
{
    static constexpr auto lit_max_char_count = Literal::lit_max_char_count;

    static constexpr auto lit_char_classes = Literal::lit_char_classes;

    using lit_case_folding = _cfl_folding<CaseFolding>;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        return Literal::template lit_first_char<Encoding>();
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos,
                                                 std::size_t char_class)
    {
        return Literal::lit_insert(trie, pos, char_class);
    }

    template <typename Reader>
    struct tp
    {
        lexy::token_parser_for<Literal, CaseFolding<Reader>> impl;
        typename Reader::marker                              end;

        constexpr explicit tp(const Reader& reader)
        : impl(CaseFolding<Reader>{reader}), end(reader.current())
        {}

        constexpr bool try_parse(Reader _reader)
        {
            CaseFolding<Reader> reader{_reader};
            auto                result = impl.try_parse(reader);
            end                        = impl.end;
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, Reader reader)
        {
            impl.report_error(context, CaseFolding<Reader>{reader});
        }
    };
};
} // namespace lexyd

namespace lexy
{
template <typename Literal, template <typename> typename CaseFolding>
constexpr auto token_kind_of<lexy::dsl::_cfl<Literal, CaseFolding>> = lexy::literal_token_kind;
} // namespace lexy

//=== ASCII ===//
namespace lexy
{
template <typename Reader>
struct _acfr // ascii case folding reader
{
    Reader _impl;

    using encoding = typename Reader::encoding;
    using iterator = typename Reader::iterator;
    using marker   = typename Reader::marker;

    constexpr auto peek() const -> typename encoding::int_type
    {
        auto c = _impl.peek();
        if (encoding::to_int_type('A') <= c && c <= encoding::to_int_type('Z'))
            return typename encoding::int_type(c + encoding::to_int_type('a' - 'A'));
        else
            return c;
    }

    constexpr void bump()
    {
        _impl.bump();
    }

    constexpr iterator position() const
    {
        return _impl.position();
    }

    constexpr marker current() const noexcept
    {
        return _impl.current();
    }
    constexpr void reset(marker m) noexcept
    {
        _impl.reset(m);
    }
};
} // namespace lexy

namespace lexyd::ascii
{
struct _cf_dsl
{
    template <typename Encoding>
    static constexpr auto is_inplace = true;

    template <typename Reader>
    using case_folding = lexy::_acfr<Reader>;

    template <typename Literal>
    constexpr auto operator()(Literal) const
    {
        static_assert(lexy::is_literal_rule<Literal>);
        static_assert(std::is_void_v<typename Literal::lit_case_folding>, "cannot case fold twice");
        return _cfl<Literal, case_folding>{};
    }
};

/// Matches Literal with case insensitive ASCII characters.
inline constexpr auto case_folding = _cf_dsl{};
} // namespace lexyd::ascii

//=== Unicode ===//
namespace lexy
{
template <typename Reader>
struct _sucfr32 // simple unicode case folding reader, UTF-32
{
    Reader _impl;

    constexpr explicit _sucfr32(Reader impl) : _impl(impl) {}

    using encoding = typename Reader::encoding;
    using iterator = typename Reader::iterator;
    using marker   = typename Reader::marker;

    constexpr auto peek() const -> typename encoding::int_type
    {
        auto c = _impl.peek();
        return lexy::simple_case_fold(lexy::code_point(c)).value();
    }

    constexpr void bump()
    {
        _impl.bump();
    }

    constexpr iterator position() const
    {
        return _impl.position();
    }

    constexpr marker current() const noexcept
    {
        return _impl.current();
    }
    constexpr void reset(marker m) noexcept
    {
        _impl.reset(m);
    }
};

template <typename Reader>
struct _sucfrm // simple unicode case folding reader, UTF-8 and UTF-16
{
    using encoding = typename Reader::encoding;
    using iterator = typename Reader::iterator;
    using marker   = typename Reader::marker;

    Reader                       _impl;
    typename Reader::marker      _cur_pos;
    typename encoding::char_type _buffer[4];
    unsigned char                _buffer_size;
    unsigned char                _buffer_cur;

    constexpr explicit _sucfrm(Reader impl)
    : _impl(impl), _cur_pos(_impl.current()), _buffer{}, _buffer_size(0), _buffer_cur(0)
    {
        _fill();
    }

    constexpr void _fill()
    {
        _cur_pos = _impl.current();

        // We need to read the next code point at this point.
        auto result = lexy::_detail::parse_code_point(_impl);
        if (result.error == lexy::_detail::cp_error::success)
        {
            // Fill the buffer with the folded code point.
            auto folded  = lexy::simple_case_fold(lexy::code_point(result.cp));
            _buffer_size = static_cast<unsigned char>(
                lexy::_detail::encode_code_point<encoding>(folded.value(), _buffer, 4));
            _buffer_cur = 0;
            _impl.reset(result.end);
        }
        else
        {
            // Fill the buffer with the partial code point.
            _buffer_cur = _buffer_size = 0;
            while (_impl.position() != result.end.position())
            {
                _buffer[_buffer_size] = static_cast<typename encoding::char_type>(_impl.peek());
                ++_buffer_size;
                _impl.bump();
            }
        }
    }

    constexpr auto peek() const -> typename encoding::int_type
    {
        if (_buffer_cur == _buffer_size)
            return encoding::eof();

        auto cur = _buffer[_buffer_cur];
        return encoding::to_int_type(cur);
    }

    constexpr void bump()
    {
        ++_buffer_cur;
        if (_buffer_cur == _buffer_size)
            _fill();
    }

    constexpr iterator position() const
    {
        return current().position();
    }

    constexpr marker current() const noexcept
    {
        // We only report a marker at a code point boundary.
        // This has two consequences:
        // 1. If we don't match a rule, the error token does not include any common start code
        //    units. That's actually nice, and makes it unnecessary to handle that situation in the
        //    error reporting. The only relevant difference is in the error token.
        // 2. If the user wants to match partial code unit sequences, the behavior can become buggy.
        //    However, that's not really something we should worry about.
        return _cur_pos;
    }
    constexpr void reset(marker m) noexcept
    {
        _impl.reset(m);
    }
};

template <typename Reader>
using _sucfr_for
    = std::conditional_t<std::is_same_v<typename Reader::encoding, lexy::utf32_encoding>,
                         _sucfr32<Reader>, _sucfrm<Reader>>;

template <typename Reader>
struct _sucfr : _sucfr_for<Reader>
{
    using _sucfr_for<Reader>::_sucfr_for;
};
} // namespace lexy

namespace lexyd::unicode
{
struct _scf_dsl
{
    template <typename Encoding>
    static constexpr auto is_inplace = std::is_same_v<Encoding, lexy::utf32_encoding>;

    template <typename Reader>
    using case_folding = lexy::_sucfr<Reader>;

    template <typename Literal>
    constexpr auto operator()(Literal) const
    {
        static_assert(lexy::is_literal_rule<Literal>);
        static_assert(std::is_void_v<typename Literal::lit_case_folding>, "cannot case fold twice");
        return _cfl<Literal, case_folding>{};
    }
};

/// Matches Literal with case insensitive Unicode characters (simple case folding).
inline constexpr auto simple_case_folding = _scf_dsl{};
} // namespace lexyd::unicode

#endif // LEXY_DSL_CASE_FOLDING_HPP_INCLUDED

