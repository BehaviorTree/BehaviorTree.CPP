#include "behaviortree_cpp_v3/scripting/script_parser.hpp"
#include "behaviortree_cpp_v3/scripting/operators.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/action/validate.hpp>
#include <lexy_ext/report_error.hpp>
#include <lexy/input/string_input.hpp>

namespace BT
{
Optional<ScriptFunction> ParseScript(const std::string& script)
{
  using ErrorReport = lexy_ext::_report_error<char*>;
  char error_msgs_buffer[2048];

  auto input = lexy::string_input<lexy::utf8_encoding>(script);
  auto result =
      lexy::parse<BT::Grammar::stmt>(input, ErrorReport().to(error_msgs_buffer));
  if (result.has_value() && result.error_count() == 0)
  {
    try
    {
      std::vector<BT::Ast::ExprBase::Ptr> exprs = LEXY_MOV(result).value();
      if (exprs.empty())
      {
        return nonstd::make_unexpected("Empty Script");
      }

      return [exprs](Ast::Environment& env) -> Any {
        for (auto i = 0u; i < exprs.size() - 1; ++i)
        {
          exprs[i]->evaluate(env);
        }
        return exprs.back()->evaluate(env);
      };
    }
    catch (std::runtime_error& err)
    {
      return nonstd::make_unexpected(err.what());
    }
  }
  else
  {
    return nonstd::make_unexpected(error_msgs_buffer);
  }
}

BT::Optional<Any> ParseScriptAndExecute(Ast::Environment& env, const std::string& script)
{
  auto executor = ParseScript(script);
  if (executor)
  {
    return executor.value()(env);
  }
  else   // forward the error
  {
    return nonstd::make_unexpected(executor.error());
  }
}

}   // namespace BT
