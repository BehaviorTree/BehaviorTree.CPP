// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_GRAMMAR_HPP_INCLUDED
#define LEXY_GRAMMAR_HPP_INCLUDED

#include <cstdint>
#include <lexy/_detail/config.hpp>
#include <lexy/_detail/detect.hpp>
#include <lexy/_detail/type_name.hpp>
#include <lexy/callback/base.hpp>

//=== rule ===//
// We use a shorthand namespace to decrease symbol size.
namespace lexyd
{
struct rule_base
{};

struct branch_base : rule_base
{};

struct unconditional_branch_base : branch_base
{};

struct _token_base
{};
struct _char_class_base
{};
struct _lset_base
{};
struct _lit_base
{};
struct _sep_base
{};

struct _operation_base
{};
} // namespace lexyd

namespace lexy
{
namespace dsl = lexyd;

template <typename T>
constexpr bool is_rule = std::is_base_of_v<dsl::rule_base, T>;

template <typename T>
constexpr bool is_branch_rule = std::is_base_of_v<dsl::branch_base, T>;
template <typename T>
constexpr bool is_unconditional_branch_rule = std::is_base_of_v<dsl::unconditional_branch_base, T>;

template <typename T>
constexpr bool is_token_rule = std::is_base_of_v<dsl::_token_base, T>;

template <typename T>
constexpr bool is_char_class_rule = std::is_base_of_v<dsl::_char_class_base, T>;
template <typename T>
constexpr bool is_literal_rule = std::is_base_of_v<dsl::_lit_base, T>;
template <typename T>
constexpr bool is_literal_set_rule = std::is_base_of_v<dsl::_lset_base, T>;

template <typename T>
constexpr auto is_separator = std::is_base_of_v<lexy::dsl::_sep_base, T>;

template <typename T>
constexpr auto is_operation = std::is_base_of_v<lexy::dsl::_operation_base, T>;

template <typename... T>
constexpr bool _require_branch_rule = (is_branch_rule<T> && ...);
} // namespace lexy

#define LEXY_REQUIRE_BRANCH_RULE(Rule, Name)                                                       \
    static_assert(lexy::_require_branch_rule<Rule>, Name                                           \
                  " requires a branch condition."                                                  \
                  " You may need to use `>>` to specify the condition that is used for dispatch."  \
                  " See https://lexy.foonathan.net/learn/branching/ for more information.")

//=== predefined_token_kind ===//
namespace lexy
{
enum predefined_token_kind : std::uint_least16_t
{
    unknown_token_kind = UINT_LEAST16_MAX,

    error_token_kind      = UINT_LEAST16_MAX - 1,
    whitespace_token_kind = UINT_LEAST16_MAX - 2,
    any_token_kind        = UINT_LEAST16_MAX - 3,

    literal_token_kind  = UINT_LEAST16_MAX - 4,
    position_token_kind = UINT_LEAST16_MAX - 5,
    eof_token_kind      = UINT_LEAST16_MAX - 6,

    identifier_token_kind = UINT_LEAST16_MAX - 7,
    digits_token_kind     = UINT_LEAST16_MAX - 8,

    _smallest_predefined_token_kind = digits_token_kind,
};

template <typename T>
constexpr const char* token_kind_name(const T&) noexcept
{
    return "token";
}
constexpr const char* token_kind_name(predefined_token_kind kind) noexcept
{
    switch (kind)
    {
    case unknown_token_kind:
        return "token";

    case error_token_kind:
        return "error token";
    case whitespace_token_kind:
        return "whitespace";
    case any_token_kind:
        return "any";

    case literal_token_kind:
        return "literal";
    case position_token_kind:
        return "position";
    case eof_token_kind:
        return "EOF";

    case identifier_token_kind:
        return "identifier";
    case digits_token_kind:
        return "digits";
    }

    return ""; // unreachable
}

/// Specialize to define the token kind of a rule.
template <typename TokenRule>
constexpr auto token_kind_of = lexy::unknown_token_kind;

template <typename TokenRule>
constexpr auto token_kind_of<const TokenRule> = token_kind_of<TokenRule>;
} // namespace lexy

