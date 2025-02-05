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
  char error_msgs_buffer[2048];

  auto input = lexy::string_input<lexy::utf8_encoding>(script);
  auto result =
      lexy::parse<BT::Grammar::stmt>(input, ErrorReport().to(error_msgs_buffer));
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

class SafeErrorReport
{
  mutable std::string error_buffer;
  mutable std::size_t count = 0;

  struct _sink
  {
    std::string* buffer;
    std::size_t* count;
    using return_type = std::size_t;

    template <typename Input, typename Reader, typename Tag>
    void operator()(const lexy::error_context<Input>& context,
                    const lexy::error<Reader, Tag>& error)
    {
      *buffer += "error: while parsing ";
      *buffer += context.production();
      *buffer += "\n";
      (*count)++;
    }

    std::size_t finish() &&
    {
      return *count;
    }
  };

public:
  using return_type = std::size_t;

  constexpr auto sink() const
  {
    return _sink{ &error_buffer, &count };
  }
  const std::string& get_errors() const
  {
    return error_buffer;
  }
};

Result ValidateScript(const std::string& script)
{
  auto input = lexy::string_input<lexy::utf8_encoding>(script);
  SafeErrorReport error_report;  // Replace char buffer with our safe handler

  auto result = lexy::parse<BT::Grammar::stmt>(input, error_report);

  if(!result.has_value() || result.error_count() != 0)
  {
    return nonstd::make_unexpected(error_report.get_errors());
  }

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

}  // namespace BT
