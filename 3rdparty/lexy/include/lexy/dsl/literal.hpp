// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LITERAL_HPP_INCLUDED
#define LEXY_DSL_LITERAL_HPP_INCLUDED

#include <lexy/_detail/code_point.hpp>
#include <lexy/_detail/integer_sequence.hpp>
#include <lexy/_detail/iterator.hpp>
#include <lexy/_detail/nttp_string.hpp>
#include <lexy/_detail/swar.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/token.hpp>

//=== lit_matcher ===//
namespace lexy::_detail
{
template <std::size_t CurCharIndex, typename CharT, CharT... Cs, typename Reader>
constexpr auto match_literal(Reader& reader)
{
    static_assert(lexy::is_char_encoding<typename Reader::encoding>);
    using char_type = typename Reader::encoding::char_type;
    if constexpr (CurCharIndex >= sizeof...(Cs))
    {
        (void)reader;
        return std::true_type{};
    }
    // We only use SWAR if the reader supports it and we have enough to fill at least one.
    else if constexpr (is_swar_reader<Reader> && sizeof...(Cs) >= swar_length<char_type>)
    {
        // Try and pack as many characters into a swar as possible, starting at the current
        // index.
        constexpr auto pack = swar_pack<CurCharIndex>(transcode_char<char_type>(Cs)...);

        // Do a single swar comparison.
        if ((reader.peek_swar() & pack.mask) == pack.value)
        {
            reader.bump_swar(pack.count);

            // Recurse with the incremented index.
            return bool(match_literal<CurCharIndex + pack.count, CharT, Cs...>(reader));
        }
        else
        {
            auto partial = swar_find_difference<CharT>(reader.peek_swar() & pack.mask, pack.value);
            reader.bump_swar(partial);
            return false;
        }
    }
    else
    {
        static_assert(CurCharIndex == 0);

        // Compare each code unit, bump on success, cancel on failure.
        return ((reader.peek() == transcode_int<typename Reader::encoding>(Cs)
                     ? (reader.bump(), true)
                     : false)
                && ...);
    }
}
} // namespace lexy::_detail

//=== lit_trie ===//
namespace lexy::_detail
{
template <typename Encoding, template <typename> typename CaseFolding, std::size_t MaxCharCount,
          typename... CharClasses>
struct lit_trie
{
    using encoding  = Encoding;
    using char_type = typename Encoding::char_type;

    template <typename Reader>
    using reader = CaseFolding<Reader>;

    static constexpr auto max_node_count = MaxCharCount + 1; // root node
    static constexpr auto max_transition_count
        = max_node_count == 1 ? 1 : max_node_count - 1; // it is a tree
    static constexpr auto node_no_match = std::size_t(-1);

    std::size_t node_count;
    std::size_t node_value[max_node_count];
    // Index of a char class that must not match at the end.
    // This is used for keywords.
    std::size_t node_char_class[max_node_count];

    char_type   transition_char[max_transition_count];
    std::size_t transition_from[max_transition_count];
    std::size_t transition_to[max_transition_count];

    LEXY_CONSTEVAL lit_trie()
    : node_count(1), node_value{}, node_char_class{}, transition_char{}, transition_from{},
      transition_to{}
    {
        node_value[0]      = node_no_match;
        node_char_class[0] = sizeof...(CharClasses);
    }

    template <typename CharT>
    LEXY_CONSTEVAL std::size_t insert(std::size_t from, CharT _c)
    {
        auto c = transcode_char<char_type>(_c);

        // We need to find a transition.
        // In a tree, we're always having node_count - 1 transitions, so that's the upper bound.
        // Everytime we add a transition from a node, its transition index is >= its node index.
        // As such, we start looking at the node index.
        for (auto i = from; i != node_count - 1; ++i)
        {
            if (transition_from[i] == from && transition_char[i] == c)
                return transition_to[i];
        }

        auto to             = node_count;
        node_value[to]      = node_no_match;
        node_char_class[to] = sizeof...(CharClasses);

        auto trans             = node_count - 1;
        transition_char[trans] = c;
        transition_from[trans] = from; // trans >= from as from < node_count
        transition_to[trans]   = to;

        ++node_count;
        return to;
    }

    template <typename CharT, CharT... C>
    LEXY_CONSTEVAL std::size_t insert(std::size_t pos, type_string<CharT, C...>)
    {
        return ((pos = insert(pos, C)), ...);
    }

    LEXY_CONSTEVAL auto node_transitions(std::size_t node) const
    {
        struct
        {
            std::size_t length;
            std::size_t index[max_transition_count];
        } result{};

        // We need to consider the same range only.
        for (auto i = node; i != node_count - 1; ++i)
            if (transition_from[i] == node)
            {
                result.index[result.length] = i;
                ++result.length;
            }

        return result;
    }
};

template <typename... CharClasses>
struct char_class_list
{
    template <typename Encoding, template <typename> typename CaseFolding, std::size_t N>
    using trie_type = lit_trie<Encoding, CaseFolding, N, CharClasses...>;

