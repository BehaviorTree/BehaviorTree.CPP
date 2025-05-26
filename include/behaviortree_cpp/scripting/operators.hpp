/*  Copyright (C) 2022 Davide Faconti -  All Rights Reserved
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

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "behaviortree_cpp/scripting/any_types.hpp"
#include "behaviortree_cpp/scripting/script_parser.hpp"

// Naive implementation of an AST with simple evaluation function.
namespace BT::Ast
{
using SimpleString = SafeAny::SimpleString;

using expr_ptr = std::shared_ptr<struct ExprBase>;

// extended string to number that consider enums and booleans
inline double StringToDouble(const Any& value, const Environment& env)
{
  const auto str = value.cast<std::string>();
  if(str == "true")
  {
    return 1.0;
  }
  if(str == "false")
  {
    return 0.0;
  }
  if(env.enums)
  {
    auto it = env.enums->find(str);
    if(it != env.enums->end())
    {
      return it->second;
    }
  }
  return value.cast<double>();
}

struct ExprBase
{
  using Ptr = std::shared_ptr<ExprBase>;

  virtual ~ExprBase() = default;
  virtual Any evaluate(Environment& env) const = 0;
};

inline std::string ErrorNotInit(const char* side, const char* op_str)
{
  return StrCat("The ", side, " operand of the operator [", op_str,
                "] is not initialized");
}

struct ExprLiteral : ExprBase
{
  Any value;

  ExprLiteral(Any v) : value(v)
  {}

  Any evaluate(Environment&) const override
  {
    return value;
  }
};

struct ExprName : ExprBase
{
  std::string name;

  explicit ExprName(std::string n) : name(LEXY_MOV(n))
  {}

  Any evaluate(Environment& env) const override
  {
    //search first in the enums table
    if(env.enums)
    {
      auto enum_ptr = env.enums->find(name);
      if(enum_ptr != env.enums->end())
      {
        return Any(double(enum_ptr->second));
      }
    }
    // search now in the variables table
    auto any_ref = env.vars->getAnyLocked(name);
    if(!any_ref)
    {
      throw RuntimeError(StrCat("Variable not found: ", name));
    }
    return *any_ref.get();
  }
};

struct ExprUnaryArithmetic : ExprBase
{
  enum op_t
  {
    negate,
    complement,
    logical_not
  } op;
  expr_ptr rhs;

  explicit ExprUnaryArithmetic(op_t op, expr_ptr e) : op(op), rhs(LEXY_MOV(e))
  {}

  Any evaluate(Environment& env) const override
  {
    auto rhs_v = rhs->evaluate(env);
    if(rhs_v.isNumber())
    {
      const double rv = rhs_v.cast<double>();
      switch(op)
      {
        case negate:
          return Any(-rv);
        case complement:
          if(rv > static_cast<double>(std::numeric_limits<int64_t>::max()) ||
             rv < static_cast<double>(std::numeric_limits<int64_t>::min()))
          {
            throw RuntimeError("Number out of range for bitwise operation");
          }
          return Any(static_cast<double>(~static_cast<int64_t>(rv)));
        case logical_not:
          return Any(static_cast<double>(!static_cast<bool>(rv)));
      }
    }
    else if(rhs_v.isString())
    {
      throw RuntimeError("Invalid operator for std::string");
    }
    throw RuntimeError("ExprUnaryArithmetic: undefined");
  }
};

struct ExprBinaryArithmetic : ExprBase
{
  enum op_t
  {
    plus,
    minus,
    times,
    div,
    concat,

    bit_and,
    bit_or,
    bit_xor,

    logic_and,
    logic_or
  } op;

  const char* opStr() const
  {
    switch(op)
    {
      case plus:
        return "+";
      case minus:
        return "-";
      case times:
        return "*";
      case div:
        return "/";
      case concat:
        return "..";
      case bit_and:
        return "&";
      case bit_or:
        return "|";
      case bit_xor:
        return "^";
      case logic_and:
        return "&&";
      case logic_or:
        return "||";
    }
    return "";
  }

  expr_ptr lhs, rhs;

  explicit ExprBinaryArithmetic(expr_ptr lhs, op_t op, expr_ptr rhs)
    : op(op), lhs(LEXY_MOV(lhs)), rhs(LEXY_MOV(rhs))
  {}

  Any evaluate(Environment& env) const override
  {
    auto lhs_v = lhs->evaluate(env);
    auto rhs_v = rhs->evaluate(env);

    if(lhs_v.empty())
    {
      throw RuntimeError(ErrorNotInit("left", opStr()));
    }
    if(rhs_v.empty())
    {
      throw RuntimeError(ErrorNotInit("right", opStr()));
    }

    if(rhs_v.isNumber() && lhs_v.isNumber())
    {
      auto lv = lhs_v.cast<double>();
      auto rv = rhs_v.cast<double>();

      switch(op)
      {
        case plus:
          return Any(lv + rv);
        case minus:
          return Any(lv - rv);
        case times:
          return Any(lv * rv);
        case div:
          return Any(lv / rv);
        default: {
        }
      }

      if(op == bit_and || op == bit_or || op == bit_xor)
      {
        try
        {
          int64_t li = lhs_v.cast<int64_t>();
          int64_t ri = rhs_v.cast<int64_t>();
          switch(op)
          {
            case bit_and:
              return Any(static_cast<double>(li & ri));
            case bit_or:
              return Any(static_cast<double>(li | ri));
            case bit_xor:
              return Any(static_cast<double>(li ^ ri));
            default: {
            }
          }
        }
        catch(...)
        {
          throw RuntimeError("Binary operators are not allowed if "
                             "one of the operands is not an integer");
        }
      }

      if(op == logic_or || op == logic_and)
      {
        try
        {
          auto lb = lhs_v.cast<bool>();
          auto rb = rhs_v.cast<bool>();
          switch(op)
          {
            case logic_or:
              return Any(static_cast<double>(lb || rb));
            case logic_and:
              return Any(static_cast<double>(lb && rb));
            default: {
            }
          }
        }
        catch(...)
        {
          throw RuntimeError("Logic operators are not allowed if "
                             "one of the operands is not castable to bool");
        }
      }
    }
    else if(rhs_v.isString() && lhs_v.isString() && op == plus)
    {
      return Any(lhs_v.cast<std::string>() + rhs_v.cast<std::string>());
    }
    else if(op == concat && ((rhs_v.isString() && lhs_v.isString()) ||
                             (rhs_v.isString() && lhs_v.isNumber()) ||
                             (rhs_v.isNumber() && lhs_v.isString())))
    {
      return Any(lhs_v.cast<std::string>() + rhs_v.cast<std::string>());
    }
    else
    {
      throw RuntimeError("Operation not permitted");
    }

    return {};  // unreachable
  }
};

template <typename T>
bool IsSame(const T& lv, const T& rv)
{
  if constexpr(std::is_same_v<double, T>)
  {
    constexpr double EPS = static_cast<double>(std::numeric_limits<float>::epsilon());
    return std::abs(lv - rv) <= EPS;
  }
  else
  {
    return (lv == rv);
  }
}

struct ExprComparison : ExprBase
{
  enum op_t
  {
    equal,
    not_equal,
    less,
    greater,
    less_equal,
    greater_equal
  };

  const char* opStr(op_t op) const
  {
    switch(op)
    {
      case equal:
        return "==";
      case not_equal:
        return "!=";
      case less:
        return "<";
      case greater:
        return ">";
      case less_equal:
        return "<=";
      case greater_equal:
        return ">=";
    }
    return "";
  }

  std::vector<op_t> ops;
  std::vector<expr_ptr> operands;

  Any evaluate(Environment& env) const override
  {
    auto SwitchImpl = [&](const auto& lv, const auto& rv, op_t op) {
      switch(op)
      {
        case equal:
          if(!IsSame(lv, rv))
            return false;
          break;
        case not_equal:
          if(IsSame(lv, rv))
            return false;
          break;
        case less:
          if(lv >= rv)
            return false;
          break;
        case greater:
          if(lv <= rv)
            return false;
          break;
        case less_equal:
          if(lv > rv)
            return false;
          break;
        case greater_equal:
          if(lv < rv)
            return false;
          break;
      }
      return true;
    };

    auto lhs_v = operands[0]->evaluate(env);
    for(auto i = 0u; i != ops.size(); ++i)
    {
      auto rhs_v = operands[i + 1]->evaluate(env);

      if(lhs_v.empty())
      {
        throw RuntimeError(ErrorNotInit("left", opStr(ops[i])));
      }
      if(rhs_v.empty())
      {
        throw RuntimeError(ErrorNotInit("right", opStr(ops[i])));
      }
      const Any False(0.0);

      if(lhs_v.isNumber() && rhs_v.isNumber())
      {
        auto lv = lhs_v.cast<double>();
        auto rv = rhs_v.cast<double>();
        if(!SwitchImpl(lv, rv, ops[i]))
        {
          return False;
        }
      }
      else if(lhs_v.isString() && rhs_v.isString())
      {
        auto lv = lhs_v.cast<SimpleString>();
        auto rv = rhs_v.cast<SimpleString>();
        if(!SwitchImpl(lv, rv, ops[i]))
        {
          return False;
        }
      }
      else if(lhs_v.isString() && rhs_v.isNumber())
      {
        auto lv = StringToDouble(lhs_v, env);
        auto rv = rhs_v.cast<double>();
        if(!SwitchImpl(lv, rv, ops[i]))
        {
          return False;
        }
      }
      else if(lhs_v.isNumber() && rhs_v.isString())
      {
        auto lv = lhs_v.cast<double>();
        auto rv = StringToDouble(rhs_v, env);
        if(!SwitchImpl(lv, rv, ops[i]))
        {
          return False;
        }
      }
      else
      {
        throw RuntimeError(StrCat("Can't mix different types in Comparison. "
                                  "Left operand [",
                                  BT::demangle(lhs_v.type()), "] right operand [",
                                  BT::demangle(rhs_v.type()), "]"));
      }
      lhs_v = rhs_v;
    }
    return Any(1.0);
  }
};

struct ExprIf : ExprBase
{
  expr_ptr condition, then, else_;

  explicit ExprIf(expr_ptr condition, expr_ptr then, expr_ptr else_)
    : condition(LEXY_MOV(condition)), then(LEXY_MOV(then)), else_(LEXY_MOV(else_))
  {}

  Any evaluate(Environment& env) const override
  {
    const auto& v = condition->evaluate(env);
    bool valid = (v.isType<SimpleString>() && v.cast<SimpleString>().size() > 0) ||
                 (v.cast<double>() != 0.0);
    if(valid)
    {
      return then->evaluate(env);
    }
    else
    {
      return else_->evaluate(env);
    }
  }
};

struct ExprAssignment : ExprBase
{
  enum op_t
  {
    assign_create,
    assign_existing,
    assign_plus,
    assign_minus,
    assign_times,
    assign_div
  } op;

  const char* opStr() const
  {
    switch(op)
    {
      case assign_create:
        return ":=";
      case assign_existing:
        return "=";
      case assign_plus:
        return "+=";
      case assign_minus:
        return "-=";
      case assign_times:
        return "*=";
      case assign_div:
        return "/=";
    }
    return "";
  }

  expr_ptr lhs, rhs;

  explicit ExprAssignment(expr_ptr _lhs, op_t op, expr_ptr _rhs)
    : op(op), lhs(LEXY_MOV(_lhs)), rhs(LEXY_MOV(_rhs))
  {}

  Any evaluate(Environment& env) const override
  {
    auto varname = dynamic_cast<ExprName*>(lhs.get());
    if(!varname)
    {
      throw RuntimeError("Assignment left operand not a blackboard entry");
    }
    const auto& key = varname->name;

    auto entry = env.vars->getEntry(key);
    if(!entry)
    {
      // variable doesn't exist, create it if using operator assign_create
      if(op == assign_create)
      {
        env.vars->createEntry(key, PortInfo());
        entry = env.vars->getEntry(key);
        if(!entry)
        {
          throw LogicError("Bug: report");
        }
      }
      else
      {
        // fail otherwise
        auto msg = StrCat("The blackboard entry [", key,
                          "] doesn't exist, yet.\n"
                          "If you want to create a new one, "
                          "use the operator "
                          "[:=] instead of [=]");
        throw RuntimeError(msg);
      }
    }
    auto value = rhs->evaluate(env);

    std::scoped_lock lock(entry->entry_mutex);
    auto* dst_ptr = &entry->value;

    auto errorPrefix = [dst_ptr, &key]() {
      return StrCat("Error assigning a value to entry [", key, "] with type [",
                    BT::demangle(dst_ptr->type()), "]. ");
    };

    if(value.empty())
    {
      throw RuntimeError(ErrorNotInit("right", opStr()));
    }

    if(op == assign_create || op == assign_existing)
    {
      // the very fist assignment can come from any type.
      // In the future, type check will be done by Any::copyInto
      if(dst_ptr->empty() && entry->info.type() == typeid(AnyTypeAllowed))
      {
        *dst_ptr = value;
      }
      else if(value.isString() && !dst_ptr->isString())
      {
        // special case: string to other type.
        // Check if we can use the StringConverter
        auto const str = value.cast<std::string>();
        const auto* entry_info = env.vars->entryInfo(key);

        if(auto converter = entry_info->converter())
        {
          *dst_ptr = converter(str);
        }
        else if(dst_ptr->isNumber())
        {
          auto num_value = StringToDouble(value, env);
          *dst_ptr = Any(num_value);
        }
        else
        {
          auto msg = StrCat(errorPrefix(),
                            "\nThe right operand is a string, "
                            "can't convert to ",
                            demangle(dst_ptr->type()));
          throw RuntimeError(msg);
        }
      }
      else
      {
        try
        {
          value.copyInto(*dst_ptr);
        }
        catch(std::exception&)
        {
          auto msg = StrCat(errorPrefix(), "\nThe right operand has type [",
                            BT::demangle(value.type()), "] and can't be converted to [",
                            BT::demangle(dst_ptr->type()), "]");
          throw RuntimeError(msg);
        }
      }
      entry->sequence_id++;
      entry->stamp = std::chrono::steady_clock::now().time_since_epoch();
      return *dst_ptr;
    }

    if(dst_ptr->empty())
    {
      throw RuntimeError(ErrorNotInit("left", opStr()));
    }

    // temporary use
    Any temp_variable = *dst_ptr;

    if(value.isNumber())
    {
      if(!temp_variable.isNumber())
      {
        throw RuntimeError("This Assignment operator can't be used "
                           "with a non-numeric type");
      }

      auto lv = temp_variable.cast<double>();
      auto rv = value.cast<double>();
      switch(op)
      {
        case assign_plus:
          temp_variable = Any(lv + rv);
          break;
        case assign_minus:
          temp_variable = Any(lv - rv);
          break;
        case assign_times:
          temp_variable = Any(lv * rv);
          break;
        case assign_div:
          temp_variable = Any(lv / rv);
          break;
        default: {
        }
      }
    }
    else if(value.isString())
    {
      if(op == assign_plus)
      {
        auto lv = temp_variable.cast<std::string>();
        auto rv = value.cast<std::string>();
        temp_variable = Any(lv + rv);
      }
      else
      {
        throw RuntimeError("Operator not supported for strings");
      }
    }

    temp_variable.copyInto(*dst_ptr);
    entry->sequence_id++;
    entry->stamp = std::chrono::steady_clock::now().time_since_epoch();
    return *dst_ptr;
  }
};
}  // namespace BT::Ast

namespace BT::Grammar
{
namespace dsl = lexy::dsl;

constexpr auto escaped_newline = dsl::backslash >> dsl::newline;

// An expression that is nested inside another expression.
struct nested_expr : lexy::transparent_production
{
  // We change the whitespace rule to allow newlines:
  // as it's nested, the REPL can properly handle continuation lines.
  static constexpr auto whitespace = dsl::ascii::space | escaped_newline;
  // The rule itself just recurses back to expression, but with the adjusted whitespace now.
  static constexpr auto rule = dsl::recurse<struct Expression>;

  static constexpr auto value = lexy::forward<Ast::expr_ptr>;
};

// An arbitrary expression.
// It uses lexy's built-in support for operator precedence parsing to automatically generate a
// proper rule. This is done by inheriting from expression_production.
struct Expression : lexy::expression_production
{
  struct expected_operand
  {
    static constexpr auto name = "expected operand";
  };

  // We need to specify the atomic part of an expression.
  static constexpr auto atom = [] {
    auto paren_expr = dsl::parenthesized(dsl::p<nested_expr>);
    auto boolean = dsl::p<BooleanLiteral>;
    auto var = dsl::p<Name>;
    auto literal = dsl::p<AnyValue>;

    return paren_expr | boolean | var | literal | dsl::error<expected_operand>;
  }();

  // Each of the nested classes defines one operation.
  // They inherit from a tag type that specify the kind of operation (prefix, infix, postfix),
  // and associativity (left, right, single (non-associative)),
  // and specify the operator rule and operand.

  // -x
  struct math_prefix : dsl::prefix_op
  {
    static constexpr auto op = dsl::op<Ast::ExprUnaryArithmetic::negate>(LEXY_LIT("-"));
    using operand = dsl::atom;
  };
  // x * x, x / x
  struct math_product : dsl::infix_op_left
  {
    static constexpr auto op = [] {
      // Don't confuse with *= or /=
      auto times = dsl::not_followed_by(LEXY_LIT("*"), dsl::lit_c<'='>);
      auto div = dsl::not_followed_by(LEXY_LIT("/"), dsl::lit_c<'='>);
      return dsl::op<Ast::ExprBinaryArithmetic::times>(times) /
             dsl::op<Ast::ExprBinaryArithmetic::div>(div);
    }();
    using operand = math_prefix;
  };
  // x + x, x - x
  struct math_sum : dsl::infix_op_left
  {
    static constexpr auto op = [] {
      // Don't confuse with += or -=
      auto plus = dsl::not_followed_by(LEXY_LIT("+"), dsl::lit_c<'='>);
      auto minus = dsl::not_followed_by(LEXY_LIT("-"), dsl::lit_c<'='>);
      return dsl::op<Ast::ExprBinaryArithmetic::plus>(plus) /
             dsl::op<Ast::ExprBinaryArithmetic::minus>(minus);
    }();

    using operand = math_product;
  };

  // x .. y
  struct string_concat : dsl::infix_op_left
  {
    static constexpr auto op = [] {
      return dsl::op<Ast::ExprBinaryArithmetic::concat>(LEXY_LIT(".."));
    }();

    using operand = math_sum;
  };

  // ~x
  struct bit_prefix : dsl::prefix_op
  {
    static constexpr auto op = [] {
      auto complement = LEXY_LIT("~");
      auto logical_not = dsl::not_followed_by(LEXY_LIT("!"), dsl::lit_c<'='>);

      return dsl::op<Ast::ExprUnaryArithmetic::complement>(complement) /
             dsl::op<Ast::ExprUnaryArithmetic::logical_not>(logical_not);
    }();
    using operand = dsl::atom;
  };

  // x & x
  struct bit_and : dsl::infix_op_left
  {
    static constexpr auto op = [] {
      // Don't confuse with &&
      auto bit_and = dsl::not_followed_by(LEXY_LIT("&"), dsl::lit_c<'&'>);
      return dsl::op<Ast::ExprBinaryArithmetic::bit_and>(bit_and);
    }();

    using operand = bit_prefix;
  };

  // x | x, x ^ x
  struct bit_or : dsl::infix_op_left
  {
    static constexpr auto op = [] {
      // Don't confuse with ||
      auto bit_or = dsl::not_followed_by(LEXY_LIT("|"), dsl::lit_c<'|'>);
      return dsl::op<Ast::ExprBinaryArithmetic::bit_or>(bit_or) /
             dsl::op<Ast::ExprBinaryArithmetic::bit_xor>(LEXY_LIT("^"));
    }();

    using operand = bit_and;
  };

  // Comparisons are list operators, which allows implementation of chaining.
  // x == y < z
  struct comparison : dsl::infix_op_list
  {
    // Other comparison operators omitted for simplicity.
    static constexpr auto op = dsl::op<Ast::ExprComparison::equal>(LEXY_LIT("==")) /
                               dsl::op<Ast::ExprComparison::not_equal>(LEXY_LIT("!=")) /
                               dsl::op<Ast::ExprComparison::less>(LEXY_LIT("<")) /
                               dsl::op<Ast::ExprComparison::greater>(LEXY_LIT(">")) /
                               dsl::op<Ast::ExprComparison::less_equal>(LEXY_LIT("<=")) /
                               dsl::op<Ast::ExprComparison::greater_equal>(LEXY_LIT(">"
                                                                                    "="));

    // The use of dsl::groups ensures that an expression can either contain math or bit or string
    // operators. Mixing requires parenthesis.
    using operand = dsl::groups<math_sum, bit_or, string_concat>;
  };

  // Logical operators,  || and &&
  struct logical : dsl::infix_op_left
  {
    static constexpr auto op =
        dsl::op<Ast::ExprBinaryArithmetic::logic_or>(LEXY_LIT("||")) /
        dsl::op<Ast::ExprBinaryArithmetic::logic_and>(LEXY_LIT("&&"));

    using operand = comparison;
  };

  // x ? y : z
  struct conditional : dsl::infix_op_single
  {
    // We treat a conditional operator, which has three operands,
    // as a binary operator where the operator consists of ?, the inner operator, and :.
    // The <void> ensures that `dsl::op` does not produce a value.
    static constexpr auto op =
        dsl::op<void>(LEXY_LIT("?") >> (dsl::p<nested_expr> + dsl::lit_c<':'>));
    using operand = logical;
  };

  struct assignment : dsl::infix_op_single
  {
    // We need to prevent `=` from matching `==`.
    static constexpr auto op =
        dsl::op<Ast::ExprAssignment::assign_create>(LEXY_LIT(":=")) /
        dsl::op<Ast::ExprAssignment::assign_existing>(
            dsl::not_followed_by(LEXY_LIT("="), dsl::lit_c<'='>)) /
        dsl::op<Ast::ExprAssignment::assign_plus>(LEXY_LIT("+=")) /
        dsl::op<Ast::ExprAssignment::assign_minus>(LEXY_LIT("-=")) /
        dsl::op<Ast::ExprAssignment::assign_times>(LEXY_LIT("*=")) /
        dsl::op<Ast::ExprAssignment::assign_div>(LEXY_LIT("/="));

    using operand = conditional;
  };

  // An expression also needs to specify the operation with the lowest binding power.
  // The operation of everything else is determined by following the `::operand` member.
  using operation = assignment;

  static constexpr auto value =
      // We need a sink as the comparison expression generates a list.
      lexy::fold_inplace<std::unique_ptr<Ast::ExprComparison>>(
          [] { return std::make_unique<Ast::ExprComparison>(); },
          [](auto& node, Ast::expr_ptr opr) { node->operands.push_back(LEXY_MOV(opr)); },
          [](auto& node, Ast::ExprComparison::op_t op) { node->ops.push_back(op); })
      // The result of the list feeds into a callback that handles all other cases.
      >> lexy::callback(
             // atoms
             lexy::forward<Ast::expr_ptr>, lexy::new_<Ast::ExprLiteral, Ast::expr_ptr>,
             lexy::new_<Ast::ExprName, Ast::expr_ptr>,
             // unary/binary operators
             lexy::new_<Ast::ExprUnaryArithmetic, Ast::expr_ptr>,
             lexy::new_<Ast::ExprBinaryArithmetic, Ast::expr_ptr>,
             // conditional and assignment
             lexy::new_<Ast::ExprIf, Ast::expr_ptr>,
             lexy::new_<Ast::ExprAssignment, Ast::expr_ptr>);
};

// A statement, which is a list of expressions separated by semicolons.
struct stmt
{
  // We don't allow newlines as whitespace at the top-level.
  // This is because we can't easily know whether we need to request more input when seeing a
  // newline or not. Once we're having a e.g. parenthesized expression, we know that we need more
  // input until we've reached ), so then change the whitespace rule.
  static constexpr auto whitespace = dsl::ascii::blank | escaped_newline | dsl::newline;

  static constexpr auto rule = [] {
    // We can't use `dsl::eol` as our terminator directly,
    // since that would try and skip whitespace, which requests more input on the REPL.
    auto at_eol = dsl::peek(dsl::eol);
    return dsl::terminator(at_eol).opt_list(dsl::p<Expression>, dsl::sep(dsl::semicolon));
  }();

  static constexpr auto value = lexy::as_list<std::vector<Ast::expr_ptr>>;
};

}  // namespace BT::Grammar
