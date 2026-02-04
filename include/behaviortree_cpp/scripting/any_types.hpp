/*  Copyright (C) 2022-2025 Davide Faconti -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "behaviortree_cpp/utils/safe_any.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace BT::Scripting
{

enum class TokenType
{
  // Literals
  Integer,
  Real,
  String,
  Boolean,
  // Identifier
  Identifier,
  // Arithmetic
  Plus,
  Minus,
  Star,
  Slash,
  DotDot,
  // Bitwise
  Ampersand,
  Pipe,
  Caret,
  Tilde,
  // Logical
  AmpAmp,
  PipePipe,
  Bang,
  // Comparison
  EqualEqual,
  BangEqual,
  Less,
  Greater,
  LessEqual,
  GreaterEqual,
  // Assignment
  ColonEqual,
  Equal,
  PlusEqual,
  MinusEqual,
  StarEqual,
  SlashEqual,
  // Ternary
  Question,
  Colon,
  // Delimiters
  LeftParen,
  RightParen,
  Semicolon,
  // Control
  EndOfInput,
  Error
};

/// Lightweight token referencing the source string via string_view.
/// The source string must outlive the token vector.
struct Token
{
  TokenType type = TokenType::Error;
  std::string_view text;
  size_t pos = 0;
};

std::vector<Token> tokenize(const std::string& source);

}  // namespace BT::Scripting
