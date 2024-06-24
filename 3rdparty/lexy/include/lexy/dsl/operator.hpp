// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_OPERATOR_HPP_INCLUDED
#define LEXY_DSL_OPERATOR_HPP_INCLUDED

#include <lexy/_detail/detect.hpp>
#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/token.hpp>

namespace lexyd
{
template <typename Condition, typename... R>
struct _br;
template <typename... R>
struct _seq_impl;
} // namespace lexyd

//=== tag type ===//
namespace lexy
{
template <typename Literal>
struct _op
{};
// GCC is buggy with auto parameters.
template <typename T, T Value>
struct _opv
{
    constexpr operator LEXY_DECAY_DECLTYPE(Value)() const
    {
        return Value;
    }
};

#if LEXY_HAS_NTTP
template <auto Operator>
#else
template <const auto& Operator>
#endif
using op = typename LEXY_DECAY_DECLTYPE(Operator)::op_tag_type;
} // namespace lexy

//=== op rule ===//
namespace lexy::_detail
{
template <typename... Literals>
struct op_lit_list
{
    static constexpr auto size = sizeof...(Literals);

    template <typename Encoding>
    static LEXY_CONSTEVAL auto _build_trie()
    {
        auto result = make_empty_trie<Encoding, Literals...>();

        auto value      = std::size_t(0);
        auto char_class = std::size_t(0);
        ((result.node_value[Literals::lit_insert(result, 0, char_class)] = value++,
          char_class += Literals::lit_char_classes.size),
         ...);

        return result;
    }
    template <typename Encoding>
    static constexpr lit_trie_for<Encoding, Literals...> trie = _build_trie<Encoding>();

    template <typename... T>
    constexpr auto operator+(op_lit_list<T...>) const
    {
        return op_lit_list<Literals..., T...>{};
    }
};

template <typename Reader>
struct parsed_operator
{
    typename Reader::marker cur;
    std::size_t             idx;
};

template <typename OpList, typename Reader>
constexpr auto parse_operator(Reader& reader)
{
    using encoding   = typename Reader::encoding;
    using op_matcher = lexy::_detail::lit_trie_matcher<OpList::template trie<encoding>, 0>;

    auto begin = reader.current();
    auto op    = op_matcher::try_match(reader);
    return parsed_operator<Reader>{begin, op};
}
} // namespace lexy::_detail

namespace lexyd
{
template <typename Tag, typename Reader>
using _detect_op_tag_ctor = decltype(Tag(LEXY_DECLVAL(Reader).position()));

template <typename Tag, typename Reader, typename Context>
using _detect_op_tag_ctor_with_state
    = decltype(Tag(*LEXY_DECLVAL(Context).control_block->parse_state,
                   LEXY_DECLVAL(Reader).position()));

template <typename TagType, typename Literal, typename... R>
struct _op : branch_base
{
    using op_tag_type = TagType;
    using op_literals = lexy::_detail::op_lit_list<Literal>;

    template <typename NextParser, typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool op_finish(Context& context, Reader& reader,
                                           lexy::_detail::parsed_operator<Reader> op,
                                           Args&&... args)
    {
        context.on(_ev::token{}, typename Literal::token_type{}, op.cur.position(),
                   reader.position());

        using continuation
            = lexy::whitespace_parser<Context, lexy::parser_for<_seq_impl<R...>, NextParser>>;
        if constexpr (std::is_void_v<TagType>)
            return continuation::parse(context, reader, LEXY_FWD(args)...);
        else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor_with_state, op_tag_type,
                                                      Reader, Context>)
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       op_tag_type(*context.control_block->parse_state, op.pos));
        else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor, op_tag_type, Reader>)
            return continuation::parse(context, reader, LEXY_FWD(args)...,
                                       op_tag_type(op.cur.position()));
        else
            return continuation::parse(context, reader, LEXY_FWD(args)..., op_tag_type{});
    }

    template <typename Reader>
    struct bp
    {
        lexy::branch_parser_for<Literal, Reader> impl;

        template <typename ControlBlock>
        constexpr auto try_parse(const ControlBlock* cb, const Reader& reader)
        {
            return impl.try_parse(cb, reader);
        }

        template <typename Context>
        constexpr void cancel(Context& context)
        {
            impl.cancel(context);
        }

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            using continuation = lexy::parser_for<_seq_impl<R...>, NextParser>;

            if constexpr (std::is_void_v<TagType>)
                return impl.template finish<continuation>(context, reader, LEXY_FWD(args)...);
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor_with_state,
                                                          op_tag_type, Reader, Context>)
                return impl
                    .template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                   op_tag_type(*context.control_block->parse_state,
                                                               reader.position()));
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor, op_tag_type, Reader>)
                return impl.template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                          op_tag_type(reader.position()));
            else
                return impl.template finish<continuation>(context, reader, LEXY_FWD(args)...,
                                                          op_tag_type{});
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            [[maybe_unused]] auto pos = reader.position();

            using continuation
                = lexy::parser_for<Literal, lexy::parser_for<_seq_impl<R...>, NextParser>>;
            if constexpr (std::is_void_v<TagType>)
                return continuation::parse(context, reader, LEXY_FWD(args)...);
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor_with_state,
                                                          op_tag_type, Reader, Context>)
                return continuation::parse(context, reader, LEXY_FWD(args)...,
                                           op_tag_type(*context.control_block->parse_state, pos));
            else if constexpr (lexy::_detail::is_detected<_detect_op_tag_ctor, op_tag_type, Reader>)
                return continuation::parse(context, reader, LEXY_FWD(args)..., op_tag_type(pos));
            else
                return continuation::parse(context, reader, LEXY_FWD(args)..., op_tag_type{});
        }
    };
};

