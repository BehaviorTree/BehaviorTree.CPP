/*  Copyright (C) 2023 Davide Faconti -  All Rights Reserved
 *
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

#include "behaviortree_cpp/condition_node.h"
#include "behaviortree_cpp/scripting/script_parser.hpp"

namespace BT
{
/**
 * @brief Execute a script, and if the result is true, return
 * SUCCESS, FAILURE otherwise.
 */
class ScriptCondition : public ConditionNode
{
public:
  ScriptCondition(const std::string& name, const NodeConfig& config)
    : ConditionNode(name, config)
  {
    setRegistrationID("ScriptCondition");
    loadExecutor();
  }

  static PortsList providedPorts()
  {
    return { InputPort("code", "Piece of code that can be parsed. Must return false or "
                               "true") };
  }

private:
  virtual BT::NodeStatus tick() override
  {
    loadExecutor();

    Ast::Environment env = { config().blackboard, config().enums };
    auto result = _executor(env);
    return (result.cast<bool>()) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
  }

  void loadExecutor()
  {
    std::string script;
    if(!getInput("code", script))
    {
      throw RuntimeError("Missing port [code] in ScriptCondition");
    }
    if(script == _script)
    {
      return;
    }
    auto executor = ParseScript(script);
    if(!executor)
    {
      throw RuntimeError(executor.error());
    }
    else
    {
      _executor = executor.value();
      _script = script;
    }
  }

  std::string _script;
  ScriptFunction _executor;
};

}  // namespace BT
