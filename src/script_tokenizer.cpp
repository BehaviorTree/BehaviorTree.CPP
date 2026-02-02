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
      bool is_real = false;
      bool has_error = false;

      // Check for hex prefix
      if(c == '0' && i + 1 < len && (source[i + 1] == 'x' || source[i + 1] == 'X'))
      {
        i += 2;  // skip "0x"/"0X"
        if(i >= len || !isHexDigit(source[i]))
        {
          has_error = true;
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
          has_error = true;
          while(i < len && (isIdentChar(source[i]) || source[i] == '.'))
          {
            ++i;
          }
        }
      }
      else
      {
        // Decimal integer part
        while(i < len && isDigit(source[i]))
        {
          ++i;
        }
        // Check for fractional part
        if(i < len && source[i] == '.')
        {
          // Distinguish from ".." (concat operator)
          if(i + 1 < len && source[i + 1] == '.')
          {
            // Stop here: "65.." is Integer("65") + DotDot
          }
          else if(i + 1 < len && isDigit(source[i + 1]))
          {
            is_real = true;
            ++i;  // consume '.'
            while(i < len && isDigit(source[i]))
            {
              ++i;
            }
          }
          else
          {
            // "65." or "65.x" -- incomplete real
            has_error = true;
            ++i;  // consume the dot
            while(i < len && (isIdentChar(source[i]) || source[i] == '.'))
            {
              ++i;
            }
          }
        }
        // Check for exponent (only for decimal numbers)
        if(!has_error && i < len && (source[i] == 'e' || source[i] == 'E'))
        {
          is_real = true;
          ++i;  // consume 'e'/'E'
          if(i < len && (source[i] == '+' || source[i] == '-'))
          {
            ++i;  // consume sign
          }
          if(i >= len || !isDigit(source[i]))
          {
            has_error = true;
          }
          else
          {
            while(i < len && isDigit(source[i]))
            {
              ++i;
            }
          }
        }
        // Check for trailing alpha (e.g. "3foo", "65.43foo")
        if(!has_error && i < len && isIdentStart(source[i]))
        {
          has_error = true;
          while(i < len && isIdentChar(source[i]))
          {
            ++i;
          }
        }
      }

      std::string_view text(&source[start], i - start);
      if(has_error)
      {
        tokens.push_back({ TokenType::Error, text, start });
      }
      else if(is_real)
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
      std::string_view text(&source[start], i - start);
      if(text == "true" || text == "false")
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
      const char next = source[i + 1];
      TokenType two_char_type = TokenType::Error;
      bool matched = true;

      if(c == '.' && next == '.')
      {
        two_char_type = TokenType::DotDot;
      }
      else if(c == '&' && next == '&')
      {
        two_char_type = TokenType::AmpAmp;
      }
      else if(c == '|' && next == '|')
      {
        two_char_type = TokenType::PipePipe;
      }
      else if(c == '=' && next == '=')
      {
        two_char_type = TokenType::EqualEqual;
      }
      else if(c == '!' && next == '=')
      {
        two_char_type = TokenType::BangEqual;
      }
      else if(c == '<' && next == '=')
      {
        two_char_type = TokenType::LessEqual;
      }
      else if(c == '>' && next == '=')
      {
        two_char_type = TokenType::GreaterEqual;
      }
      else if(c == ':' && next == '=')
      {
        two_char_type = TokenType::ColonEqual;
      }
      else if(c == '+' && next == '=')
      {
        two_char_type = TokenType::PlusEqual;
      }
      else if(c == '-' && next == '=')
      {
        two_char_type = TokenType::MinusEqual;
      }
      else if(c == '*' && next == '=')
      {
        two_char_type = TokenType::StarEqual;
      }
      else if(c == '/' && next == '=')
      {
        two_char_type = TokenType::SlashEqual;
      }
      else
      {
        matched = false;
      }

      if(matched)
      {
        std::string_view text(&source[start], 2);
        tokens.push_back({ two_char_type, text, start });
        i += 2;
        continue;
      }
    }

    // Single-character operators and delimiters
    {
      TokenType type = TokenType::Error;
      switch(c)
      {
        case '+':
          type = TokenType::Plus;
          break;
        case '-':
          type = TokenType::Minus;
          break;
        case '*':
          type = TokenType::Star;
          break;
        case '/':
          type = TokenType::Slash;
          break;
        case '&':
          type = TokenType::Ampersand;
          break;
        case '|':
          type = TokenType::Pipe;
          break;
        case '^':
          type = TokenType::Caret;
          break;
        case '~':
          type = TokenType::Tilde;
          break;
        case '!':
          type = TokenType::Bang;
          break;
        case '<':
          type = TokenType::Less;
          break;
        case '>':
          type = TokenType::Greater;
          break;
        case '=':
          type = TokenType::Equal;
          break;
        case '?':
          type = TokenType::Question;
          break;
        case ':':
          type = TokenType::Colon;
          break;
        case '(':
          type = TokenType::LeftParen;
          break;
        case ')':
          type = TokenType::RightParen;
          break;
        case ';':
          type = TokenType::Semicolon;
          break;
        default:
          break;
      }
      std::string_view text(&source[start], 1);
      tokens.push_back({ type, text, start });
      ++i;
    }
  }

  // Sentinel
  tokens.push_back({ TokenType::EndOfInput, {}, i });
  return tokens;
}

}  // namespace BT::Scripting
