// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_HPP_INCLUDED
#define LEXY_DSL_HPP_INCLUDED

#include <lexy/dsl/any.hpp>
#include <lexy/dsl/ascii.hpp>
#include <lexy/dsl/bits.hpp>
#include <lexy/dsl/bom.hpp>
#include <lexy/dsl/brackets.hpp>
#include <lexy/dsl/branch.hpp>
#include <lexy/dsl/byte.hpp>
#include <lexy/dsl/capture.hpp>
#include <lexy/dsl/case_folding.hpp>
#include <lexy/dsl/char_class.hpp>
#include <lexy/dsl/choice.hpp>
#include <lexy/dsl/code_point.hpp>
#include <lexy/dsl/combination.hpp>
#include <lexy/dsl/context_counter.hpp>
#include <lexy/dsl/context_flag.hpp>
#include <lexy/dsl/context_identifier.hpp>
#include <lexy/dsl/delimited.hpp>
#include <lexy/dsl/digit.hpp>
#include <lexy/dsl/effect.hpp>
#include <lexy/dsl/eof.hpp>
#include <lexy/dsl/error.hpp>
#include <lexy/dsl/expression.hpp>
#include <lexy/dsl/flags.hpp>
#include <lexy/dsl/follow.hpp>
#include <lexy/dsl/identifier.hpp>
#include <lexy/dsl/if.hpp>
#include <lexy/dsl/integer.hpp>
#include <lexy/dsl/list.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/lookahead.hpp>
#include <lexy/dsl/loop.hpp>
#include <lexy/dsl/member.hpp>
#include <lexy/dsl/newline.hpp>
#include <lexy/dsl/operator.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/dsl/parse_as.hpp>
#include <lexy/dsl/peek.hpp>
#include <lexy/dsl/position.hpp>
#include <lexy/dsl/production.hpp>
#include <lexy/dsl/punctuator.hpp>
#include <lexy/dsl/recover.hpp>
#include <lexy/dsl/repeat.hpp>
#include <lexy/dsl/return.hpp>
#include <lexy/dsl/scan.hpp>
#include <lexy/dsl/separator.hpp>
#include <lexy/dsl/sequence.hpp>
#include <lexy/dsl/sign.hpp>
#include <lexy/dsl/subgrammar.hpp>
#include <lexy/dsl/symbol.hpp>
#include <lexy/dsl/terminator.hpp>
#include <lexy/dsl/times.hpp>
#include <lexy/dsl/token.hpp>
#include <lexy/dsl/unicode.hpp>
#include <lexy/dsl/until.hpp>
#include <lexy/dsl/whitespace.hpp>

#if LEXY_EXPERIMENTAL
#    include <lexy/dsl/parse_tree_node.hpp>
#endif

#endif // LEXY_DSL_HPP_INCLUDED