//=== production ===//
namespace lexy
{
template <typename Production>
using production_rule = LEXY_DECAY_DECLTYPE(Production::rule);

template <typename Production>
constexpr bool is_production = _detail::is_detected<production_rule, Production>;

/// Base class to indicate that this production is conceptually a token.
/// This inhibits whitespace skipping inside the production.
///
/// When generating a parse tree, it will also merge tokens of the same kind into the same node.
struct token_production
{};

template <typename Production>
constexpr bool is_token_production = std::is_base_of_v<token_production, Production>;

/// Base class to indicate that this production is transparent for the parse tree generation.
/// It will not create a node in the tree, all children will be added to the its parent.
/// If parse tree generation is not used, it has no effect.
struct transparent_production
{};

template <typename Production>
constexpr bool is_transparent_production = std::is_base_of_v<transparent_production, Production>;

template <typename Production>
LEXY_CONSTEVAL const char* production_name()
{
    return _detail::type_name<Production>();
}

template <typename Production>
using _detect_max_recursion_depth = decltype(Production::max_recursion_depth);

template <typename EntryProduction>
LEXY_CONSTEVAL std::size_t max_recursion_depth()
{
    if constexpr (_detail::is_detected<_detect_max_recursion_depth, EntryProduction>)
        return EntryProduction::max_recursion_depth;
    else
        return 1024; // Arbitrary power of two.
}

template <typename T>
using _enable_production_or_operation = std::enable_if_t<is_production<T> || is_operation<T>>;

struct production_info
{
    const char* const* id;
    const char*        name;
    bool               is_token;
    bool               is_transparent;

    template <typename Production, typename = _enable_production_or_operation<Production>>
    constexpr production_info(Production)
    : id(_detail::type_id<Production>()), name(production_name<Production>()),
      is_token(is_token_production<Production>),
      is_transparent(is_transparent_production<Production>)
    {}

    friend constexpr bool operator==(production_info lhs, production_info rhs)
    {
        return lhs.id == rhs.id;
    }
    friend constexpr bool operator!=(production_info lhs, production_info rhs)
    {
        return !(lhs == rhs);
    }
};
} // namespace lexy

namespace lexy
{
template <typename Production>
using _detect_whitespace = decltype(Production::whitespace);

template <typename Production>
constexpr auto _production_defines_whitespace
    = lexy::_detail::is_detected<_detect_whitespace, Production>;

template <typename Production, typename WhitespaceProduction>
auto _production_whitespace()
{
    if constexpr (_production_defines_whitespace<Production>)
    {
        // We have whitespace defined in the production.
        return Production::whitespace;
    }
    else if constexpr (_production_defines_whitespace<WhitespaceProduction>)
    {
        // We have whitespace defined in the whitespace production.
        return WhitespaceProduction::whitespace;
    }

    // If we didn't have any cases, function returns void.
}
template <typename Production, typename WhitespaceProduction>
using production_whitespace = decltype(_production_whitespace<Production, WhitespaceProduction>());
} // namespace lexy

namespace lexy
{
template <typename To, typename... Args>
inline constexpr auto _is_convertible = false;
template <typename To, typename Arg>
inline constexpr auto _is_convertible<To, Arg> = std::is_convertible_v<Arg, To>;
template <>
inline constexpr auto _is_convertible<void> = true;

template <typename ParseState, typename Production>
using _detect_value_of =
    // We're testing a non-const ParseState on purpose, to handle cases where a user forgot to const
    // qualify value_of() (it causes a hard error instead of going to ::value).
    typename decltype(LEXY_DECLVAL(ParseState&).value_of(Production{}))::return_type;

template <typename Production>
using _detect_value = decltype(Production::value);

template <typename Production, typename Sink>
struct _sfinae_sink
{
    Sink _sink;

    using return_type = typename Sink::return_type;

