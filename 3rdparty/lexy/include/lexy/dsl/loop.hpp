// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_LOOP_HPP_INCLUDED
#define LEXY_DSL_LOOP_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>

namespace lexyd
{
struct _break : unconditional_branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename LoopControl, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context&, Reader&, LoopControl& cntrl, Args&&...)
        {
            cntrl.loop_break = true;
            return true;
        }
    };

    template <typename Reader>
    using bp = lexy::unconditional_branch_parser<_break, Reader>;
};

/// Exits a loop().
constexpr auto break_ = _break{};
} // namespace lexyd

namespace lexyd
{
template <typename Rule>
struct _loop : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            struct loop_control_t
            {
                bool loop_break = false;
            } control;

            while (!control.loop_break)
            {
                using parser = lexy::parser_for<Rule, lexy::pattern_parser<loop_control_t>>;
                if (!parser::parse(context, reader, control))
                    return false;
            }

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Repeatedly matches the rule until a break rule matches.
template <typename Rule>
constexpr auto loop(Rule)
{
    return _loop<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Branch>
struct _whl : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Branch, Reader> branch{};
            while (branch.try_parse(context.control_block, reader))
            {
                if (!branch.template finish<lexy::pattern_parser<>>(context, reader))
                    return false;
            }
            branch.cancel(context);

            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };
};

/// Matches the branch rule as often as possible.
template <typename Rule>
constexpr auto while_(Rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "while()");
    return _whl<Rule>{};
}
} // namespace lexyd

namespace lexyd
{
/// Matches the rule at least once, then as often as possible.
template <typename Rule>
constexpr auto while_one(Rule rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "while_one()");
    return rule >> while_(rule);
}
} // namespace lexyd

namespace lexyd
{
/// Matches then once, then `while_(condition >> then)`.
template <typename Then, typename Condition>
constexpr auto do_while(Then then, Condition condition)
{
    return _maybe_branch(then, while_(condition >> then));
}
} // namespace lexyd

#endif // LEXY_DSL_LOOP_HPP_INCLUDED

