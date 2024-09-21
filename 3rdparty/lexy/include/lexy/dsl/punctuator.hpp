// Copyright (C) 2020-2024 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_DSL_PUNCTUATOR_HPP_INCLUDED
#define LEXY_DSL_PUNCTUATOR_HPP_INCLUDED

#include <lexy/dsl/base.hpp>
#include <lexy/dsl/literal.hpp>

namespace lexyd
{
#define LEXY_PUNCT(Name, String)                                                                   \
    struct _##Name : LEXY_NTTP_STRING(_lit, String)                                                \
    {};                                                                                            \
    inline constexpr auto(Name) = _##Name {}

LEXY_PUNCT(period, ".");
LEXY_PUNCT(comma, ",");
LEXY_PUNCT(colon, ":");
LEXY_PUNCT(double_colon, "::");
LEXY_PUNCT(semicolon, ";");

LEXY_PUNCT(exclamation_mark, "!");
LEXY_PUNCT(question_mark, "?");

LEXY_PUNCT(hyphen, "-");
LEXY_PUNCT(slash, "/");
LEXY_PUNCT(backslash, "\\");
LEXY_PUNCT(apostrophe, "'");
LEXY_PUNCT(ampersand, "&");
LEXY_PUNCT(caret, "^");
LEXY_PUNCT(asterisk, "*");
LEXY_PUNCT(tilde, "~");
LEXY_PUNCT(vbar, "|");

LEXY_PUNCT(hash_sign, "#");
LEXY_PUNCT(dollar_sign, "$");
LEXY_PUNCT(at_sign, "@");
LEXY_PUNCT(percent_sign, "%");
LEXY_PUNCT(equal_sign, "=");

#undef LEXY_PUNCT
} // namespace lexyd

#endif // LEXY_DSL_PUNCTUATOR_HPP_INCLUDED