    static constexpr auto size = sizeof...(CharClasses);

    template <typename... T>
    constexpr auto operator+(char_class_list<T...>) const
    {
        return char_class_list<CharClasses..., T...>{};
    }
};

template <typename CurrentCaseFolding, typename... Literals>
struct _merge_case_folding;
template <>
struct _merge_case_folding<void>
{
    template <typename Reader>
    using case_folding = Reader;
};
template <typename CurrentCaseFolding>
struct _merge_case_folding<CurrentCaseFolding>
{
    template <typename Reader>
    using case_folding = typename CurrentCaseFolding::template reader<Reader>;
};
template <typename CurrentCaseFolding, typename H, typename... T>
struct _merge_case_folding<CurrentCaseFolding, H, T...>
: _merge_case_folding<std::conditional_t<std::is_void_v<CurrentCaseFolding>,
                                         typename H::lit_case_folding, CurrentCaseFolding>,
                      T...>
{
    static_assert(std::is_same_v<CurrentCaseFolding,
                                 typename H::lit_case_folding> //
                      || std::is_void_v<CurrentCaseFolding>
                      || std::is_void_v<typename H::lit_case_folding>,
                  "cannot mix literals with different case foldings in a literal_set");
};

template <typename Reader>
using lit_no_case_fold = Reader;

template <typename Encoding, typename... Literals>
LEXY_CONSTEVAL auto make_empty_trie()
{
    constexpr auto max_char_count = (0 + ... + Literals::lit_max_char_count);

    // Merge all mentioned character classes in a single list.
    constexpr auto char_classes
        = (lexy::_detail::char_class_list{} + ... + Literals::lit_char_classes);

    // Need to figure out case folding as well.
    return typename decltype(char_classes)::template trie_type<
        Encoding, _merge_case_folding<void, Literals...>::template case_folding, max_char_count>{};
}
template <typename Encoding, typename... Literals>
using lit_trie_for = decltype(make_empty_trie<Encoding, Literals...>());

template <std::size_t CharClassIdx, bool, typename...>
struct _node_char_class_impl
{
    template <typename Reader>
    LEXY_FORCE_INLINE static constexpr std::false_type match(const Reader&)
    {
        return {};
    }
};
template <typename H, typename... T>
struct _node_char_class_impl<0, true, H, T...>
{
    template <typename Reader>
    LEXY_FORCE_INLINE static constexpr bool match(Reader reader)
    {
        return lexy::token_parser_for<H, Reader>(reader).try_parse(reader);
    }
};
template <std::size_t Idx, typename H, typename... T>
struct _node_char_class_impl<Idx, true, H, T...> : _node_char_class_impl<Idx - 1, true, T...>
{};
template <std::size_t CharClassIdx, typename... CharClasses>
using _node_char_class
    = _node_char_class_impl<CharClassIdx, (CharClassIdx < sizeof...(CharClasses)), CharClasses...>;

template <const auto& Trie, std::size_t CurNode>
struct lit_trie_matcher;
template <typename Encoding, template <typename> typename CaseFolding, std::size_t N,
          typename... CharClasses, const lit_trie<Encoding, CaseFolding, N, CharClasses...>& Trie,
          std::size_t CurNode>
struct lit_trie_matcher<Trie, CurNode>
{
    template <std::size_t TransIdx, typename Reader, typename IntT>
    LEXY_FORCE_INLINE static constexpr bool _try_transition(std::size_t& result, Reader& reader,
                                                            IntT cur)
    {
        static_assert(Trie.transition_from[TransIdx] == CurNode);

        using encoding            = typename Reader::encoding;
        constexpr auto trans_char = Trie.transition_char[TransIdx];
        if (cur != encoding::to_int_type(trans_char))
            return false;

        reader.bump();
        result = lit_trie_matcher<Trie, Trie.transition_to[TransIdx]>::try_match(reader);
        return true;
    }

    static constexpr auto transitions = Trie.node_transitions(CurNode);

    template <typename Indices = make_index_sequence<transitions.length>>
    struct _impl;
    template <std::size_t... Idx>
    struct _impl<index_sequence<Idx...>>
    {
        template <typename Reader>
        LEXY_FORCE_INLINE static constexpr std::size_t try_match(Reader& reader)
        {
            constexpr auto cur_value = Trie.node_value[CurNode];

            if constexpr (sizeof...(Idx) > 0)
            {
                auto cur      = reader.current();
                auto cur_char = reader.peek();

                auto next_value = Trie.node_no_match;
                (void)(_try_transition<transitions.index[Idx]>(next_value, reader, cur_char)
                       || ...);
                if (next_value != Trie.node_no_match)
                    // We prefer a longer match.
                    return next_value;

                // We haven't found a longer match, return our match.
                reader.reset(cur);
            }

            // But first, we might need to check that we don't match that nodes char class.
            constexpr auto char_class = Trie.node_char_class[CurNode];
            if constexpr (cur_value == Trie.node_no_match || char_class >= sizeof...(CharClasses))
            {
                return cur_value;
            }
            else
            {
                if (_node_char_class<char_class, CharClasses...>::match(reader))
                    return Trie.node_no_match;
                else
                    return cur_value;
            }
        }
    };

