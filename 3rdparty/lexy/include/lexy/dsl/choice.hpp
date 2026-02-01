// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_CHOICE_HPP_INCLUDED
#define LEXY_DSL_CHOICE_HPP_INCLUDED

#include <lexy/_detail/tuple.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct exhausted_choice
{
    static LEXY_CONSTEVAL auto name()
    {
        return "exhausted choice";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename... R>
struct _chc
// Only make it a branch rule if it doesn't have an unconditional branch.
// A choice rule with an unconditional branch is itself an unconditional branch, which is most
// likely a bug.
: std::conditional_t<(lexy::is_unconditional_branch_rule<R> || ...), rule_base, branch_base>
{
    static constexpr auto _any_unconditional = (lexy::is_unconditional_branch_rule<R> || ...);

    template <typename Reader, typename Indices = lexy::_detail::make_index_sequence<sizeof...(R)>>
    struct bp;
    template <typename Reader, std::size_t... Idx>
    struct bp<Reader, lexy::_detail::index_sequence<Idx...>>
    {
        template <typename Rule>
        using rp = lexy::branch_parser_for<Rule, Reader>;

        lexy::_detail::tuple<rp<R>...> r_parsers;
        std::size_t                    branch_idx;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
            -> std::conditional_t<_any_unconditional, std::true_type, bool>
        {
            auto try_r = [&](std::size_t idx, auto& parser) {
                if (!parser.try_parse(cb, reader))
                    return false;

                branch_idx = idx;
                return true;
            };

            // Need to try each possible branch.
            auto found_branch = (try_r(Idx, r_parsers.template get<Idx>()) || ...);
            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return {};
            }
            else
            {
                return found_branch;
            }
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            // Need to cancel all branches.
            (r_parsers.template get<Idx>().cancel(context), ...);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            // Need to call finish on the selected branch, and cancel on all others before that.
            auto result = false;
            (void)((Idx == branch_idx
                        ? (result
                           = r_parsers.template get<Idx>()
                                 .template finish<NextParser>(context, reader, LEXY_FWD(args)...),
                           true)
                        : (r_parsers.template get<Idx>().cancel(context), false))
                   || ...);
            return result;
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto result = false;
            auto try_r  = [&](auto&& parser) {
                if (!parser.try_parse(context.control_block, reader))
                {
                    parser.cancel(context);
                    return false;
                }

                // LEXY_FWD(args) will break MSVC builds targeting C++17.
                result = parser.template finish<NextParser>(context, reader,
                                                            static_cast<Args&&>(args)...);
                return true;
            };

            // Try to parse each branch in order.
            auto found_branch = (try_r(lexy::branch_parser_for<R, Reader>{}) || ...);
            if constexpr (_any_unconditional)
            {
                LEXY_ASSERT(found_branch,
                            "it is unconditional, but we still haven't found a rule?!");
                return result;
            }
            else
            {
                if (found_branch)
                    return result;

                auto err = lexy::error<Reader, lexy::exhausted_choice>(reader.position());
                context.on(_ev::error{}, err);
                return false;
            }
        }
    };
};

template <typename R, typename S>
constexpr auto operator|(R, S)
{
    LEXY_REQUIRE_BRANCH_RULE(R, "choice");
    LEXY_REQUIRE_BRANCH_RULE(S, "choice");
    return _chc<R, S>{};
}
template <typename... R, typename S>
constexpr auto operator|(_chc<R...>, S)
{
    LEXY_REQUIRE_BRANCH_RULE(S, "choice");
    return _chc<R..., S>{};
}
template <typename R, typename... S>
constexpr auto operator|(R, _chc<S...>)
{
    LEXY_REQUIRE_BRANCH_RULE(R, "choice");
    return _chc<R, S...>{};
}
template <typename... R, typename... S>
constexpr auto operator|(_chc<R...>, _chc<S...>)
{
    return _chc<R..., S...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_CHOICE_HPP_INCLUDED

