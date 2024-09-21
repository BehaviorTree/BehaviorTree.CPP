// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_FLAGS_HPP_INCLUDED
#define LEXY_DSL_FLAGS_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/error.hpp>

namespace lexy
{
struct duplicate_flag
{
    static LEXY_CONSTEVAL auto name()
    {
        return "duplicate flag";
    }
};
} // namespace lexy

namespace lexyd
{
template <const auto& Table, typename Token, typename Tag>
struct _sym;

template <typename FlagRule, auto Default, typename DuplicateError = void>
struct _flags : rule_base
{
    using _enum_type = LEXY_DECAY_DECLTYPE(Default);
    using _int_type  = std::underlying_type_t<_enum_type>;

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader>
        static constexpr bool _parse(_int_type& result, Context& context, Reader& reader)
        {
            result = _int_type(Default);

            while (true)
            {
                auto begin = reader.position();

                lexy::branch_parser_for<FlagRule, Reader> bp{};
                if (!bp.try_parse(context.control_block, reader))
                {
                    bp.cancel(context);
                    break;
                }

                if (!bp.template finish<lexy::pattern_parser<_enum_type>>(context, reader))
                    return false;

                auto flag = _int_type(bp.value());
                if ((result & flag) == flag)
                {
                    using tag = lexy::_detail::type_or<DuplicateError, lexy::duplicate_flag>;
                    auto err  = lexy::error<Reader, tag>(begin, reader.position());
                    context.on(_ev::error{}, err);
                    // We can trivially recover.
                }
                result |= flag;
            }

            return true;
        }

        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            _int_type result{};
            if (!_parse(result, context, reader))
                return false;
            return NextParser::parse(context, reader, LEXY_FWD(args)..., _enum_type(result));
        }
    };

    template <typename Tag>
    static constexpr _flags<FlagRule, Default, Tag> error = {};
};

template <auto Default, const auto& Table, typename Token, typename Tag>
constexpr auto flags(_sym<Table, Token, Tag> flag_rule)
{
    using table_type = LEXY_DECAY_DECLTYPE(Table);
    using enum_type  = LEXY_DECAY_DECLTYPE(Default);
    static_assert(std::is_same_v<enum_type, typename table_type::mapped_type>);
    static_assert(std::is_enum_v<enum_type>);

    return _flags<decltype(flag_rule), Default>{};
}
template <const auto& Table, typename Token, typename Tag>
constexpr auto flags(_sym<Table, Token, Tag> flag_rule)
{
    using table_type = LEXY_DECAY_DECLTYPE(Table);
    using enum_type  = typename table_type::mapped_type;
    static_assert(std::is_enum_v<enum_type>);

    return _flags<decltype(flag_rule), enum_type{}>{};
}
} // namespace lexyd

namespace lexyd
{
template <typename Rule, auto If, auto Else>
struct _flag : rule_base
{
    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            lexy::branch_parser_for<Rule, Reader> branch{};
            if (branch.try_parse(context.control_block, reader))
                return branch.template finish<NextParser>(context, reader, LEXY_FWD(args)..., If);
            else
            {
                branch.cancel(context);
                return NextParser::parse(context, reader, LEXY_FWD(args)..., Else);
            }
        }
    };
};

template <auto If, auto Else = LEXY_DECAY_DECLTYPE(If){}, typename Rule>
constexpr auto flag(Rule)
{
    LEXY_REQUIRE_BRANCH_RULE(Rule, "flag()");
    return _flag<Rule, If, Else>{};
}

template <typename Rule>
constexpr auto flag(Rule rule)
{
    return flag<true, false>(rule);
}
} // namespace lexyd

#endif // LEXY_DSL_FLAGS_HPP_INCLUDED

