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

#include "behaviortree_cpp/scripting/script_parser.hpp"

#include "behaviortree_cpp/scripting/operators.hpp"

#include <charconv>

namespace BT
{

namespace Scripting
{

class ScriptParser
{
public:
  explicit ScriptParser(std::vector<Token> tokens) : tokens_(std::move(tokens))
  {}

  std::vector<Ast::expr_ptr> parseAll()
  {
    std::vector<Ast::expr_ptr> stmts;
    while(!atEnd())
    {
      stmts.push_back(parseExpr(0));
      // consume optional semicolons between statements
      while(check(TokenType::Semicolon))
      {
        advance();
      }
    }
    return stmts;
  }

private:
  std::vector<Token> tokens_;
  size_t current_ = 0;

  // Binding power constants.  Higher value = tighter binding.
  static constexpr int kAssignmentBP = 2;
  static constexpr int kTernaryBP = 4;
  static constexpr int kComparisonBP = 10;
  static constexpr int kMulDivBP = 18;
  static constexpr int kPrefixBP = 20;  // tighter than any infix

  //--- Token access ---

  const Token& peek() const
  {
    return tokens_[current_];
  }

  const Token& advance()
  {
    const Token& tok = tokens_[current_];
    if(!atEnd())
      current_++;
    return tok;
  }

  bool atEnd() const
  {
    return peek().type == TokenType::EndOfInput;
  }

  bool check(TokenType type) const
  {
    return peek().type == type;
  }

  const Token& expect(TokenType type, const char* msg)
  {
    if(!check(type))
    {
      throw RuntimeError(StrCat("Parse error at position ", std::to_string(peek().pos),
                                ": ", msg, " (got '", peek().text, "')"));
    }
    return advance();
  }

  //--- Binding power helpers ---

  // Returns the left binding power of an infix/postfix operator,
  // or -1 if the token is not an infix operator.
  static int leftBP(TokenType type)
  {
    switch(type)
    {
      // Assignment (level 1, non-associative)
      case TokenType::ColonEqual:
      case TokenType::Equal:
      case TokenType::PlusEqual:
      case TokenType::MinusEqual:
      case TokenType::StarEqual:
      case TokenType::SlashEqual:
        return kAssignmentBP;
      // Ternary (level 2, non-associative)
      case TokenType::Question:
        return kTernaryBP;
      // Logical OR (level 3)
      case TokenType::PipePipe:
        return 6;
      // Logical AND (level 4)
      case TokenType::AmpAmp:
        return 8;
      // Comparison (level 5, chainable)
      case TokenType::EqualEqual:
      case TokenType::BangEqual:
      case TokenType::Less:
      case TokenType::Greater:
      case TokenType::LessEqual:
      case TokenType::GreaterEqual:
        return kComparisonBP;
      // Bitwise OR/XOR (level 6)
      case TokenType::Pipe:
      case TokenType::Caret:
        return 12;
      // Bitwise AND (level 7)
      case TokenType::Ampersand:
        return 14;
      // Add/Sub/Concat (level 8)
      case TokenType::Plus:
      case TokenType::Minus:
      case TokenType::DotDot:
        return 16;
      // Mul/Div (level 9)
      case TokenType::Star:
      case TokenType::Slash:
        return kMulDivBP;
      default:
        return -1;
    }
  }

  static bool isComparison(TokenType type)
  {
    return type == TokenType::EqualEqual || type == TokenType::BangEqual ||
           type == TokenType::Less || type == TokenType::Greater ||
           type == TokenType::LessEqual || type == TokenType::GreaterEqual;
  }

  static bool isAssignment(TokenType type)
  {
    return type == TokenType::ColonEqual || type == TokenType::Equal ||
           type == TokenType::PlusEqual || type == TokenType::MinusEqual ||
           type == TokenType::StarEqual || type == TokenType::SlashEqual;
  }

  //--- Parsing ---

