#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/scripting/script_parser.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 4)
  {
    return 0;
  }

  FuzzedDataProvider fuzz_data(data, size);

  try
  {
    BT::Ast::Environment env;
    env.vars = BT::Blackboard::create();
    env.enums = std::make_shared<BT::EnumsTable>();

    // Add some test variables to the blackboard
    env.vars->set("test_int", 42);
    env.vars->set("test_double", 3.14);
    env.vars->set("test_bool", true);
    env.vars->set("test_string", std::string("test"));

    // Add some test enums
    (*env.enums)["RUNNING"] = 0;
    (*env.enums)["SUCCESS"] = 1;
    (*env.enums)["FAILURE"] = 2;

    std::string script = fuzz_data.ConsumeRandomLengthString();

    auto validation_result = BT::ValidateScript(script);

    if(!validation_result)
    {
      auto parsed_script = BT::ParseScript(script);
      if(parsed_script)
      {
        try
        {
          auto result = parsed_script.value()(env);

          if(result.isNumber())
          {
            volatile auto num = result.cast<double>();
          }

          env.vars->set("result", result);

          BT::Any read_back;
          (void)env.vars->get("result", read_back);
        }
        catch(const BT::RuntimeError&)  // NOLINT(bugprone-empty-catch)
        {
          // Intentionally empty - fuzzer expects exceptions
        }
      }
    }

    BT::ParseScriptAndExecute(env, script);
  }
  catch(const std::exception&)  // NOLINT(bugprone-empty-catch)
  {
    // Intentionally empty - fuzzer expects exceptions
  }

  return 0;
}
