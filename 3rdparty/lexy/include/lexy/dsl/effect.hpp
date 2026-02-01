// Copyright (C) 2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_EFFECT_HPP_INCLUDED
#define LEXY_DSL_EFFECT_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/dsl/base.hpp>

namespace lexyd
{
template <typename EffRule, typename State>
using _detect_eff_fn = decltype(EffRule::_fn()(LEXY_DECLVAL(State&)));

template <LEXY_NTTP_PARAM Fn>
struct _eff : rule_base
{
    static constexpr auto _fn()
    {
        return Fn;
    }

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            using control_block_type = LEXY_DECAY_DECLTYPE(*context.control_block);
            using state_type         = typename control_block_type::state_type;

            if constexpr (lexy::_detail::is_detected<_detect_eff_fn, _eff, state_type>)
            {
                using return_type = decltype(Fn(*context.control_block->parse_state));
                if constexpr (std::is_void_v<return_type>)
                {
                    Fn(*context.control_block->parse_state);
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                }
                else
                {
                    return NextParser::parse(context, reader, LEXY_FWD(args)...,
                                             Fn(*context.control_block->parse_state));
                }
            }
            else
            {
                using return_type = decltype(Fn());
                if constexpr (std::is_void_v<return_type>)
                {
                    Fn();
                    return NextParser::parse(context, reader, LEXY_FWD(args)...);
                }
                else
                {
                    return NextParser::parse(context, reader, LEXY_FWD(args)..., Fn());
                }
            }
        }
    };
};

/// Invokes Fn and produces its value as result.
template <LEXY_NTTP_PARAM Fn>
constexpr auto effect = _eff<Fn>{};
} // namespace lexyd

#endif // LEXY_DSL_EFFECT_HPP_INCLUDED