    template <typename Reader>
    LEXY_FORCE_INLINE static constexpr std::size_t try_match(Reader& _reader)
    {
        static_assert(lexy::is_char_encoding<typename Reader::encoding>);
        if constexpr (std::is_same_v<CaseFolding<Reader>, Reader>)
        {
            return _impl<>::try_match(_reader);
        }
        else
        {
            CaseFolding<Reader> reader{_reader};
            auto                result = _impl<>::try_match(reader);
            _reader.reset(reader.current());
            return result;
        }
    }
};
} // namespace lexy::_detail

//=== lit ===//
namespace lexyd
{
template <typename CharT, CharT... C>
struct _lit
: token_base<_lit<CharT, C...>,
             std::conditional_t<sizeof...(C) == 0, unconditional_branch_base, branch_base>>,
  _lit_base
{
    static constexpr auto lit_max_char_count = sizeof...(C);
    static constexpr auto lit_char_classes   = lexy::_detail::char_class_list{};
    using lit_case_folding                   = void;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        typename Encoding::char_type result = 0;
        (void)((result = lexy::_detail::transcode_char<decltype(result)>(C), true) || ...);
        return result;
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos, std::size_t)
    {
        return ((pos = trie.insert(pos, C)), ...);
    }

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr auto try_parse(Reader reader)
        {
            auto result = lexy::_detail::match_literal<0, CharT, C...>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using char_type    = typename Reader::encoding::char_type;
            constexpr auto str = lexy::_detail::type_string<CharT, C...>::template c_str<char_type>;

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, end.position());
            auto err = lexy::error<Reader, lexy::expected_literal>(begin, str, index, sizeof...(C));
            context.on(_ev::error{}, err);
        }
    };
};

template <auto C>
constexpr auto lit_c = _lit<LEXY_DECAY_DECLTYPE(C), C>{};

template <unsigned char... C>
constexpr auto lit_b = _lit<unsigned char, C...>{};

#if LEXY_HAS_NTTP
/// Matches the literal string.
template <lexy::_detail::string_literal Str>
constexpr auto lit = lexy::_detail::to_type_string<_lit, Str>{};
#endif

#define LEXY_LIT(Str)                                                                              \
    LEXY_NTTP_STRING(::lexyd::_lit, Str) {}
} // namespace lexyd

namespace lexy
{
template <typename CharT, CharT... C>
inline constexpr auto token_kind_of<lexy::dsl::_lit<CharT, C...>> = lexy::literal_token_kind;
} // namespace lexy

//=== lit_cp ===//
namespace lexyd
{
template <char32_t... Cp>
struct _lcp : token_base<_lcp<Cp...>>, _lit_base
{
    template <typename Encoding>
    struct _string_t
    {
        typename Encoding::char_type data[4 * sizeof...(Cp)];
        std::size_t                  length = 0;

        constexpr _string_t() : data{}
        {
            ((length += lexy::_detail::encode_code_point<Encoding>(Cp, data + length, 4)), ...);
        }
    };
    template <typename Encoding>
    static constexpr _string_t<Encoding> _string = _string_t<Encoding>{};

    static constexpr auto lit_max_char_count = 4 * sizeof...(Cp);
    static constexpr auto lit_char_classes   = lexy::_detail::char_class_list{};
    using lit_case_folding                   = void;

    template <typename Encoding>
    static constexpr auto lit_first_char() -> typename Encoding::char_type
    {
        return _string<Encoding>.data[0];
    }

    template <typename Trie>
    static LEXY_CONSTEVAL std::size_t lit_insert(Trie& trie, std::size_t pos, std::size_t)
    {
        using encoding = typename Trie::encoding;

        for (auto i = 0u; i != _string<encoding>.length; ++i)
            pos = trie.insert(pos, _string<encoding>.data[i]);

        return pos;
    }

    template <typename Reader,
              typename Indices
              = lexy::_detail::make_index_sequence<_string<typename Reader::encoding>.length>>
    struct tp;
    template <typename Reader, std::size_t... Idx>
    struct tp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            using encoding = typename Reader::encoding;

            auto result = lexy::_detail::match_literal<0, typename encoding::char_type,
                                                       _string<encoding>.data[Idx]...>(reader);
            end         = reader.current();
            return result;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            using encoding = typename Reader::encoding;

