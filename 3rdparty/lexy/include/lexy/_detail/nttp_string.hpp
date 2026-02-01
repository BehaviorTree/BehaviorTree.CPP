// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED
#define LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED

#include <lexy/_detail/assert.hpp>
#include <lexy/_detail/config.hpp>
#include <lexy/encoding.hpp>

namespace lexy::_detail
{
// Note: we can't use type_string<auto...>, it doesn't work on older GCC.
template <typename CharT, CharT... Cs>
struct type_string
{
    using char_type = CharT;

    template <template <typename C, C...> typename T>
    using rename = T<CharT, Cs...>;

    static constexpr auto size = sizeof...(Cs);

    template <typename T = char_type>
    static constexpr T c_str[sizeof...(Cs) + 1] = {transcode_char<T>(Cs)..., T()};
};
} // namespace lexy::_detail

#if LEXY_HAS_NTTP // string NTTP implementation

#    include <lexy/_detail/integer_sequence.hpp>

namespace lexy::_detail
{
template <std::size_t N, typename CharT>
struct string_literal
{
    CharT data[N];

    using char_type = CharT;

    LEXY_CONSTEVAL string_literal(const CharT* str) : data{}
    {
        for (auto i = 0u; i != N; ++i)
            data[i] = str[i];
    }
    LEXY_CONSTEVAL string_literal(CharT c) : data{}
    {
        data[0] = c;
    }

    static LEXY_CONSTEVAL auto size()
    {
        return N;
    }
};
template <std::size_t N, typename CharT>
string_literal(const CharT (&)[N]) -> string_literal<N - 1, CharT>;
template <typename CharT>
string_literal(CharT) -> string_literal<1, CharT>;

template <template <typename C, C... Cs> typename T, string_literal Str, std::size_t... Idx>
auto _to_type_string(index_sequence<Idx...>)
{
    return T<typename decltype(Str)::char_type, Str.data[Idx]...>{};
}
template <template <typename C, C... Cs> typename T, string_literal Str>
using to_type_string
    = decltype(_to_type_string<T, Str>(make_index_sequence<decltype(Str)::size()>{}));
} // namespace lexy::_detail

#    define LEXY_NTTP_STRING(T, Str)                                                               \
        ::lexy::_detail::to_type_string<T, ::lexy::_detail::string_literal(Str)>

#elif defined(__GNUC__) // literal implementation

#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#    ifdef __clang__
#        pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#    endif

template <typename CharT, CharT... Cs>
constexpr ::lexy::_detail::type_string<CharT, Cs...> operator""_lexy_string_udl()
{
    return {};
}

#    define LEXY_NTTP_STRING(T, Str) decltype(Str##_lexy_string_udl)::rename<T>

#    pragma GCC diagnostic pop

#else // string<Cs...> macro implementation

namespace lexy::_detail
{
template <typename A, typename B>
struct cat_;
template <typename CharT, CharT... C1, CharT... C2>
struct cat_<type_string<CharT, C1...>, type_string<CharT, C2...>>
{
    using type = type_string<CharT, C1..., C2...>;
};
template <typename A, typename B>
using cat = typename cat_<A, B>::type;

template <template <typename CharT, CharT...> typename T, typename TypeString, std::size_t Size,
          std::size_t MaxSize>
struct macro_type_string
{
    static_assert(Size <= MaxSize, "string out of range");
    using type = typename TypeString::template rename<T>;
};

} // namespace lexy::_detail

#    define LEXY_NTTP_STRING_LENGTH(Str) (sizeof(Str) / sizeof(Str[0]) - 1)

// extract Ith character if not out of bounds
#    define LEXY_NTTP_STRING1(Str, I)                                                              \
        ::std::conditional_t<                                                                      \
            (I < LEXY_NTTP_STRING_LENGTH(Str)),                                                    \
            ::lexy::_detail::type_string<::LEXY_DECAY_DECLTYPE(Str[0]),                            \
                                         (I >= LEXY_NTTP_STRING_LENGTH(Str) ? Str[0] : Str[I])>,   \
            ::lexy::_detail::type_string<::LEXY_DECAY_DECLTYPE(Str[0])>>

// recursively split the string in two
#    define LEXY_NTTP_STRING2(Str, I)                                                              \
        ::lexy::_detail::cat<LEXY_NTTP_STRING1(Str, I), LEXY_NTTP_STRING1(Str, I + 1)>
#    define LEXY_NTTP_STRING4(Str, I)                                                              \
        ::lexy::_detail::cat<LEXY_NTTP_STRING2(Str, I), LEXY_NTTP_STRING2(Str, I + 2)>
#    define LEXY_NTTP_STRING8(Str, I)                                                              \
        ::lexy::_detail::cat<LEXY_NTTP_STRING4(Str, I), LEXY_NTTP_STRING4(Str, I + 4)>
#    define LEXY_NTTP_STRING16(Str, I)                                                             \
        ::lexy::_detail::cat<LEXY_NTTP_STRING8(Str, I), LEXY_NTTP_STRING8(Str, I + 8)>
#    define LEXY_NTTP_STRING32(Str, I)                                                             \
        ::lexy::_detail::cat<LEXY_NTTP_STRING16(Str, I), LEXY_NTTP_STRING16(Str, I + 16)>

// instantiate with overflow check
#    define LEXY_NTTP_STRING(T, Str)                                                               \
        ::lexy::_detail::macro_type_string<T, LEXY_NTTP_STRING32(Str, 0),                          \
                                           LEXY_NTTP_STRING_LENGTH(Str), 32>::type

#endif

#endif // LEXY_DETAIL_NTTP_STRING_HPP_INCLUDED

