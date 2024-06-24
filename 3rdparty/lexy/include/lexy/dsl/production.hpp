// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PRODUCTION_HPP_INCLUDED
#define LEXY_DSL_PRODUCTION_HPP_INCLUDED

#include <lexy/action/base.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/error.hpp>

namespace lexyd
{
/// Parses the rule of the production as if it were part of the current production.
template <typename Production>
constexpr auto inline_ = lexy::production_rule<Production>{};
} // namespace lexyd

namespace lexyd
{
template <typename Production, typename Context, typename Reader>
/* not force inline */ constexpr bool _parse_production(Context& context, Reader& reader)
{
    using parser = lexy::parser_for<lexy::production_rule<Production>, lexy::_detail::final_parser>;
    return parser::parse(context, reader);
}
template <typename ProductionParser, typename Context, typename Reader>
/* not force inline */ constexpr bool _finish_production(ProductionParser& parser, Context& context,
                                                         Reader& reader)
{
    return parser.template finish<lexy::_detail::final_parser>(context, reader);
}

template <typename Production>
struct _prd
// If the production defines whitespace, it can't be a branch production.
: std::conditional_t<lexy::_production_defines_whitespace<Production>, rule_base,
                     _copy_base<lexy::production_rule<Production>>>
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            // Create a context for the production and parse the context there.
            auto sub_context = context.sub_context(Production{});
            sub_context.on(_ev::production_start{}, reader.position());

            // Skip initial whitespace if the rule changed.
            if constexpr (lexy::_production_defines_whitespace<Production>)
            {
                if (!lexy::whitespace_parser<decltype(sub_context),
                                             lexy::pattern_parser<>>::parse(sub_context, reader))
                {
                    sub_context.on(_ev::production_cancel{}, reader.position());
                    return false;
                }
            }

            if (_parse_production<Production>(sub_context, reader))
            {
                sub_context.on(_ev::production_finish{}, reader.position());

                using continuation = lexy::_detail::context_finish_parser<NextParser>;
                return continuation::parse(context, reader, sub_context, LEXY_FWD(args)...);
            }
            else
            {
                // Cancel.
                sub_context.on(_ev::production_cancel{}, reader.position());
                return false;
            }
        }
    };

    template <typename Reader>
    struct bp
    {
        lexy::production_branch_parser<Production, Reader> parser;
        typename Reader::iterator                          begin;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            begin = reader.position();
            return parser.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            // Cancel in a new context.
            auto sub_context = context.sub_context(Production{});
            sub_context.on(_ev::production_start{}, begin);
            parser.cancel(sub_context);
            sub_context.on(_ev::production_cancel{}, begin);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            static_assert(!lexy::_production_defines_whitespace<Production>);

            // Finish the production in a new context.
            auto sub_context = context.sub_context(Production{});
            sub_context.on(_ev::production_start{}, begin);
            if (_finish_production(parser, sub_context, reader))
            {
                sub_context.on(_ev::production_finish{}, reader.position());

                using continuation = lexy::_detail::context_finish_parser<NextParser>;
                return continuation::parse(context, reader, sub_context, LEXY_FWD(args)...);
            }
            else
            {
                // Cancel.
                sub_context.on(_ev::production_cancel{}, reader.position());
                return false;
            }
        }
    };
};

/// Parses the production.
template <typename Production>
constexpr auto p = _prd<Production>{};
} // namespace lexyd

namespace lexy
{
struct max_recursion_depth_exceeded
{
    static LEXY_CONSTEVAL auto name()
    {
        return "maximum recursion depth exceeded";
    }
};
} // namespace lexy

namespace lexyd
{
template <typename Production, typename DepthError = void>
struct _recb : branch_base
{
    template <typename NextParser>
    struct _depth_handler
    {
        template <typename Context, typename Reader>
        static constexpr bool increment_depth(Context& context, Reader& reader)
        {
            auto control_block = context.control_block;
            LEXY_ASSERT(control_block->max_depth > 0,
                        "dsl::recurse_branch<P> is disabled in this context");

            // We're doing a recursive call, check for an exceeded depth.
            if (control_block->cur_depth >= control_block->max_depth)
            {
                // We did report error from which we can't recover.
                using tag = lexy::_detail::type_or<DepthError, lexy::max_recursion_depth_exceeded>;
                auto err  = lexy::error<Reader, tag>(reader.position());
                context.on(_ev::error{}, err);
                return false;
            }

            ++control_block->cur_depth;
            return true;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            auto control_block = context.control_block;
            --control_block->cur_depth;
            return NextParser::parse(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename Reader>
    struct bp
    {
        LEXY_REQUIRE_BRANCH_RULE(lexy::production_rule<Production>, "recurse_branch");

        using impl = lexy::branch_parser_for<_prd<Production>, Reader>;
        impl _impl;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return _impl.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            _impl.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            using depth = _depth_handler<NextParser>;
            if (!depth::increment_depth(context, reader))
                return false;
            return _impl.template finish<depth>(context, reader, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using depth = _depth_handler<NextParser>;
            if (!depth::increment_depth(context, reader))
                return false;

            return lexy::parser_for<_prd<Production>, depth>::parse(context, reader,
                                                                    LEXY_FWD(args)...);
        }
    };

    template <typename Tag>
    static constexpr _recb<Production, Tag> max_depth_error = _recb<Production, Tag>{};
};

template <typename Production, typename DepthError = void>
struct _rec : rule_base
{
    template <typename NextParser>
    using p = lexy::parser_for<_recb<Production, DepthError>, NextParser>;

    template <typename Tag>
    static constexpr _rec<Production, Tag> max_depth_error = _rec<Production, Tag>{};
};

/// Parses the production, recursively.
/// `dsl::p` requires that the production is already defined in order to propagate a branch
/// condition outwards.
template <typename Production>
constexpr auto recurse = _rec<Production>{};

template <typename Production>
constexpr auto recurse_branch = _recb<Production>{};
} // namespace lexyd

#endif // LEXY_DSL_PRODUCTION_HPP_INCLUDED

