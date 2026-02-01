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

#include "behaviortree_cpp/decorator_node.h"
#include "behaviortree_cpp/scripting/script_parser.hpp"

#include <type_traits>

namespace BT
{
/**
 * @brief The PreconditionNode evaluates a script condition before ticking its child.
 *
 * If the script in the "if" port returns true, the child is ticked.
 * If the script returns false, the node returns the status specified in the "else" port
 * (FAILURE by default).
 *
 * Once the child starts (returns RUNNING), subsequent ticks will continue
 * executing the child without re-evaluating the precondition until completion.
 *
 * Example usage:
 *
 *   <Precondition if="A > B && color != BLUE" else="FAILURE">
 *     <SomeAction/>
 *   </Precondition>
 */
class PreconditionNode : public DecoratorNode
{
public:
  PreconditionNode(const std::string& name, const NodeConfig& config)
    : DecoratorNode(name, config)
  {
    loadExecutor();
  }

  ~PreconditionNode() override = default;

  PreconditionNode(const PreconditionNode&) = delete;
  PreconditionNode& operator=(const PreconditionNode&) = delete;
  PreconditionNode(PreconditionNode&&) = delete;
  PreconditionNode& operator=(PreconditionNode&&) = delete;

  static PortsList providedPorts()
  {
    return { InputPort<std::string>("if"),
             InputPort<NodeStatus>("else", NodeStatus::FAILURE,
                                   "Return status if condition is "
                                   "false") };
  }

private:
  virtual BT::NodeStatus tick() override
  {
    loadExecutor();

    BT::NodeStatus else_return = NodeStatus::FAILURE;
    if(!getInput("else", else_return))
    {
      throw RuntimeError("Missing parameter [else] in Precondition");
    }

    // Only check the 'if' script if we haven't started ticking the children yet.
    Ast::Environment env = { config().blackboard, config().enums };
    bool tick_children =
        _children_running || (_children_running = _executor(env).cast<bool>());

    if(!tick_children)
    {
      return else_return;
    }

    auto const child_status = child_node_->executeTick();
    if(isStatusCompleted(child_status))
    {
      resetChild();
      _children_running = false;
    }
    return child_status;
  }

  void loadExecutor()
  {
    std::string script;
    if(!getInput("if", script))
    {
      throw RuntimeError("Missing parameter [if] in Precondition");
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
  bool _children_running = false;
};

}  // namespace BT
