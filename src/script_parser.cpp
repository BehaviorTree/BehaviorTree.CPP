#include "behaviortree_cpp/scripting/script_parser.hpp"
#include "behaviortree_cpp/scripting/operators.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/action/validate.hpp>
#include <lexy_ext/report_error.hpp>
#include <lexy/input/string_input.hpp>

namespace BT
{

using ErrorReport = lexy_ext::_report_error<char*>;

Expected<ScriptFunction> ParseScript(const std::string& script)
{
  std::string error_msgs_buffer;  // dynamically growing error buffer

  auto input = lexy::string_input<lexy::utf8_encoding>(script);

  auto reporter = ErrorReport().to(std::back_inserter(error_msgs_buffer));
  auto result = lexy::parse<BT::Grammar::stmt>(input, reporter);
  if(result.has_value() && result.error_count() == 0)
  {
    try
    {
      std::vector<BT::Ast::ExprBase::Ptr> exprs = LEXY_MOV(result).value();
      if(exprs.empty())
      {
        return nonstd::make_unexpected("Empty Script");
      }

      return [exprs, script](Ast::Environment& env) -> Any {
        try
        {
          for(auto i = 0u; i < exprs.size() - 1; ++i)
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
    catch(std::runtime_error& err)
    {
      return nonstd::make_unexpected(err.what());
    }
  }
  else
  {
    return nonstd::make_unexpected(error_msgs_buffer);
  }
}

BT::Expected<Any> ParseScriptAndExecute(Ast::Environment& env, const std::string& script)
{
  auto executor = ParseScript(script);
  if(executor)
  {
    return executor.value()(env);
  }
  else  // forward the error
  {
    return nonstd::make_unexpected(executor.error());
  }
}

Result ValidateScript(const std::string& script)
{
  std::string error_msgs_buffer;  // dynamically growing error buffer

  auto input = lexy::string_input<lexy::utf8_encoding>(script);

  auto reporter = ErrorReport().to(std::back_inserter(error_msgs_buffer));
  auto result = lexy::parse<BT::Grammar::stmt>(input, reporter);
  if(result.has_value() && result.error_count() == 0)
  {
    try
    {
      std::vector<BT::Ast::ExprBase::Ptr> exprs = LEXY_MOV(result).value();
      if(exprs.empty())
      {
        return nonstd::make_unexpected("Empty Script");
      }
      // valid script
      return {};
    }
    catch(std::runtime_error& err)
    {
      return nonstd::make_unexpected(err.what());
    }
  }
  return nonstd::make_unexpected(error_msgs_buffer);
}

}  // namespace BT
