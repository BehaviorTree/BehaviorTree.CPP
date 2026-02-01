// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_CALLBACK_STRING_HPP_INCLUDED
#define LEXY_CALLBACK_STRING_HPP_INCLUDED

#include <lexy/_detail/code_point.hpp>
#include <lexy/callback/base.hpp>
#include <lexy/code_point.hpp>
#include <lexy/encoding.hpp>
#include <lexy/input/base.hpp>
#include <lexy/lexeme.hpp>

namespace lexy
{
struct nullopt;

template <typename String>
using _string_char_type = LEXY_DECAY_DECLTYPE(LEXY_DECLVAL(String)[0]);

template <typename String, typename Encoding, typename CaseFoldingDSL = void>
struct _as_string
{
    using return_type = String;
    using _char_type  = _string_char_type<String>;
    static_assert(lexy::_detail::is_compatible_char_type<Encoding, _char_type>,
                  "invalid character type/encoding combination");

    static constexpr String&& _case_folding(String&& str)
    {
        if constexpr (std::is_void_v<CaseFoldingDSL>)
        {
            return LEXY_MOV(str);
        }
        else if constexpr (CaseFoldingDSL::template is_inplace<Encoding>)
        {
            // We can change the string in place.
            auto original_reader = lexy::_range_reader<Encoding>(str.begin(), str.end());
            auto reader = typename CaseFoldingDSL::template case_folding<decltype(original_reader)>{
                original_reader};
            for (auto ptr = str.data(); true; ++ptr)
            {
                auto cur = reader.peek();
                if (cur == Encoding::eof())
                    break;
                reader.bump();

                // Once we've bumped it, we're not looking at it again.
                *ptr = static_cast<_char_type>(cur);
            }

            return LEXY_MOV(str);
        }
        else
        {
            // We store the existing string somewhere else and clear it.
            // Then we can read the case folded string and append each code unit.
            auto original = LEXY_MOV(str);
            str           = String();
            str.reserve(original.size());

            auto original_reader = lexy::_range_reader<Encoding>(original.begin(), original.end());
            auto reader = typename CaseFoldingDSL::template case_folding<decltype(original_reader)>{
                original_reader};
            while (true)
            {
                auto cur = reader.peek();
                if (cur == Encoding::eof())
                    break;
                str.push_back(static_cast<_char_type>(cur));
                reader.bump();
            }

            return LEXY_MOV(str);
        }
    }

    template <typename NewCaseFoldingDSL>
    constexpr auto case_folding(NewCaseFoldingDSL) const
    {
        return _as_string<String, Encoding, NewCaseFoldingDSL>{};
    }

    constexpr String operator()(nullopt&&) const
    {
        return String();
    }
    constexpr String&& operator()(String&& str) const
    {
        return _case_folding(LEXY_MOV(str));
    }

    template <typename Iterator>
    constexpr auto operator()(Iterator begin, Iterator end) const -> decltype(String(begin, end))
    {
        return _case_folding(String(begin, end));
    }
    template <typename Str = String, typename Iterator>
    constexpr auto operator()(const typename Str::allocator_type& allocator, Iterator begin,
                              Iterator end) const -> decltype(String(begin, end, allocator))
    {
        return _case_folding(String(begin, end, allocator));
    }

    template <typename Reader>
    constexpr String operator()(lexeme<Reader> lex) const
    {
        static_assert(lexy::char_type_compatible_with_reader<Reader, _char_type>,
                      "cannot convert lexeme to this string type");

        using iterator = typename lexeme<Reader>::iterator;
        if constexpr (std::is_convertible_v<iterator, const _char_type*>)
            return _case_folding(String(lex.data(), lex.size()));
        else
            return _case_folding(String(lex.begin(), lex.end()));
    }
    template <typename Str = String, typename Reader>
    constexpr String operator()(const typename Str::allocator_type& allocator,
                                lexeme<Reader>                      lex) const
    {
        static_assert(lexy::char_type_compatible_with_reader<Reader, _char_type>,
                      "cannot convert lexeme to this string type");

        using iterator = typename lexeme<Reader>::iterator;
        if constexpr (std::is_convertible_v<iterator, const _char_type*>)
            return _case_folding(String(lex.data(), lex.size(), allocator));
        else
            return _case_folding(String(lex.begin(), lex.end(), allocator));
    }

    constexpr String operator()(code_point cp) const
    {
        typename Encoding::char_type buffer[4] = {};
        auto size = _detail::encode_code_point<Encoding>(cp.value(), buffer, 4);
        return _case_folding(String(buffer, buffer + size));
    }
    template <typename Str = String>
    constexpr String operator()(const typename Str::allocator_type& allocator, code_point cp) const
    {
        typename Encoding::char_type buffer[4] = {};
        auto size = _detail::encode_code_point<Encoding>(cp.value(), buffer, 4);
        return _case_folding(String(buffer, buffer + size, allocator));
    }

    struct _sink
    {
        String _result;

        using return_type = String;

        template <typename CharT, typename = decltype(LEXY_DECLVAL(String).push_back(CharT()))>
        constexpr void operator()(CharT c)
        {
            _result.push_back(c);
        }

        constexpr void operator()(String&& str)
        {
            _result.append(LEXY_MOV(str));
        }

        template <typename Str = String, typename Iterator>
        constexpr auto operator()(Iterator begin, Iterator end)
            -> decltype(void(LEXY_DECLVAL(Str).append(begin, end)))
        {
            _result.append(begin, end);
        }

        template <typename Reader>
        constexpr void operator()(lexeme<Reader> lex)
        {
            static_assert(lexy::char_type_compatible_with_reader<Reader, _char_type>,
                          "cannot convert lexeme to this string type");
            _result.append(lex.begin(), lex.end());
        }

        constexpr void operator()(code_point cp)
        {
            typename Encoding::char_type buffer[4] = {};
            auto size = _detail::encode_code_point<Encoding>(cp.value(), buffer, 4);
            _result.append(buffer, buffer + size);
        }

        constexpr String&& finish() &&
        {
            return _case_folding(LEXY_MOV(_result));
        }
    };

    constexpr auto sink() const
    {
        return _sink{String()};
    }
    template <typename S = String>
    constexpr auto sink(const typename S::allocator_type& allocator) const
    {
        return _sink{String(allocator)};
    }
};

/// A callback with sink that creates a string (e.g. `std::string`).
/// As a callback, it converts a lexeme into the string.
/// As a sink, it repeatedly calls `.push_back()` for individual characters,
/// or `.append()` for lexemes or other strings.
template <typename String, typename Encoding = deduce_encoding<_string_char_type<String>>>
constexpr auto as_string = _as_string<String, Encoding>{};
} // namespace lexy

#endif // LEXY_CALLBACK_STRING_HPP_INCLUDED