  /// Prefix: atoms and unary prefix operators
  Ast::expr_ptr parsePrefix()
  {
    const auto& tok = peek();

    // Unary minus
    if(tok.type == TokenType::Minus)
    {
      advance();
      auto operand = parseExpr(kPrefixBP);
      return std::make_shared<Ast::ExprUnaryArithmetic>(Ast::ExprUnaryArithmetic::negate,
                                                        std::move(operand));
    }
    // Bitwise complement
    if(tok.type == TokenType::Tilde)
    {
      advance();
      auto operand = parseExpr(kPrefixBP);
      return std::make_shared<Ast::ExprUnaryArithmetic>(
          Ast::ExprUnaryArithmetic::complement, std::move(operand));
    }
    // Logical NOT
    if(tok.type == TokenType::Bang)
    {
      advance();
      auto operand = parseExpr(kPrefixBP);
      return std::make_shared<Ast::ExprUnaryArithmetic>(
          Ast::ExprUnaryArithmetic::logical_not, std::move(operand));
    }
    // Parenthesized expression
    if(tok.type == TokenType::LeftParen)
    {
      advance();
      auto expr = parseExpr(0);
      expect(TokenType::RightParen, "expected ')'");
      return expr;
    }
    // Boolean literal
    if(tok.type == TokenType::Boolean)
    {
      advance();
      double val = (tok.text == "true") ? 1.0 : 0.0;
      return std::make_shared<Ast::ExprLiteral>(Any(val));
    }
    // Integer literal
    if(tok.type == TokenType::Integer)
    {
      advance();
      int64_t val = 0;
      const char* first = tok.text.data();
      const char* last = first + tok.text.size();
      if(tok.text.size() > 2 && tok.text[0] == '0' &&
         (tok.text[1] == 'x' || tok.text[1] == 'X'))
      {
        std::from_chars(first + 2, last, val, 16);
      }
      else
      {
        std::from_chars(first, last, val, 10);
      }
      return std::make_shared<Ast::ExprLiteral>(Any(val));
    }
    // Real literal
    if(tok.type == TokenType::Real)
    {
      advance();
      double val = convertFromString<double>(tok.text);
      return std::make_shared<Ast::ExprLiteral>(Any(val));
    }
    // String literal
    if(tok.type == TokenType::String)
    {
      advance();
      return std::make_shared<Ast::ExprLiteral>(Any(std::string(tok.text)));
    }
    // Identifier
    if(tok.type == TokenType::Identifier)
    {
      advance();
      return std::make_shared<Ast::ExprName>(std::string(tok.text));
    }
    // Error token from tokenizer
    if(tok.type == TokenType::Error)
    {
      throw RuntimeError(
          StrCat("Invalid token '", tok.text, "' at position ", std::to_string(tok.pos)));
    }

    throw RuntimeError(StrCat("Expected operand at position ", std::to_string(tok.pos),
                              " (got '", tok.text, "')"));
  }

  /// Main Pratt expression parser
  Ast::expr_ptr parseExpr(int minBP)
  {
    auto left = parsePrefix();

    while(true)
    {
      auto tokType = peek().type;
      int lbp = leftBP(tokType);
      if(lbp < 0 || lbp < minBP)
      {
        break;
      }

      // Assignment (non-associative: parse once, then break)
      if(isAssignment(tokType))
      {
        left = parseAssignment(std::move(left));
        break;
      }

      // Ternary (non-associative: parse once, then break)
      if(tokType == TokenType::Question)
      {
        left = parseTernary(std::move(left));
        break;
      }

      // Chained comparison
      if(isComparison(tokType))
      {
        left = parseChainedComparison(std::move(left));
        continue;
      }

      // Regular left-associative binary operator
      const auto& opTok = advance();
      // Right BP = LBP + 1 for left-associativity
      auto right = parseExpr(lbp + 1);
      left = makeBinary(std::move(left), opTok.type, std::move(right));
    }

    return left;
  }

  Ast::expr_ptr parseAssignment(Ast::expr_ptr left)
  {
    const auto& opTok = advance();
    Ast::ExprAssignment::op_t op{};
    switch(opTok.type)
    {
      case TokenType::ColonEqual:
        op = Ast::ExprAssignment::assign_create;
        break;
      case TokenType::Equal:
        op = Ast::ExprAssignment::assign_existing;
        break;
      case TokenType::PlusEqual:
        op = Ast::ExprAssignment::assign_plus;
        break;
      case TokenType::MinusEqual:
        op = Ast::ExprAssignment::assign_minus;
        break;
      case TokenType::StarEqual:
        op = Ast::ExprAssignment::assign_times;
        break;
      case TokenType::SlashEqual:
        op = Ast::ExprAssignment::assign_div;
        break;
      default:
        throw RuntimeError("Internal error: unexpected assignment op");
    }
    // Parse RHS -- use minBP=0 to allow full expression
    auto right = parseExpr(0);
    return std::make_shared<Ast::ExprAssignment>(std::move(left), op, std::move(right));
  }

  Ast::expr_ptr parseTernary(Ast::expr_ptr condition)
  {
    advance();                     // consume '?'
    auto thenExpr = parseExpr(0);  // full expression inside
    expect(TokenType::Colon, "expected ':' in ternary expression");
    auto elseExpr = parseExpr(kTernaryBP);
    return std::make_shared<Ast::ExprIf>(std::move(condition), std::move(thenExpr),
                                         std::move(elseExpr));
  }

