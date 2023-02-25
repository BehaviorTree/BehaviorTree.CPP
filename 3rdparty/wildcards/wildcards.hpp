// THIS FILE HAS BEEN GENERATED AUTOMATICALLY. DO NOT EDIT DIRECTLY.
// Generated: 2019-03-08 09:59:35.958950200
// Copyright Tomas Zeman 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef WILDCARDS_HPP
#define WILDCARDS_HPP 
#define WILDCARDS_VERSION_MAJOR 1
#define WILDCARDS_VERSION_MINOR 5
#define WILDCARDS_VERSION_PATCH 0
#ifndef WILDCARDS_CARDS_HPP
#define WILDCARDS_CARDS_HPP 
#include <utility>
namespace wildcards
{
template <typename T>
struct cards
{
constexpr cards(T a, T s, T e)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{false},
alt_enabled{false}
{
}
constexpr cards(T a, T s, T e, T so, T sc, T sn, T ao, T ac, T ar)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{true},
set_open{std::move(so)},
set_close{std::move(sc)},
set_not{std::move(sn)},
alt_enabled{true},
alt_open{std::move(ao)},
alt_close{std::move(ac)},
alt_or{std::move(ar)}
{
}
T anything;
T single;
T escape;
bool set_enabled;
T set_open;
T set_close;
T set_not;
bool alt_enabled;
T alt_open;
T alt_close;
T alt_or;
};
enum class cards_type
{
standard,
extended
};
template <>
struct cards<char>
{
constexpr cards(cards_type type = cards_type::extended)
: set_enabled{type == cards_type::extended}, alt_enabled{type == cards_type::extended}
{
}
constexpr cards(char a, char s, char e)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{false},
alt_enabled{false}
{
}
constexpr cards(char a, char s, char e, char so, char sc, char sn, char ao, char ac, char ar)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{true},
set_open{std::move(so)},
set_close{std::move(sc)},
set_not{std::move(sn)},
alt_enabled{true},
alt_open{std::move(ao)},
alt_close{std::move(ac)},
alt_or{std::move(ar)}
{
}
char anything{'*'};
char single{'?'};
char escape{'\\'};
bool set_enabled{true};
char set_open{'['};
char set_close{']'};
char set_not{'!'};
bool alt_enabled{true};
char alt_open{'('};
char alt_close{')'};
char alt_or{'|'};
};
template <>
struct cards<char16_t>
{
constexpr cards(cards_type type = cards_type::extended)
: set_enabled{type == cards_type::extended}, alt_enabled{type == cards_type::extended}
{
}
constexpr cards(char16_t a, char16_t s, char16_t e)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{false},
alt_enabled{false}
{
}
constexpr cards(char16_t a, char16_t s, char16_t e, char16_t so, char16_t sc, char16_t sn,
char16_t ao, char16_t ac, char16_t ar)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{true},
set_open{std::move(so)},
set_close{std::move(sc)},
set_not{std::move(sn)},
alt_enabled{true},
alt_open{std::move(ao)},
alt_close{std::move(ac)},
alt_or{std::move(ar)}
{
}
char16_t anything{u'*'};
char16_t single{u'?'};
char16_t escape{u'\\'};
bool set_enabled{true};
char16_t set_open{u'['};
char16_t set_close{u']'};
char16_t set_not{u'!'};
bool alt_enabled{true};
char16_t alt_open{u'('};
char16_t alt_close{u')'};
char16_t alt_or{u'|'};
};
template <>
struct cards<char32_t>
{
constexpr cards(cards_type type = cards_type::extended)
: set_enabled{type == cards_type::extended}, alt_enabled{type == cards_type::extended}
{
}
constexpr cards(char32_t a, char32_t s, char32_t e)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{false},
alt_enabled{false}
{
}
constexpr cards(char32_t a, char32_t s, char32_t e, char32_t so, char32_t sc, char32_t sn,
char32_t ao, char32_t ac, char32_t ar)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{true},
set_open{std::move(so)},
set_close{std::move(sc)},
set_not{std::move(sn)},
alt_enabled{true},
alt_open{std::move(ao)},
alt_close{std::move(ac)},
alt_or{std::move(ar)}
{
}
char32_t anything{U'*'};
char32_t single{U'?'};
char32_t escape{U'\\'};
bool set_enabled{true};
char32_t set_open{U'['};
char32_t set_close{U']'};
char32_t set_not{U'!'};
bool alt_enabled{true};
char32_t alt_open{U'('};
char32_t alt_close{U')'};
char32_t alt_or{U'|'};
};
template <>
struct cards<wchar_t>
{
constexpr cards(cards_type type = cards_type::extended)
: set_enabled{type == cards_type::extended}, alt_enabled{type == cards_type::extended}
{
}
constexpr cards(wchar_t a, wchar_t s, wchar_t e)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{false},
alt_enabled{false}
{
}
constexpr cards(wchar_t a, wchar_t s, wchar_t e, wchar_t so, wchar_t sc, wchar_t sn, wchar_t ao,
wchar_t ac, wchar_t ar)
: anything{std::move(a)},
single{std::move(s)},
escape{std::move(e)},
set_enabled{true},
set_open{std::move(so)},
set_close{std::move(sc)},
set_not{std::move(sn)},
alt_enabled{true},
alt_open{std::move(ao)},
alt_close{std::move(ac)},
alt_or{std::move(ar)}
{
}
wchar_t anything{L'*'};
wchar_t single{L'?'};
wchar_t escape{L'\\'};
bool set_enabled{true};
wchar_t set_open{L'['};
wchar_t set_close{L']'};
wchar_t set_not{L'!'};
bool alt_enabled{true};
wchar_t alt_open{L'('};
wchar_t alt_close{L')'};
wchar_t alt_or{L'|'};
};
template <typename T>
constexpr cards<T> make_cards(T&& a, T&& s, T&& e)
{
return {std::forward<T>(a), std::forward<T>(s), std::forward<T>(e)};
}
template <typename T>
constexpr cards<T> make_cards(T&& a, T&& s, T&& e, T&& so, T&& sc, T&& sn, T&& ao, T&& ac, T&& ar)
{
return {std::forward<T>(a), std::forward<T>(s), std::forward<T>(e),
std::forward<T>(so), std::forward<T>(sc), std::forward<T>(sn),
std::forward<T>(ao), std::forward<T>(ac), std::forward<T>(ar)};
}
}
#endif
#ifndef WILDCARDS_MATCH_HPP
#define WILDCARDS_MATCH_HPP 
#include <stdexcept>
#include <type_traits>
#include <utility>
#ifndef CONFIG_HPP
#define CONFIG_HPP 
#ifndef QUICKCPPLIB_HAS_FEATURE_H
#define QUICKCPPLIB_HAS_FEATURE_H 
#if __cplusplus >= 201103L
#if !defined(__cpp_alias_templates)
#define __cpp_alias_templates 190000
#endif
#if !defined(__cpp_attributes)
#define __cpp_attributes 190000
#endif
#if !defined(__cpp_constexpr)
#if __cplusplus >= 201402L
#define __cpp_constexpr 201304
#else
#define __cpp_constexpr 190000
#endif
#endif
#if !defined(__cpp_decltype)
#define __cpp_decltype 190000
#endif
#if !defined(__cpp_delegating_constructors)
#define __cpp_delegating_constructors 190000
#endif
#if !defined(__cpp_explicit_conversion)
#define __cpp_explicit_conversion 190000
#endif
#if !defined(__cpp_inheriting_constructors)
#define __cpp_inheriting_constructors 190000
#endif
#if !defined(__cpp_initializer_lists)
#define __cpp_initializer_lists 190000
#endif
#if !defined(__cpp_lambdas)
#define __cpp_lambdas 190000
#endif
#if !defined(__cpp_nsdmi)
#define __cpp_nsdmi 190000
#endif
#if !defined(__cpp_range_based_for)
#define __cpp_range_based_for 190000
#endif
#if !defined(__cpp_raw_strings)
#define __cpp_raw_strings 190000
#endif
#if !defined(__cpp_ref_qualifiers)
#define __cpp_ref_qualifiers 190000
#endif
#if !defined(__cpp_rvalue_references)
#define __cpp_rvalue_references 190000
#endif
#if !defined(__cpp_static_assert)
#define __cpp_static_assert 190000
#endif
#if !defined(__cpp_unicode_characters)
#define __cpp_unicode_characters 190000
#endif
#if !defined(__cpp_unicode_literals)
#define __cpp_unicode_literals 190000
#endif
#if !defined(__cpp_user_defined_literals)
#define __cpp_user_defined_literals 190000
#endif
#if !defined(__cpp_variadic_templates)
#define __cpp_variadic_templates 190000
#endif
#endif
#if __cplusplus >= 201402L
#if !defined(__cpp_aggregate_nsdmi)
#define __cpp_aggregate_nsdmi 190000
#endif
#if !defined(__cpp_binary_literals)
#define __cpp_binary_literals 190000
#endif
#if !defined(__cpp_decltype_auto)
#define __cpp_decltype_auto 190000
#endif
#if !defined(__cpp_generic_lambdas)
#define __cpp_generic_lambdas 190000
#endif
#if !defined(__cpp_init_captures)
#define __cpp_init_captures 190000
#endif
#if !defined(__cpp_return_type_deduction)
#define __cpp_return_type_deduction 190000
#endif
#if !defined(__cpp_sized_deallocation)
#define __cpp_sized_deallocation 190000
#endif
#if !defined(__cpp_variable_templates)
#define __cpp_variable_templates 190000
#endif
#endif
#if defined(_MSC_VER) && !defined(__clang__)
#if !defined(__cpp_exceptions) && defined(_CPPUNWIND)
#define __cpp_exceptions 190000
#endif
#if !defined(__cpp_rtti) && defined(_CPPRTTI)
#define __cpp_rtti 190000
#endif
#if !defined(__cpp_alias_templates) && _MSC_VER >= 1800
#define __cpp_alias_templates 190000
#endif
#if !defined(__cpp_attributes)
#define __cpp_attributes 190000
#endif
#if !defined(__cpp_constexpr) && _MSC_FULL_VER >= 190023506
#define __cpp_constexpr 190000
#endif
#if !defined(__cpp_decltype) && _MSC_VER >= 1600
#define __cpp_decltype 190000
#endif
#if !defined(__cpp_delegating_constructors) && _MSC_VER >= 1800
#define __cpp_delegating_constructors 190000
#endif
#if !defined(__cpp_explicit_conversion) && _MSC_VER >= 1800
#define __cpp_explicit_conversion 190000
#endif
#if !defined(__cpp_inheriting_constructors) && _MSC_VER >= 1900
#define __cpp_inheriting_constructors 190000
#endif
#if !defined(__cpp_initializer_lists) && _MSC_VER >= 1900
#define __cpp_initializer_lists 190000
#endif
#if !defined(__cpp_lambdas) && _MSC_VER >= 1600
#define __cpp_lambdas 190000
#endif
#if !defined(__cpp_nsdmi) && _MSC_VER >= 1900
#define __cpp_nsdmi 190000
#endif
#if !defined(__cpp_range_based_for) && _MSC_VER >= 1700
#define __cpp_range_based_for 190000
#endif
#if !defined(__cpp_raw_strings) && _MSC_VER >= 1800
#define __cpp_raw_strings 190000
#endif
#if !defined(__cpp_ref_qualifiers) && _MSC_VER >= 1900
#define __cpp_ref_qualifiers 190000
#endif
#if !defined(__cpp_rvalue_references) && _MSC_VER >= 1600
#define __cpp_rvalue_references 190000
#endif
#if !defined(__cpp_static_assert) && _MSC_VER >= 1600
#define __cpp_static_assert 190000
#endif
#if !defined(__cpp_user_defined_literals) && _MSC_VER >= 1900
#define __cpp_user_defined_literals 190000
#endif
#if !defined(__cpp_variadic_templates) && _MSC_VER >= 1800
#define __cpp_variadic_templates 190000
#endif
#if !defined(__cpp_binary_literals) && _MSC_VER >= 1900
#define __cpp_binary_literals 190000
#endif
#if !defined(__cpp_decltype_auto) && _MSC_VER >= 1900
#define __cpp_decltype_auto 190000
#endif
#if !defined(__cpp_generic_lambdas) && _MSC_VER >= 1900
#define __cpp_generic_lambdas 190000
#endif
#if !defined(__cpp_init_captures) && _MSC_VER >= 1900
#define __cpp_init_captures 190000
#endif
#if !defined(__cpp_return_type_deduction) && _MSC_VER >= 1900
#define __cpp_return_type_deduction 190000
#endif
#if !defined(__cpp_sized_deallocation) && _MSC_VER >= 1900
#define __cpp_sized_deallocation 190000
#endif
#if !defined(__cpp_variable_templates) && _MSC_FULL_VER >= 190023506
#define __cpp_variable_templates 190000
#endif
#endif
#if(defined(__GNUC__) && !defined(__clang__))
#define QUICKCPPLIB_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if !defined(__cpp_exceptions) && defined(__EXCEPTIONS)
#define __cpp_exceptions 190000
#endif
#if !defined(__cpp_rtti) && defined(__GXX_RTTI)
#define __cpp_rtti 190000
#endif
#if defined(__GXX_EXPERIMENTAL_CXX0X__)
#if !defined(__cpp_alias_templates) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_alias_templates 190000
#endif
#if !defined(__cpp_attributes) && (QUICKCPPLIB_GCC >= 40800)
#define __cpp_attributes 190000
#endif
#if !defined(__cpp_constexpr) && (QUICKCPPLIB_GCC >= 40600)
#define __cpp_constexpr 190000
#endif
#if !defined(__cpp_decltype) && (QUICKCPPLIB_GCC >= 40300)
#define __cpp_decltype 190000
#endif
#if !defined(__cpp_delegating_constructors) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_delegating_constructors 190000
#endif
#if !defined(__cpp_explicit_conversion) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_explicit_conversion 190000
#endif
#if !defined(__cpp_inheriting_constructors) && (QUICKCPPLIB_GCC >= 40800)
#define __cpp_inheriting_constructors 190000
#endif
#if !defined(__cpp_initializer_lists) && (QUICKCPPLIB_GCC >= 40800)
#define __cpp_initializer_lists 190000
#endif
#if !defined(__cpp_lambdas) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_lambdas 190000
#endif
#if !defined(__cpp_nsdmi) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_nsdmi 190000
#endif
#if !defined(__cpp_range_based_for) && (QUICKCPPLIB_GCC >= 40600)
#define __cpp_range_based_for 190000
#endif
#if !defined(__cpp_raw_strings) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_raw_strings 190000
#endif
#if !defined(__cpp_ref_qualifiers) && (QUICKCPPLIB_GCC >= 40801)
#define __cpp_ref_qualifiers 190000
#endif
#if !defined(__cpp_rvalue_references) && defined(__cpp_rvalue_reference)
#define __cpp_rvalue_references __cpp_rvalue_reference
#endif
#if !defined(__cpp_static_assert) && (QUICKCPPLIB_GCC >= 40300)
#define __cpp_static_assert 190000
#endif
#if !defined(__cpp_unicode_characters) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_unicode_characters 190000
#endif
#if !defined(__cpp_unicode_literals) && (QUICKCPPLIB_GCC >= 40500)
#define __cpp_unicode_literals 190000
#endif
#if !defined(__cpp_user_defined_literals) && (QUICKCPPLIB_GCC >= 40700)
#define __cpp_user_defined_literals 190000
#endif
#if !defined(__cpp_variadic_templates) && (QUICKCPPLIB_GCC >= 40400)
#define __cpp_variadic_templates 190000
#endif
#endif
#endif
#if defined(__clang__)
#define QUICKCPPLIB_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#if !defined(__cpp_exceptions) && (defined(__EXCEPTIONS) || defined(_CPPUNWIND))
#define __cpp_exceptions 190000
#endif
#if !defined(__cpp_rtti) && (defined(__GXX_RTTI) || defined(_CPPRTTI))
#define __cpp_rtti 190000
#endif
#if defined(__GXX_EXPERIMENTAL_CXX0X__)
#if !defined(__cpp_alias_templates) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_alias_templates 190000
#endif
#if !defined(__cpp_attributes) && (QUICKCPPLIB_CLANG >= 30300)
#define __cpp_attributes 190000
#endif
#if !defined(__cpp_constexpr) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_constexpr 190000
#endif
#if !defined(__cpp_decltype) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_decltype 190000
#endif
#if !defined(__cpp_delegating_constructors) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_delegating_constructors 190000
#endif
#if !defined(__cpp_explicit_conversion) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_explicit_conversion 190000
#endif
#if !defined(__cpp_inheriting_constructors) && (QUICKCPPLIB_CLANG >= 30300)
#define __cpp_inheriting_constructors 190000
#endif
#if !defined(__cpp_initializer_lists) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_initializer_lists 190000
#endif
#if !defined(__cpp_lambdas) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_lambdas 190000
#endif
#if !defined(__cpp_nsdmi) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_nsdmi 190000
#endif
#if !defined(__cpp_range_based_for) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_range_based_for 190000
#endif
#if !defined(__cpp_raw_strings) && defined(__cpp_raw_string_literals)
#define __cpp_raw_strings __cpp_raw_string_literals
#endif
#if !defined(__cpp_raw_strings) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_raw_strings 190000
#endif
#if !defined(__cpp_ref_qualifiers) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_ref_qualifiers 190000
#endif
#if !defined(__cpp_rvalue_references) && defined(__cpp_rvalue_reference)
#define __cpp_rvalue_references __cpp_rvalue_reference
#endif
#if !defined(__cpp_rvalue_references) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_rvalue_references 190000
#endif
#if !defined(__cpp_static_assert) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_static_assert 190000
#endif
#if !defined(__cpp_unicode_characters) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_unicode_characters 190000
#endif
#if !defined(__cpp_unicode_literals) && (QUICKCPPLIB_CLANG >= 30000)
#define __cpp_unicode_literals 190000
#endif
#if !defined(__cpp_user_defined_literals) && defined(__cpp_user_literals)
#define __cpp_user_defined_literals __cpp_user_literals
#endif
#if !defined(__cpp_user_defined_literals) && (QUICKCPPLIB_CLANG >= 30100)
#define __cpp_user_defined_literals 190000
#endif
#if !defined(__cpp_variadic_templates) && (QUICKCPPLIB_CLANG >= 20900)
#define __cpp_variadic_templates 190000
#endif
#endif
#endif
#endif
#define cfg_HAS_CONSTEXPR14 (__cpp_constexpr >= 201304)
#if cfg_HAS_CONSTEXPR14
#define cfg_constexpr14 constexpr
#else
#define cfg_constexpr14 
#endif
#if cfg_HAS_CONSTEXPR14 && defined(__clang__)
#define cfg_HAS_FULL_FEATURED_CONSTEXPR14 1
#else
#define cfg_HAS_FULL_FEATURED_CONSTEXPR14 0
#endif
#endif
#ifndef CX_FUNCTIONAL_HPP
#define CX_FUNCTIONAL_HPP 
#include <utility>
namespace cx
{
template <typename T>
struct less
{
constexpr auto operator()(const T& lhs, const T& rhs) const -> decltype(lhs < rhs)
{
return lhs < rhs;
}
};
template <>
struct less<void>
{
template <typename T, typename U>
constexpr auto operator()(T&& lhs, U&& rhs) const
-> decltype(std::forward<T>(lhs) < std::forward<U>(rhs))
{
return std::forward<T>(lhs) < std::forward<U>(rhs);
}
};
template <typename T>
struct equal_to
{
constexpr auto operator()(const T& lhs, const T& rhs) const -> decltype(lhs == rhs)
{
return lhs == rhs;
}
};
template <>
struct equal_to<void>
{
template <typename T, typename U>
constexpr auto operator()(T&& lhs, U&& rhs) const
-> decltype(std::forward<T>(lhs) == std::forward<U>(rhs))
{
return std::forward<T>(lhs) == std::forward<U>(rhs);
}
};
}
#endif
#ifndef CX_ITERATOR_HPP
#define CX_ITERATOR_HPP 
#include <cstddef>
#include <initializer_list>
namespace cx
{
template <typename It>
constexpr It next(It it)
{
return it + 1;
}
template <typename It>
constexpr It prev(It it)
{
return it - 1;
}
template <typename C>
constexpr auto size(const C& c) -> decltype(c.size())
{
return c.size();
}
template <typename T, std::size_t N>
constexpr std::size_t size(const T (&)[N])
{
return N;
}
template <typename C>
constexpr auto empty(const C& c) -> decltype(c.empty())
{
return c.empty();
}
template <typename T, std::size_t N>
constexpr bool empty(const T (&)[N])
{
return false;
}
template <typename E>
constexpr bool empty(std::initializer_list<E> il)
{
return il.size() == 0;
}
template <typename C>
constexpr auto begin(const C& c) -> decltype(c.begin())
{
return c.begin();
}
template <typename C>
constexpr auto begin(C& c) -> decltype(c.begin())
{
return c.begin();
}
template <typename T, std::size_t N>
constexpr T* begin(T (&array)[N])
{
return &array[0];
}
template <typename E>
constexpr const E* begin(std::initializer_list<E> il)
{
return il.begin();
}
template <typename C>
constexpr auto cbegin(const C& c) -> decltype(cx::begin(c))
{
return cx::begin(c);
}
template <typename C>
constexpr auto end(const C& c) -> decltype(c.end())
{
return c.end();
}
template <typename C>
constexpr auto end(C& c) -> decltype(c.end())
{
return c.end();
}
template <typename T, std::size_t N>
constexpr T* end(T (&array)[N])
{
return &array[N];
}
template <typename E>
constexpr const E* end(std::initializer_list<E> il)
{
return il.end();
}
template <typename C>
constexpr auto cend(const C& c) -> decltype(cx::end(c))
{
return cx::end(c);
}
}
#endif
#ifndef WILDCARDS_UTILITY_HPP
#define WILDCARDS_UTILITY_HPP 
#include <type_traits>
#include <utility>
namespace wildcards
{
template <typename C>
struct const_iterator
{
using type = typename std::remove_cv<
typename std::remove_reference<decltype(cx::cbegin(std::declval<C>()))>::type>::type;
};
template <typename C>
using const_iterator_t = typename const_iterator<C>::type;
template <typename C>
struct iterator
{
using type = typename std::remove_cv<
typename std::remove_reference<decltype(cx::begin(std::declval<C>()))>::type>::type;
};
template <typename C>
using iterator_t = typename iterator<C>::type;
template <typename It>
struct iterated_item
{
using type = typename std::remove_cv<
typename std::remove_reference<decltype(*std::declval<It>())>::type>::type;
};
template <typename It>
using iterated_item_t = typename iterated_item<It>::type;
template <typename C>
struct container_item
{
using type = typename std::remove_cv<
typename std::remove_reference<decltype(*cx::begin(std::declval<C>()))>::type>::type;
};
template <typename C>
using container_item_t = typename container_item<C>::type;
}
#endif
namespace wildcards
{
template <typename SequenceIterator, typename PatternIterator>
struct full_match_result
{
bool res;
SequenceIterator s, send, s1;
PatternIterator p, pend, p1;
constexpr operator bool() const
{
return res;
}
};
namespace detail
{
template <typename SequenceIterator, typename PatternIterator>
struct match_result
{
bool res;
SequenceIterator s;
PatternIterator p;
constexpr operator bool() const
{
return res;
}
};
template <typename SequenceIterator, typename PatternIterator>
constexpr match_result<SequenceIterator, PatternIterator> make_match_result(bool res,
SequenceIterator s,
PatternIterator p)
{
return {std::move(res), std::move(s), std::move(p)};
}
template <typename SequenceIterator, typename PatternIterator>
constexpr full_match_result<SequenceIterator, PatternIterator> make_full_match_result(
SequenceIterator s, SequenceIterator send, PatternIterator p, PatternIterator pend,
match_result<SequenceIterator, PatternIterator> mr)
{
return {std::move(mr.res), std::move(s), std::move(send), std::move(mr.s),
std::move(p), std::move(pend), std::move(mr.p)};
}
#if !cfg_HAS_FULL_FEATURED_CONSTEXPR14
constexpr bool throw_invalid_argument(const char* what_arg)
{
return what_arg == nullptr ? false : throw std::invalid_argument(what_arg);
}
template <typename T>
constexpr T throw_invalid_argument(T t, const char* what_arg)
{
return what_arg == nullptr ? t : throw std::invalid_argument(what_arg);
}
constexpr bool throw_logic_error(const char* what_arg)
{
return what_arg == nullptr ? false : throw std::logic_error(what_arg);
}
template <typename T>
constexpr T throw_logic_error(T t, const char* what_arg)
{
return what_arg == nullptr ? t : throw std::logic_error(what_arg);
}
#endif
enum class is_set_state
{
open,
not_or_first,
first,
next
};
template <typename PatternIterator>
constexpr bool is_set(
PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
is_set_state state = is_set_state::open)
{
#if cfg_HAS_CONSTEXPR14
if (!c.set_enabled)
{
return false;
}
while (p != pend)
{
switch (state)
{
case is_set_state::open:
if (*p != c.set_open)
{
return false;
}
state = is_set_state::not_or_first;
break;
case is_set_state::not_or_first:
if (*p == c.set_not)
{
state = is_set_state::first;
}
else
{
state = is_set_state::next;
}
break;
case is_set_state::first:
state = is_set_state::next;
break;
case is_set_state::next:
if (*p == c.set_close)
{
return true;
}
break;
default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::logic_error(
"The program execution should never end up here throwing this exception");
#else
return throw_logic_error(
"The program execution should never end up here throwing this exception");
#endif
}
p = cx::next(p);
}
return false;
#else
return c.set_enabled && p != pend &&
(state == is_set_state::open
? *p == c.set_open && is_set(cx::next(p), pend, c, is_set_state::not_or_first)
:
state == is_set_state::not_or_first
? *p == c.set_not ? is_set(cx::next(p), pend, c, is_set_state::first)
: is_set(cx::next(p), pend, c, is_set_state::next)
: state == is_set_state::first
? is_set(cx::next(p), pend, c, is_set_state::next)
: state == is_set_state::next
? *p == c.set_close ||
is_set(cx::next(p), pend, c, is_set_state::next)
: throw std::logic_error("The program execution should never end up "
"here throwing this exception"));
#endif
}
enum class set_end_state
{
open,
not_or_first,
first,
next
};
template <typename PatternIterator>
constexpr PatternIterator set_end(
PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
set_end_state state = set_end_state::open)
{
#if cfg_HAS_CONSTEXPR14
if (!c.set_enabled)
{
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The use of sets is disabled");
#else
return throw_invalid_argument(p, "The use of sets is disabled");
#endif
}
while (p != pend)
{
switch (state)
{
case set_end_state::open:
if (*p != c.set_open)
{
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The given pattern is not a valid set");
#else
return throw_invalid_argument(p, "The given pattern is not a valid set");
#endif
}
state = set_end_state::not_or_first;
break;
case set_end_state::not_or_first:
if (*p == c.set_not)
{
state = set_end_state::first;
}
else
{
state = set_end_state::next;
}
break;
case set_end_state::first:
state = set_end_state::next;
break;
case set_end_state::next:
if (*p == c.set_close)
{
return cx::next(p);
}
break;
default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::logic_error(
"The program execution should never end up here throwing this exception");
#else
return throw_logic_error(
p, "The program execution should never end up here throwing this exception");
#endif
}
p = cx::next(p);
}
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The given pattern is not a valid set");
#else
return throw_invalid_argument(p, "The given pattern is not a valid set");
#endif
#else
return !c.set_enabled
? throw std::invalid_argument("The use of sets is disabled")
: p == pend
? throw std::invalid_argument("The given pattern is not a valid set")
:
state == set_end_state::open
? *p == c.set_open
? set_end(cx::next(p), pend, c, set_end_state::not_or_first)
: throw std::invalid_argument("The given pattern is not a valid set")
:
state == set_end_state::not_or_first
? *p == c.set_not ? set_end(cx::next(p), pend, c, set_end_state::first)
: set_end(cx::next(p), pend, c, set_end_state::next)
: state == set_end_state::first
? set_end(cx::next(p), pend, c, set_end_state::next)
: state == set_end_state::next
? *p == c.set_close
? cx::next(p)
: set_end(cx::next(p), pend, c, set_end_state::next)
: throw std::logic_error(
"The program execution should never end up "
"here throwing this exception");
#endif
}
enum class match_set_state
{
open,
not_or_first_in,
first_out,
next_in,
next_out
};
template <typename SequenceIterator, typename PatternIterator,
typename EqualTo = cx::equal_to<void>>
constexpr match_result<SequenceIterator, PatternIterator> match_set(
SequenceIterator s, SequenceIterator send, PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
const EqualTo& equal_to = EqualTo(), match_set_state state = match_set_state::open)
{
#if cfg_HAS_CONSTEXPR14
if (!c.set_enabled)
{
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The use of sets is disabled");
#else
return throw_invalid_argument(make_match_result(false, s, p), "The use of sets is disabled");
#endif
}
while (p != pend)
{
switch (state)
{
case match_set_state::open:
if (*p != c.set_open)
{
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The given pattern is not a valid set");
#else
return throw_invalid_argument(make_match_result(false, s, p),
"The given pattern is not a valid set");
#endif
}
state = match_set_state::not_or_first_in;
break;
case match_set_state::not_or_first_in:
if (*p == c.set_not)
{
state = match_set_state::first_out;
}
else
{
if (s == send)
{
return make_match_result(false, s, p);
}
if (equal_to(*s, *p))
{
return make_match_result(true, s, p);
}
state = match_set_state::next_in;
}
break;
case match_set_state::first_out:
if (s == send || equal_to(*s, *p))
{
return make_match_result(false, s, p);
}
state = match_set_state::next_out;
break;
case match_set_state::next_in:
if (*p == c.set_close || s == send)
{
return make_match_result(false, s, p);
}
if (equal_to(*s, *p))
{
return make_match_result(true, s, p);
}
break;
case match_set_state::next_out:
if (*p == c.set_close)
{
return make_match_result(true, s, p);
}
if (s == send || equal_to(*s, *p))
{
return make_match_result(false, s, p);
}
break;
default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::logic_error(
"The program execution should never end up here throwing this exception");
#else
return throw_logic_error(
make_match_result(false, s, p),
"The program execution should never end up here throwing this exception");
#endif
}
p = cx::next(p);
}
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The given pattern is not a valid set");
#else
return throw_invalid_argument(make_match_result(false, s, p),
"The given pattern is not a valid set");
#endif
#else
return !c.set_enabled
? throw std::invalid_argument("The use of sets is disabled")
: p == pend
? throw std::invalid_argument("The given pattern is not a valid set")
: state == match_set_state::open
? *p == c.set_open
? match_set(s, send, cx::next(p), pend, c, equal_to,
match_set_state::not_or_first_in)
:
throw std::invalid_argument("The given pattern is not a valid set")
:
state == match_set_state::not_or_first_in
? *p == c.set_not
? match_set(s, send, cx::next(p), pend, c, equal_to,
match_set_state::first_out)
:
s == send ? make_match_result(false, s, p)
: equal_to(*s, *p)
? make_match_result(true, s, p)
: match_set(s, send, cx::next(p), pend, c,
equal_to, match_set_state::next_in)
:
state == match_set_state::first_out
? s == send || equal_to(*s, *p)
? make_match_result(false, s, p)
: match_set(s, send, cx::next(p), pend, c, equal_to,
match_set_state::next_out)
:
state == match_set_state::next_in
? *p == c.set_close || s == send
? make_match_result(false, s, p)
: equal_to(*s, *p) ? make_match_result(true, s, p)
: match_set(s, send, cx::next(p),
pend, c, equal_to, state)
:
state == match_set_state::next_out
? *p == c.set_close
? make_match_result(true, s, p)
: s == send || equal_to(*s, *p)
? make_match_result(false, s, p)
: match_set(s, send, cx::next(p), pend, c,
equal_to, state)
: throw std::logic_error(
"The program execution should never end up "
"here "
"throwing this exception");
#endif
}
enum class is_alt_state
{
open,
next,
escape
};
template <typename PatternIterator>
constexpr bool is_alt(
PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
is_alt_state state = is_alt_state::open, int depth = 0)
{
#if cfg_HAS_CONSTEXPR14
if (!c.alt_enabled)
{
return false;
}
while (p != pend)
{
switch (state)
{
case is_alt_state::open:
if (*p != c.alt_open)
{
return false;
}
state = is_alt_state::next;
++depth;
break;
case is_alt_state::next:
if (*p == c.escape)
{
state = is_alt_state::escape;
}
else if (c.set_enabled && *p == c.set_open &&
is_set(cx::next(p), pend, c, is_set_state::not_or_first))
{
p = cx::prev(set_end(cx::next(p), pend, c, set_end_state::not_or_first));
}
else if (*p == c.alt_open)
{
++depth;
}
else if (*p == c.alt_close)
{
--depth;
if (depth == 0)
{
return true;
}
}
break;
case is_alt_state::escape:
state = is_alt_state::next;
break;
default:

throw std::logic_error(
"The program execution should never end up here throwing this exception");

}
p = cx::next(p);
}
return false;
#else
return c.alt_enabled && p != pend &&
(state == is_alt_state::open
? *p == c.alt_open && is_alt(cx::next(p), pend, c, is_alt_state::next, depth + 1)
: state == is_alt_state::next
? *p == c.escape
? is_alt(cx::next(p), pend, c, is_alt_state::escape, depth)
: c.set_enabled && *p == c.set_open &&
is_set(cx::next(p), pend, c, is_set_state::not_or_first)
? is_alt(set_end(cx::next(p), pend, c, set_end_state::not_or_first),
pend, c, state, depth)
: *p == c.alt_open
? is_alt(cx::next(p), pend, c, state, depth + 1)
: *p == c.alt_close
? depth == 1 ||
is_alt(cx::next(p), pend, c, state, depth - 1)
: is_alt(cx::next(p), pend, c, state, depth)
:
state == is_alt_state::escape
? is_alt(cx::next(p), pend, c, is_alt_state::next, depth)
: throw std::logic_error(
"The program execution should never end up here throwing this "
"exception"));
#endif
}
enum class alt_end_state
{
open,
next,
escape
};
template <typename PatternIterator>
constexpr PatternIterator alt_end(
PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
alt_end_state state = alt_end_state::open, int depth = 0)
{
#if cfg_HAS_CONSTEXPR14
if (!c.alt_enabled)
{
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The use of alternatives is disabled");
#else
return throw_invalid_argument(p, "The use of alternatives is disabled");
#endif
}
while (p != pend)
{
switch (state)
{
case alt_end_state::open:
if (*p != c.alt_open)
{
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The given pattern is not a valid alternative");
#else
return throw_invalid_argument(p, "The given pattern is not a valid alternative");
#endif
}
state = alt_end_state::next;
++depth;
break;
case alt_end_state::next:
if (*p == c.escape)
{
state = alt_end_state::escape;
}
else if (c.set_enabled && *p == c.set_open &&
is_set(cx::next(p), pend, c, is_set_state::not_or_first))
{
p = cx::prev(set_end(cx::next(p), pend, c, set_end_state::not_or_first));
}
else if (*p == c.alt_open)
{
++depth;
}
else if (*p == c.alt_close)
{
--depth;
if (depth == 0)
{
return cx::next(p);
}
}
break;
case alt_end_state::escape:
state = alt_end_state::next;
break;
default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::logic_error(
"The program execution should never end up here throwing this exception");
#else
return throw_logic_error(
p, "The program execution should never end up here throwing this exception");
#endif
}
p = cx::next(p);
}
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The given pattern is not a valid alternative");
#else
return throw_invalid_argument(p, "The given pattern is not a valid alternative");
#endif
#else
return !c.alt_enabled
? throw std::invalid_argument("The use of alternatives is disabled")
: p == pend
? throw std::invalid_argument("The given pattern is not a valid alternative")
: state == alt_end_state::open
? *p == c.alt_open
? alt_end(cx::next(p), pend, c, alt_end_state::next, depth + 1)
: throw std::invalid_argument(
"The given pattern is not a valid alternative")
: state == alt_end_state::next
? *p == c.escape
? alt_end(cx::next(p), pend, c, alt_end_state::escape, depth)
: c.set_enabled && *p == c.set_open &&
is_set(cx::next(p), pend, c,
is_set_state::not_or_first)
? alt_end(set_end(cx::next(p), pend, c,
set_end_state::not_or_first),
pend, c, state, depth)
: *p == c.alt_open
? alt_end(cx::next(p), pend, c, state, depth + 1)
: *p == c.alt_close
? depth == 1 ? cx::next(p)
: alt_end(cx::next(p), pend, c,
state, depth - 1)
: alt_end(cx::next(p), pend, c, state, depth)
:
state == alt_end_state::escape
? alt_end(cx::next(p), pend, c, alt_end_state::next, depth)
: throw std::logic_error(
"The program execution should never end up here throwing "
"this "
"exception");
#endif
}
enum class alt_sub_end_state
{
next,
escape
};
template <typename PatternIterator>
constexpr PatternIterator alt_sub_end(
PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
alt_sub_end_state state = alt_sub_end_state::next, int depth = 1)
{
#if cfg_HAS_CONSTEXPR14
if (!c.alt_enabled)
{
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The use of alternatives is disabled");
#else
return throw_invalid_argument(p, "The use of alternatives is disabled");
#endif
}
while (p != pend)
{
switch (state)
{
case alt_sub_end_state::next:
if (*p == c.escape)
{
state = alt_sub_end_state::escape;
}
else if (c.set_enabled && *p == c.set_open &&
is_set(cx::next(p), pend, c, is_set_state::not_or_first))
{
p = cx::prev(set_end(cx::next(p), pend, c, set_end_state::not_or_first));
}
else if (*p == c.alt_open)
{
++depth;
}
else if (*p == c.alt_close)
{
--depth;
if (depth == 0)
{
return p;
}
}
else if (*p == c.alt_or)
{
if (depth == 1)
{
return p;
}
}
break;
case alt_sub_end_state::escape:
state = alt_sub_end_state::next;
break;
default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::logic_error(
"The program execution should never end up here throwing this exception");
#else
return throw_logic_error(
p, "The program execution should never end up here throwing this exception");
#endif
}
p = cx::next(p);
}
#if cfg_HAS_FULL_FEATURED_CONSTEXPR14
throw std::invalid_argument("The given pattern is not a valid alternative");
#else
return throw_invalid_argument(p, "The given pattern is not a valid alternative");
#endif
#else
return !c.alt_enabled
? throw std::invalid_argument("The use of alternatives is disabled")
: p == pend
? throw std::invalid_argument("The given pattern is not a valid alternative")
: state == alt_sub_end_state::next
? *p == c.escape
? alt_sub_end(cx::next(p), pend, c, alt_sub_end_state::escape, depth)
: c.set_enabled && *p == c.set_open &&
is_set(cx::next(p), pend, c, is_set_state::not_or_first)
? alt_sub_end(set_end(cx::next(p), pend, c,
set_end_state::not_or_first),
pend, c, state, depth)
: *p == c.alt_open
? alt_sub_end(cx::next(p), pend, c, state, depth + 1)
: *p == c.alt_close
? depth == 1 ? p : alt_sub_end(cx::next(p), pend,
c, state, depth - 1)
: *p == c.alt_or
? depth == 1 ? p
: alt_sub_end(cx::next(p), pend,
c, state, depth)
: alt_sub_end(cx::next(p), pend, c, state,
depth)
:
state == alt_sub_end_state::escape
? alt_sub_end(cx::next(p), pend, c, alt_sub_end_state::next, depth)
: throw std::logic_error(
"The program execution should never end up here throwing "
"this "
"exception");
#endif
}
template <typename SequenceIterator, typename PatternIterator,
typename EqualTo = cx::equal_to<void>>
constexpr match_result<SequenceIterator, PatternIterator> match(
SequenceIterator s, SequenceIterator send, PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
const EqualTo& equal_to = EqualTo(), bool partial = false, bool escape = false);
template <typename SequenceIterator, typename PatternIterator,
typename EqualTo = cx::equal_to<void>>
constexpr match_result<SequenceIterator, PatternIterator> match_alt(
SequenceIterator s, SequenceIterator send, PatternIterator p1, PatternIterator p1end,
PatternIterator p2, PatternIterator p2end,
const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
const EqualTo& equal_to = EqualTo(), bool partial = false)
{
#if cfg_HAS_CONSTEXPR14
auto result1 = match(s, send, p1, p1end, c, equal_to, true);
if (result1)
{
auto result2 = match(result1.s, send, p2, p2end, c, equal_to, partial);
if (result2)
{
return result2;
}
}
p1 = cx::next(p1end);
if (p1 == p2)
{
return make_match_result(false, s, p1end);
}
return match_alt(s, send, p1, alt_sub_end(p1, p2, c), p2, p2end, c, equal_to, partial);
#else
return match(s, send, p1, p1end, c, equal_to, true) &&
match(match(s, send, p1, p1end, c, equal_to, true).s, send, p2, p2end, c, equal_to,
partial)
? match(match(s, send, p1, p1end, c, equal_to, true).s, send, p2, p2end, c, equal_to,
partial)
: cx::next(p1end) == p2
? make_match_result(false, s, p1end)
: match_alt(s, send, cx::next(p1end), alt_sub_end(cx::next(p1end), p2, c), p2,
p2end, c, equal_to, partial);
#endif
}
template <typename SequenceIterator, typename PatternIterator, typename EqualTo>
constexpr match_result<SequenceIterator, PatternIterator> match(
SequenceIterator s, SequenceIterator send, PatternIterator p, PatternIterator pend,
const cards<iterated_item_t<PatternIterator>>& c, const EqualTo& equal_to, bool partial,
bool escape)
{
#if cfg_HAS_CONSTEXPR14
if (p == pend)
{
return make_match_result(partial || s == send, s, p);
}
if (escape)
{
if (s == send || !equal_to(*s, *p))
{
return make_match_result(false, s, p);
}
return match(cx::next(s), send, cx::next(p), pend, c, equal_to, partial);
}
if (*p == c.anything)
{
auto result = match(s, send, cx::next(p), pend, c, equal_to, partial);
if (result)
{
return result;
}
if (s == send)
{
return make_match_result(false, s, p);
}
return match(cx::next(s), send, p, pend, c, equal_to, partial);
}
if (*p == c.single)
{
if (s == send)
{
return make_match_result(false, s, p);
}
return match(cx::next(s), send, cx::next(p), pend, c, equal_to, partial);
}
if (*p == c.escape)
{
return match(s, send, cx::next(p), pend, c, equal_to, partial, true);
}
if (c.set_enabled && *p == c.set_open && is_set(cx::next(p), pend, c, is_set_state::not_or_first))
{
auto result =
match_set(s, send, cx::next(p), pend, c, equal_to, match_set_state::not_or_first_in);
if (!result)
{
return result;
}
return match(cx::next(s), send, set_end(cx::next(p), pend, c, set_end_state::not_or_first),
pend, c, equal_to, partial);
}
if (c.alt_enabled && *p == c.alt_open && is_alt(cx::next(p), pend, c, is_alt_state::next, 1))
{
auto p_alt_end = alt_end(cx::next(p), pend, c, alt_end_state::next, 1);
return match_alt(s, send, cx::next(p), alt_sub_end(cx::next(p), p_alt_end, c), p_alt_end, pend,
c, equal_to, partial);
}
if (s == send || !equal_to(*s, *p))
{
return make_match_result(false, s, p);
}
return match(cx::next(s), send, cx::next(p), pend, c, equal_to, partial);
#else
return p == pend
? make_match_result(partial || s == send, s, p)
: escape
? s == send || !equal_to(*s, *p)
? make_match_result(false, s, p)
: match(cx::next(s), send, cx::next(p), pend, c, equal_to, partial)
: *p == c.anything
? match(s, send, cx::next(p), pend, c, equal_to, partial)
? match(s, send, cx::next(p), pend, c, equal_to, partial)
: s == send ? make_match_result(false, s, p)
: match(cx::next(s), send, p, pend, c, equal_to, partial)
: *p == c.single
? s == send ? make_match_result(false, s, p)
: match(cx::next(s), send, cx::next(p), pend, c,
equal_to, partial)
: *p == c.escape
? match(s, send, cx::next(p), pend, c, equal_to, partial, true)
: c.set_enabled && *p == c.set_open &&
is_set(cx::next(p), pend, c,
is_set_state::not_or_first)
? !match_set(s, send, cx::next(p), pend, c, equal_to,
match_set_state::not_or_first_in)
? match_set(s, send, cx::next(p), pend, c,
equal_to,
match_set_state::not_or_first_in)
: match(cx::next(s), send,
set_end(cx::next(p), pend, c,
set_end_state::not_or_first),
pend, c, equal_to, partial)
: c.alt_enabled && *p == c.alt_open &&
is_alt(cx::next(p), pend, c,
is_alt_state::next, 1)
? match_alt(
s, send, cx::next(p),
alt_sub_end(cx::next(p),
alt_end(cx::next(p), pend, c,
alt_end_state::next, 1),
c),
alt_end(cx::next(p), pend, c,
alt_end_state::next, 1),
pend, c, equal_to, partial)
: s == send || !equal_to(*s, *p)
? make_match_result(false, s, p)
: match(cx::next(s), send, cx::next(p), pend,
c, equal_to, partial);
#endif
}
}
template <typename Sequence, typename Pattern, typename EqualTo = cx::equal_to<void>>
constexpr full_match_result<const_iterator_t<Sequence>, const_iterator_t<Pattern>> match(
Sequence&& sequence, Pattern&& pattern,
const cards<container_item_t<Pattern>>& c = cards<container_item_t<Pattern>>(),
const EqualTo& equal_to = EqualTo())
{
return detail::make_full_match_result(
cx::cbegin(sequence), cx::cend(sequence), cx::cbegin(pattern), cx::cend(pattern),
detail::match(cx::cbegin(sequence), cx::cend(std::forward<Sequence>(sequence)),
cx::cbegin(pattern), cx::cend(std::forward<Pattern>(pattern)), c, equal_to));
}
template <typename Sequence, typename Pattern, typename EqualTo = cx::equal_to<void>,
typename = typename std::enable_if<!std::is_same<EqualTo, cards_type>::value>::type>
constexpr full_match_result<const_iterator_t<Sequence>, const_iterator_t<Pattern>> match(
Sequence&& sequence, Pattern&& pattern, const EqualTo& equal_to)
{
return match(std::forward<Sequence>(sequence), std::forward<Pattern>(pattern),
cards<container_item_t<Pattern>>(), equal_to);
}
}
#endif
#ifndef WILDCARDS_MATCHER_HPP
#define WILDCARDS_MATCHER_HPP 
#include <cstddef>
#include <type_traits>
#include <utility>
#ifndef CX_STRING_VIEW_HPP
#define CX_STRING_VIEW_HPP 
#include <cstddef>
#include <ostream>
#ifndef CX_ALGORITHM_HPP
#define CX_ALGORITHM_HPP 
namespace cx
{
template <typename Iterator1, typename Iterator2>
constexpr bool equal(Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2)
{
#if cfg_HAS_CONSTEXPR14
while (first1 != last1 && first2 != last2 && *first1 == *first2)
{
++first1, ++first2;
}
return first1 == last1 && first2 == last2;
#else
return first1 != last1 && first2 != last2 && *first1 == *first2
? equal(first1 + 1, last1, first2 + 1, last2)
: first1 == last1 && first2 == last2;
#endif
}
}
#endif
namespace cx
{
template <typename T>
class basic_string_view
{
public:
using value_type = T;
constexpr basic_string_view() = default;
template <std::size_t N>
constexpr basic_string_view(const T (&str)[N]) : data_{&str[0]}, size_{N - 1}
{
}
constexpr basic_string_view(const T* str, std::size_t s) : data_{str}, size_{s}
{
}
constexpr const T* data() const
{
return data_;
}
constexpr std::size_t size() const
{
return size_;
}
constexpr bool empty() const
{
return size() == 0;
}
constexpr const T* begin() const
{
return data_;
}
constexpr const T* cbegin() const
{
return begin();
}
constexpr const T* end() const
{
return data_ + size_;
}
constexpr const T* cend() const
{
return end();
}
private:
const T* data_{nullptr};
std::size_t size_{0};
};
template <typename T>
constexpr bool operator==(const basic_string_view<T>& lhs, const basic_string_view<T>& rhs)
{
return equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <typename T>
constexpr bool operator!=(const basic_string_view<T>& lhs, const basic_string_view<T>& rhs)
{
return !(lhs == rhs);
}
template <typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& o, const basic_string_view<T>& s)
{
o << s.data();
return o;
}
template <typename T, std::size_t N>
constexpr basic_string_view<T> make_string_view(const T (&str)[N])
{
return {str, N - 1};
}
template <typename T>
constexpr basic_string_view<T> make_string_view(const T* str, std::size_t s)
{
return {str, s};
}
using string_view = basic_string_view<char>;
using u16string_view = basic_string_view<char16_t>;
using u32string_view = basic_string_view<char32_t>;
using wstring_view = basic_string_view<wchar_t>;
namespace literals
{
constexpr string_view operator"" _sv(const char* str, std::size_t s)
{
return {str, s};
}
constexpr u16string_view operator"" _sv(const char16_t* str, std::size_t s)
{
return {str, s};
}
constexpr u32string_view operator"" _sv(const char32_t* str, std::size_t s)
{
return {str, s};
}
constexpr wstring_view operator"" _sv(const wchar_t* str, std::size_t s)
{
return {str, s};
}
}
}
#endif
namespace wildcards
{
template <typename Pattern, typename EqualTo = cx::equal_to<void>>
class matcher
{
public:
constexpr explicit matcher(Pattern&& pattern, const cards<container_item_t<Pattern>>& c =
cards<container_item_t<Pattern>>(),
const EqualTo& equal_to = EqualTo())
: p_{cx::cbegin(pattern)},
pend_{cx::cend(std::forward<Pattern>(pattern))},
c_{c},
equal_to_{equal_to}
{
}
constexpr matcher(Pattern&& pattern, const EqualTo& equal_to)
: p_{cx::cbegin(pattern)},
pend_{cx::cend(std::forward<Pattern>(pattern))},
c_{cards<container_item_t<Pattern>>()},
equal_to_{equal_to}
{
}
template <typename Sequence>
constexpr full_match_result<const_iterator_t<Sequence>, const_iterator_t<Pattern>> matches(
Sequence&& sequence) const
{
return detail::make_full_match_result(
cx::cbegin(sequence), cx::cend(sequence), p_, pend_,
detail::match(cx::cbegin(sequence), cx::cend(std::forward<Sequence>(sequence)), p_, pend_,
c_, equal_to_));
}
private:
const_iterator_t<Pattern> p_;
const_iterator_t<Pattern> pend_;
cards<container_item_t<Pattern>> c_;
EqualTo equal_to_;
};
template <typename Pattern, typename EqualTo = cx::equal_to<void>>
constexpr matcher<Pattern, EqualTo> make_matcher(
Pattern&& pattern,
const cards<container_item_t<Pattern>>& c = cards<container_item_t<Pattern>>(),
const EqualTo& equal_to = EqualTo())
{
return matcher<Pattern, EqualTo>{std::forward<Pattern>(pattern), c, equal_to};
}
template <typename Pattern, typename EqualTo = cx::equal_to<void>,
typename = typename std::enable_if<!std::is_same<EqualTo, cards_type>::value>::type>
constexpr matcher<Pattern, EqualTo> make_matcher(Pattern&& pattern, const EqualTo& equal_to)
{
return make_matcher(std::forward<Pattern>(pattern), cards<container_item_t<Pattern>>(), equal_to);
}
namespace literals
{
constexpr auto operator"" _wc(const char* str, std::size_t s)
-> decltype(make_matcher(cx::make_string_view(str, s + 1)))
{
return make_matcher(cx::make_string_view(str, s + 1));
}
constexpr auto operator"" _wc(const char16_t* str, std::size_t s)
-> decltype(make_matcher(cx::make_string_view(str, s + 1)))
{
return make_matcher(cx::make_string_view(str, s + 1));
}
constexpr auto operator"" _wc(const char32_t* str, std::size_t s)
-> decltype(make_matcher(cx::make_string_view(str, s + 1)))
{
return make_matcher(cx::make_string_view(str, s + 1));
}
constexpr auto operator"" _wc(const wchar_t* str, std::size_t s)
-> decltype(make_matcher(cx::make_string_view(str, s + 1)))
{
return make_matcher(cx::make_string_view(str, s + 1));
}
}
}
#endif
#endif