    LEXY_FORCE_INLINE constexpr _sfinae_sink(Production, Sink&& sink) : _sink(LEXY_MOV(sink)) {}

    template <typename... Args>
    LEXY_FORCE_INLINE constexpr void operator()(Args&&... args)
    {
        if constexpr (!is_sink_callback_for<Sink, Args&&...>)
            // We're attempting to call a sink of Production with the given arguments, but no such
            // overload exists.
            static_assert(_detail::error<Production, Args...>,
                          "missing value sink callback overload for production");
        _sink(LEXY_FWD(args)...);
    }

    LEXY_FORCE_INLINE constexpr auto finish() &&
    {
        return LEXY_MOV(_sink).finish();
    }
};

template <typename Production, typename ParseState = void>
constexpr bool production_has_value_callback
    = lexy::_detail::is_detected<_detect_value_of, ParseState, Production>
      || lexy::_detail::is_detected<_detect_value, Production>;

template <typename Production, typename ParseState = void>
class production_value_callback
{
    static constexpr auto _get_value([[maybe_unused]] ParseState* state)
    {
        if constexpr (lexy::_detail::is_detected<_detect_value_of, ParseState, Production>)
            return state->value_of(Production{});
        else
            return Production::value;
    }
    using _type = decltype(_get_value(nullptr));

    static auto _return_type_callback()
    {
        if constexpr (lexy::is_callback<_type>)
            return _get_value(nullptr);
        else if constexpr (lexy::is_sink<_type, std::add_lvalue_reference_t<ParseState>>)
            return _get_value(nullptr).sink(LEXY_DECLVAL(ParseState&));
        else
            return _get_value(nullptr).sink();
    }

public:
    constexpr explicit production_value_callback(ParseState* state) : _state(state) {}

    template <typename State = ParseState, typename = std::enable_if_t<std::is_void_v<State>>>
    constexpr production_value_callback() : _state(nullptr)
    {}
    template <typename State = ParseState,
              typename       = std::enable_if_t<std::is_same_v<State, ParseState>>>
    constexpr explicit production_value_callback(State& state) : _state(&state)
    {}

    using return_type = typename decltype(_return_type_callback())::return_type;

    constexpr auto sink() const
    {
        if constexpr (lexy::is_sink<_type, std::add_lvalue_reference_t<ParseState>>)
        {
            return _sfinae_sink(Production{}, _get_value(_state).sink(*_state));
        }
        else
        {
            // We're attempting to obtain a sink for Production, but none was provided.
            // As Production uses a list rule, it needs a sink.
            static_assert(lexy::is_sink<_type>, "missing value sink for production");
            return _sfinae_sink(Production{}, _get_value(_state).sink());
        }
    }

    template <typename... Args>
    constexpr return_type operator()(Args&&... args) const
    {
        if constexpr (lexy::is_callback_with_state_for<_type, ParseState, Args&&...>
                      && !std::is_void_v<ParseState>)
        {
            return _get_value(_state)[*_state](LEXY_FWD(args)...);
        }
        else if constexpr (lexy::is_callback_for<_type, Args&&...>)
        {
            return _get_value(_state)(LEXY_FWD(args)...);
        }
        else if constexpr ((lexy::is_sink<_type>                                              //
                            || lexy::is_sink<_type, std::add_lvalue_reference_t<ParseState>>) //
                           &&_is_convertible<return_type, Args&&...>)
        {
            // We don't have a matching callback, but it is a single argument that has
            // the correct type already, or we return void and have no arguments.
            // Assume it came from the list sink and return the value without invoking a
            // callback.
            return (LEXY_FWD(args), ...);
        }
        else
        {
            // We're attempting to call the callback of Production with the given arguments, but no
            // such overload exists.
            static_assert(_detail::error<Production, Args...>,
                          "missing value callback overload for production");
        }
    }

private:
    ParseState* _state;
};
} // namespace lexy

#endif // LEXY_GRAMMAR_HPP_INCLUDED
