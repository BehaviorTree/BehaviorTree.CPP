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

#include "behaviortree_cpp/scripting/any_types.hpp"

#include <cctype>

namespace BT::Scripting
{

namespace
{

bool isIdentStart(char c)
{
  return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_' || c == '@';
}

bool isIdentChar(char c)
{
  return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool isDigit(char c)
{
  return std::isdigit(static_cast<unsigned char>(c)) != 0;
}

bool isHexDigit(char c)
{
  return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}

// Consume trailing garbage after a malformed number token.
void consumeTrailingGarbage(const std::string& source, size_t len, size_t& i)
{
  while(i < len && (isIdentChar(source[i]) || source[i] == '.'))
  {
    ++i;
  }
}

struct NumberResult
{
  bool is_real = false;
  bool has_error = false;
};

NumberResult scanHexNumber(const std::string& source, size_t len, size_t& i)
{
  NumberResult result;
  i += 2;  // skip "0x"/"0X"
  if(i >= len || !isHexDigit(source[i]))
  {
    result.has_error = true;
  }
  else
  {
    while(i < len && isHexDigit(source[i]))
    {
      ++i;
    }
  }
  // Hex numbers don't support dot or exponent
  if(i < len && (source[i] == '.' || isIdentChar(source[i])))
  {
    result.has_error = true;
    consumeTrailingGarbage(source, len, i);
  }
  return result;
}

NumberResult scanDecimalNumber(const std::string& source, size_t len, size_t& i)
{
  NumberResult result;

  // Integer part
  while(i < len && isDigit(source[i]))
  {
    ++i;
  }
  // Fractional part
  if(i < len && source[i] == '.')
  {
    // Distinguish from ".." (concat operator)
    if(i + 1 < len && source[i + 1] == '.')
    {
      // Stop here: "65.." is Integer("65") + DotDot
    }
    else if(i + 1 < len && isDigit(source[i + 1]))
    {
      result.is_real = true;
      ++i;  // consume '.'
      while(i < len && isDigit(source[i]))
      {
        ++i;
      }
    }
    else
    {
      // "65." or "65.x" -- incomplete real
      result.has_error = true;
      ++i;  // consume the dot
      consumeTrailingGarbage(source, len, i);
    }
  }
  // Exponent (only for decimal numbers)
  if(!result.has_error && i < len && (source[i] == 'e' || source[i] == 'E'))
  {
    result.is_real = true;
    ++i;  // consume 'e'/'E'
    if(i < len && (source[i] == '+' || source[i] == '-'))
    {
      ++i;  // consume sign
    }
    if(i >= len || !isDigit(source[i]))
    {
      result.has_error = true;
    }
    else
    {
      while(i < len && isDigit(source[i]))
      {
        ++i;
      }
    }
  }
  // Trailing alpha (e.g. "3foo", "65.43foo")
  if(!result.has_error && i < len && isIdentStart(source[i]))
  {
    result.has_error = true;
    while(i < len && isIdentChar(source[i]))
    {
      ++i;
    }
  }
  return result;
}

TokenType matchTwoCharOp(char c, char next)
{
  if(c == '.' && next == '.')
    return TokenType::DotDot;
  if(c == '&' && next == '&')
    return TokenType::AmpAmp;
  if(c == '|' && next == '|')
    return TokenType::PipePipe;
  if(c == '=' && next == '=')
    return TokenType::EqualEqual;
  if(c == '!' && next == '=')
    return TokenType::BangEqual;
  if(c == '<' && next == '=')
    return TokenType::LessEqual;
  if(c == '>' && next == '=')
    return TokenType::GreaterEqual;
  if(c == ':' && next == '=')
    return TokenType::ColonEqual;
  if(c == '+' && next == '=')
    return TokenType::PlusEqual;
  if(c == '-' && next == '=')
    return TokenType::MinusEqual;
  if(c == '*' && next == '=')
    return TokenType::StarEqual;
  if(c == '/' && next == '=')
    return TokenType::SlashEqual;
  return TokenType::Error;
}

TokenType matchSingleCharOp(char c)
{
  switch(c)
  {
    case '+':
      return TokenType::Plus;
    case '-':
      return TokenType::Minus;
    case '*':
      return TokenType::Star;
    case '/':
      return TokenType::Slash;
    case '&':
      return TokenType::Ampersand;
    case '|':
      return TokenType::Pipe;
    case '^':
      return TokenType::Caret;
    case '~':
      return TokenType::Tilde;
    case '!':
      return TokenType::Bang;
    case '<':
      return TokenType::Less;
    case '>':
      return TokenType::Greater;
    case '=':
      return TokenType::Equal;
    case '?':
      return TokenType::Question;
    case ':':
      return TokenType::Colon;
    case '(':
      return TokenType::LeftParen;
    case ')':
      return TokenType::RightParen;
    case ';':
      return TokenType::Semicolon;
    default:
      return TokenType::Error;
  }
}

}  // namespace

std::vector<Token> tokenize(const std::string& source)
{
  std::vector<Token> tokens;
  const size_t len = source.size();
  size_t i = 0;

  while(i < len)
  {
    const char c = source[i];

    // Skip whitespace (space, tab, newline, carriage return)
    if(c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
      ++i;
      continue;
    }

    const size_t start = i;

    // Single-quoted string literal
    if(c == '\'')
    {
      ++i;
      while(i < len && source[i] != '\'')
      {
        ++i;
      }
      if(i < len)
      {
        // extract content without quotes
        std::string_view text(&source[start + 1], i - start - 1);
        tokens.push_back({ TokenType::String, text, start });
        ++i;  // skip closing quote
      }
      else
      {
        std::string_view text(&source[start], i - start);
        tokens.push_back({ TokenType::Error, text, start });
      }
      continue;
    }

    // Number literal (integer or real)
    if(isDigit(c))
    {
      NumberResult nr;
      const bool is_hex =
          c == '0' && i + 1 < len && (source[i + 1] == 'x' || source[i + 1] == 'X');
      if(is_hex)
      {
        nr = scanHexNumber(source, len, i);
      }
      else
      {
        nr = scanDecimalNumber(source, len, i);
      }

      std::string_view text(&source[start], i - start);
      if(nr.has_error)
      {
        tokens.push_back({ TokenType::Error, text, start });
      }
      else if(nr.is_real)
      {
        tokens.push_back({ TokenType::Real, text, start });
      }
      else
      {
        tokens.push_back({ TokenType::Integer, text, start });
      }
      continue;
    }

    // Identifier or keyword (true/false)
    if(isIdentStart(c))
    {
      ++i;  // consume start character (may not be isIdentChar, e.g. '@')
      while(i < len && isIdentChar(source[i]))
      {
        ++i;
      }
      if(std::string_view text(&source[start], i - start); text == "true" || text == "fal"
                                                                                     "se")
      {
        tokens.push_back({ TokenType::Boolean, text, start });
      }
      else
      {
        tokens.push_back({ TokenType::Identifier, text, start });
      }
      continue;
    }

    // Two-character operators (check before single-char)
    if(i + 1 < len)
    {
      TokenType two_char_type = matchTwoCharOp(c, source[i + 1]);
      if(two_char_type != TokenType::Error)
      {
        std::string_view text(&source[start], 2);
        tokens.push_back({ two_char_type, text, start });
        i += 2;
        continue;
      }
    }

    // Single-character operators and delimiters
    std::string_view text(&source[start], 1);
    tokens.push_back({ matchSingleCharOp(c), text, start });
    ++i;
  }

  // Sentinel
  tokens.push_back({ TokenType::EndOfInput, {}, i });
  return tokens;
}

}  // namespace BT::Scripting