  Ast::expr_ptr parseChainedComparison(Ast::expr_ptr first)
  {
    auto node = std::make_shared<Ast::ExprComparison>();
    node->operands.push_back(std::move(first));

    while(isComparison(peek().type))
    {
      node->ops.push_back(mapComparisonOp(advance().type));
      // Parse the next operand above comparison level
      // so that arithmetic binds tighter
      node->operands.push_back(parseExpr(kComparisonBP + 1));
    }
    return node;
  }

  static Ast::ExprComparison::op_t mapComparisonOp(TokenType type)
  {
    switch(type)
    {
      case TokenType::EqualEqual:
        return Ast::ExprComparison::equal;
      case TokenType::BangEqual:
        return Ast::ExprComparison::not_equal;
      case TokenType::Less:
        return Ast::ExprComparison::less;
      case TokenType::Greater:
        return Ast::ExprComparison::greater;
      case TokenType::LessEqual:
        return Ast::ExprComparison::less_equal;
      case TokenType::GreaterEqual:
        return Ast::ExprComparison::greater_equal;
      default:
        throw RuntimeError("Internal error: not a comparison op");
    }
  }

  static Ast::expr_ptr makeBinary(Ast::expr_ptr left, TokenType opType,
                                  Ast::expr_ptr right)
  {
    Ast::ExprBinaryArithmetic::op_t op{};
    switch(opType)
    {
      case TokenType::Plus:
        op = Ast::ExprBinaryArithmetic::plus;
        break;
      case TokenType::Minus:
        op = Ast::ExprBinaryArithmetic::minus;
        break;
      case TokenType::Star:
        op = Ast::ExprBinaryArithmetic::times;
        break;
      case TokenType::Slash:
        op = Ast::ExprBinaryArithmetic::div;
        break;
      case TokenType::DotDot:
        op = Ast::ExprBinaryArithmetic::concat;
        break;
      case TokenType::Ampersand:
        op = Ast::ExprBinaryArithmetic::bit_and;
        break;
      case TokenType::Pipe:
        op = Ast::ExprBinaryArithmetic::bit_or;
        break;
      case TokenType::Caret:
        op = Ast::ExprBinaryArithmetic::bit_xor;
        break;
      case TokenType::AmpAmp:
        op = Ast::ExprBinaryArithmetic::logic_and;
        break;
      case TokenType::PipePipe:
        op = Ast::ExprBinaryArithmetic::logic_or;
        break;
      default:
        throw RuntimeError("Internal error: unknown binary operator");
    }
    return std::make_shared<Ast::ExprBinaryArithmetic>(std::move(left), op,
                                                       std::move(right));
  }
};

std::vector<Ast::expr_ptr> parseStatements(const std::string& script)
{
  auto tokens = tokenize(script);
  ScriptParser parser(std::move(tokens));
  return parser.parseAll();
}

}  // namespace Scripting

//--- Public API ---

Expected<ScriptFunction> ParseScript(const std::string& script)
{
  try
  {
    auto exprs = Scripting::parseStatements(script);
    if(exprs.empty())
    {
      return nonstd::make_unexpected("Empty Script");
    }
    return [exprs = std::move(exprs), script](Ast::Environment& env) {
      try
      {
        for(size_t i = 0; i < exprs.size() - 1; ++i)
        {
          exprs[i]->evaluate(env);
        }
        return exprs.back()->evaluate(env);
      }
      catch(RuntimeError& err)
      {
        throw RuntimeError(StrCat("Error in script [", script, "]\n", err.what()));
      }
    };
  }
  catch(RuntimeError& err)
  {
    return nonstd::make_unexpected(err.what());
  }
}

Expected<Any> ParseScriptAndExecute(Ast::Environment& env, const std::string& script)
{
  auto executor = ParseScript(script);
  if(executor)
  {
    try
    {
      return executor.value()(env);
    }
    catch(RuntimeError& err)
    {
      return nonstd::make_unexpected(err.what());
    }
  }
  return nonstd::make_unexpected(executor.error());
}

Result ValidateScript(const std::string& script)
{
  try
  {
    auto exprs = Scripting::parseStatements(script);
    if(exprs.empty())
    {
      return nonstd::make_unexpected("Empty Script");
    }
    return {};
  }
  catch(RuntimeError& err)
  {
    return nonstd::make_unexpected(err.what());
  }
}

}  // namespace BT
