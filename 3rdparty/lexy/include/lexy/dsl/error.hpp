// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_ERROR_HPP_INCLUDED
#define LEXY_DSL_ERROR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Rule>
struct _err_production
{
    static constexpr auto name                = "<error>";
    static constexpr auto max_recursion_depth = 0;
    static constexpr auto rule                = Rule{};
};

template <typename Tag, typename Rule>
struct _err : unconditional_branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&...)
        {
            auto begin = reader.position();
            auto end   = reader.position();
            if constexpr (!std::is_same_v<Rule, void>)
            {
                auto backtrack = reader.current();

                // We match a dummy production that only consists of the rule.
                lexy::do_action<
                    _err_production<Rule>,
                    lexy::match_action<void, Reader>::template result_type>(lexy::_mh(),
                                                                            context.control_block
                                                                                ->parse_state,
                                                                            reader);
                end = reader.position();
                reader.reset(LEXY_MOV(backtrack));
            }

            auto err = lexy::error<Reader, Tag>(begin, end);
            context.on(_ev::error{}, err);
            return false;
        }
    };
    template <typename Reader>
    using bp = lexy::unconditional_branch_parser<_err, Reader>;

    /// Adds a rule whose match will be part of the error location.
    template <typename R>
    constexpr auto operator()(R) const
    {
        return _err<Tag, R>{};
    }
};

/// Matches nothing, produces an error with the given tag.
template <typename Tag>
constexpr auto error = _err<Tag, void>{};
} // namespace lexyd

namespace lexyd
{
template <typename Branch, typename Error>
struct _must : branch_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Try and parse the branch.
            lexy::branch_parser_for<Branch, Reader> branch{};
            if (branch.try_parse(context.control_block, reader))
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            branch.cancel(context);

            // The branch wasn't taken, so we fail with the specific error by parsing Error.
            auto result = lexy::parser_for<Error, lexy::pattern_parser<>>::parse(context, reader);
            LEXY_ASSERT(!result, "error must not recover");

            return false;
        }
    };

    // As a branch we parse it exactly the same.
    template <typename Reader>
    using bp = lexy::branch_parser_for<Branch, Reader>;
};

template <typename Branch>
struct _must_dsl
{
    template <typename Tag>
    struct _err : _must<Branch, lexyd::_err<Tag, void>>
    {
        template <typename Rule>
        constexpr auto operator()(Rule rule) const
        {
            auto err = lexyd::error<Tag>(rule);
            return _must<Branch, decltype(err)>{};
        }
    };

    template <typename Tag>
    static constexpr _err<Tag> error = _err<Tag>{};
};

/// Tries to parse `Branch` and raises a specific error on failure.
/// It can still be used as a branch rule; then behaves exactly like `Branch.`
template <typename Branch>
constexpr auto must(Branch)
{
    LEXY_REQUIRE_BRANCH_RULE(Branch, "must()");
    static_assert(!lexy::is_unconditional_branch_rule<Branch>);
    return _must_dsl<Branch>{};
}
} // namespace lexyd

#endif // LEXY_DSL_ERROR_HPP_INCLUDED

