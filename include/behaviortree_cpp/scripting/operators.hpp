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

#include "behaviortree_cpp/scripting/any_types.hpp"
#include "behaviortree_cpp/scripting/script_parser.hpp"

#include <cmath>
#include <memory>
#include <string>
#include <vector>

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

  explicit ExprName(std::string n) : name(std::move(n))
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

  explicit ExprUnaryArithmetic(op_t op, expr_ptr e) : op(op), rhs(std::move(e))
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
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs))
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
    : condition(std::move(condition)), then(std::move(then)), else_(std::move(else_))
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
    : op(op), lhs(std::move(_lhs)), rhs(std::move(_rhs))
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

namespace BT::Scripting
{

/// Parse a script string into a list of AST expression nodes.
/// Throws std::runtime_error on parse failure.
std::vector<Ast::expr_ptr> parseStatements(const std::string& script);

}  // namespace BT::Scripting