template <typename Literal>
constexpr auto op(Literal)
{
    static_assert(lexy::is_literal_rule<Literal>);
    return _op<lexy::_op<Literal>, Literal>{};
}
template <typename Literal, typename... R>
constexpr auto op(_br<Literal, R...>)
{
    static_assert(lexy::is_literal_rule<Literal>,
                  "condition in the operator must be a literal rule");
    return _op<lexy::_op<Literal>, Literal, R...>{};
}

template <typename Tag, typename Literal>
constexpr auto op(Literal)
{
    static_assert(lexy::is_literal_rule<Literal>);
    return _op<Tag, Literal>{};
}
template <typename Tag, typename Literal, typename... R>
constexpr auto op(_br<Literal, R...>)
{
    static_assert(lexy::is_literal_rule<Literal>,
                  "condition in the operator must be a literal rule");
    return _op<Tag, Literal, R...>{};
}

template <auto Tag, typename Literal>
constexpr auto op(Literal)
{
    static_assert(lexy::is_literal_rule<Literal>);
    return _op<lexy::_opv<LEXY_DECAY_DECLTYPE(Tag), Tag>, Literal>{};
}
template <auto Tag, typename Literal, typename... R>
constexpr auto op(_br<Literal, R...>)
{
    static_assert(lexy::is_literal_rule<Literal>,
                  "condition in the operator must be a literal rule");
    return _op<lexy::_opv<LEXY_DECAY_DECLTYPE(Tag), Tag>, Literal, R...>{};
}
} // namespace lexyd

//=== op choice ===//
namespace lexyd
{
template <typename... Ops>
struct _opc : branch_base
{
    using op_literals = decltype((typename Ops::op_literals{} + ...));

    template <typename NextParser, typename Context, typename Reader, typename... Args>
    LEXY_PARSER_FUNC static bool op_finish(Context& context, Reader& reader,
                                           lexy::_detail::parsed_operator<Reader> op,
                                           Args&&... args)
    {
        auto result = false;

        auto cur_idx = std::size_t(0);
        (void)((cur_idx == op.idx
                    ? (result = Ops::template op_finish<NextParser>(context, reader, op,
                                                                    LEXY_FWD(args)...),
                       true)
                    : (++cur_idx, false))
               || ...);

        return result;
    }

    template <typename Reader>
    struct bp
    {
        lexy::_detail::parsed_operator<Reader> op;
        typename Reader::marker                end;

        constexpr auto try_parse(const void*, Reader reader)
        {
            op  = lexy::_detail::parse_operator<op_literals>(reader);
            end = reader.current();
            return op.idx < op_literals::size;
        }

        template <typename Context>
        constexpr void cancel(Context&)
        {}

        template <typename NextParser, typename Context, typename... Args>
        LEXY_PARSER_FUNC bool finish(Context& context, Reader& reader, Args&&... args)
        {
            reader.reset(end);
            return op_finish<NextParser>(context, reader, op, LEXY_FWD(args)...);
        }
    };

    template <typename NextParser>
    struct p
    {
        template <typename Context, typename Reader, typename... Args>
        LEXY_PARSER_FUNC static bool parse(Context& context, Reader& reader, Args&&... args)
        {
            bp<Reader> impl{};
            if (!impl.try_parse(context.control_block, reader))
            {
                auto err = lexy::error<Reader, lexy::expected_literal_set>(impl.op.cur.position());
                context.on(_ev::error{}, err);
                return false;
            }
            else
            {
                return impl.template finish<NextParser>(context, reader, LEXY_FWD(args)...);
            }
        }
    };
};

template <typename T1, typename L1, typename... R1, typename T2, typename L2, typename... R2>
constexpr auto operator/(_op<T1, L1, R1...> lhs, _op<T2, L2, R2...> rhs)
{
    return _opc<decltype(lhs), decltype(rhs)>{};
}
template <typename... Ops, typename T2, typename L2, typename... R2>
constexpr auto operator/(_opc<Ops...>, _op<T2, L2, R2...> rhs)
{
    return _opc<Ops..., decltype(rhs)>{};
}
template <typename T1, typename L1, typename... R1, typename... Ops>
constexpr auto operator/(_op<T1, L1, R1...> lhs, _opc<Ops...>)
{
    return _opc<decltype(lhs), Ops...>{};
}
template <typename... O1, typename... O2>
constexpr auto operator/(_opc<O1...>, _opc<O2...>)
{
    return _opc<O1..., O2...>{};
}
} // namespace lexyd

#endif // LEXY_DSL_OPERATOR_HPP_INCLUDED