            auto begin = reader.position();
            auto index = lexy::_detail::range_size(begin, end.position());
            auto err   = lexy::error<Reader, lexy::expected_literal>(begin, _string<encoding>.data,
                                                                   index, _string<encoding>.length);
            context.on(_ev::error{}, err);
        }
    };
};

template <char32_t... CodePoint>
constexpr auto lit_cp = _lcp<CodePoint...>{};
} // namespace lexyd

namespace lexy
{
template <char32_t... Cp>
constexpr auto token_kind_of<lexy::dsl::_lcp<Cp...>> = lexy::literal_token_kind;
} // namespace lexy

//=== lit_set ===//
namespace lexy
{
template <typename T, template <typename> typename CaseFolding, typename... Strings>
class _symbol_table;

struct expected_literal_set
{
    static LEXY_CONSTEVAL auto name()
    {
        return "expected literal set";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Literal, template <typename> typename CaseFolding>
struct _cfl;

template <template <typename> typename CaseFolding, typename CharT, CharT... C>
constexpr auto _make_lit_rule(lexy::_detail::type_string<CharT, C...>)
{
    if constexpr (std::is_same_v<CaseFolding<lexy::_pr8>, lexy::_pr8>)
        return _lit<CharT, C...>{};
    else
        return _cfl<_lit<CharT, C...>, CaseFolding>{};
}

template <typename... Literals>
struct _lset : token_base<_lset<Literals...>>, _lset_base
{
    using as_lset = _lset;

    template <typename Encoding>
    static LEXY_CONSTEVAL auto _build_trie()
    {
        auto result = lexy::_detail::make_empty_trie<Encoding, Literals...>();

        [[maybe_unused]] auto char_class = std::size_t(0);
        ((result.node_value[Literals::lit_insert(result, 0, char_class)] = 0,
          // Keep the index correct.
          char_class += Literals::lit_char_classes.size),
         ...);

        return result;
    }
    template <typename Encoding>
    static constexpr lexy::_detail::lit_trie_for<Encoding, Literals...> _t
        = _build_trie<Encoding>();

    template <typename Reader>
    struct tp
    {
        typename Reader::marker end;

        constexpr explicit tp(const Reader& reader) : end(reader.current()) {}

        constexpr bool try_parse(Reader reader)
        {
            using encoding = typename Reader::encoding;
            using matcher  = lexy::_detail::lit_trie_matcher<_t<encoding>, 0>;

            auto result = matcher::try_match(reader);
            end         = reader.current();
            return result != _t<encoding>.node_no_match;
        }

        template <typename Context>
        constexpr void report_error(Context& context, const Reader& reader)
        {
            auto err = lexy::error<Reader, lexy::expected_literal_set>(reader.position());
            context.on(_ev::error{}, err);
        }
    };

    //=== dsl ===//
    template <typename Lit>
    constexpr auto operator/(Lit) const
    {
        if constexpr (lexy::is_literal_rule<Lit>)
        {
            return _lset<Literals..., Lit>{};
        }
        else if constexpr (sizeof...(Literals) == 0)
        {
            // We're empty, so do nothing and keep it type-erased.
            static_assert(lexy::is_literal_set_rule<Lit>);
            return Lit{};
        }
        else
        {
            // We're non empty, undo type erasure to append.
            static_assert(lexy::is_literal_set_rule<Lit>);
            return *this / typename Lit::as_lset{};
        }
    }
    template <typename... Lit>
    constexpr auto operator/(_lset<Lit...>) const
    {
        return _lset<Literals..., Lit...>{};
    }
};

/// Matches one of the specified literals.
template <typename... Literals>
constexpr auto literal_set(Literals...)
{
    static_assert((lexy::is_literal_rule<Literals> && ...));
    return _lset<Literals...>{};
}

/// Matches one of the symbols in the symbol table.
template <typename T, template <typename> typename CaseFolding, typename... Strings>
constexpr auto literal_set(const lexy::_symbol_table<T, CaseFolding, Strings...>)
{
    return _lset<decltype(_make_lit_rule<CaseFolding>(Strings{}))...>{};
}
} // namespace lexyd

#define LEXY_LITERAL_SET(...)                                                                      \
    [] {                                                                                           \
        using impl = decltype(::lexyd::literal_set(__VA_ARGS__));                                  \
        struct s : impl                                                                            \
        {                                                                                          \
            using impl::operator/;                                                                 \
        };                                                                                         \
        return s{};                                                                                \
    }()

namespace lexy
{
template <typename... Literals>
constexpr auto token_kind_of<lexy::dsl::_lset<Literals...>> = lexy::literal_token_kind;
} // namespace lexy

#endif // LEXY_DSL_LITERAL_HPP_INCLUDED

