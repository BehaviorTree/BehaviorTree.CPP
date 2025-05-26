/*  Copyright (C) 2022 Davide Faconti -  All Rights Reserved
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

#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/utils/timer_queue.h"
#include "behaviortree_cpp/scripting/script_parser.hpp"

namespace BT
{

struct TestNodeConfig
{
  /// status to return when the action is completed.
  NodeStatus return_status = NodeStatus::SUCCESS;

  /// script to execute when complete_func() returns SUCCESS
  std::string success_script;

  /// script to execute when complete_func() returns FAILURE
  std::string failure_script;

  /// script to execute when actions is completed
  std::string post_script;

  /// if async_delay > 0, this action become asynchronous and wait this amount of time
  std::chrono::milliseconds async_delay = std::chrono::milliseconds(0);

  /// Function invoked when the action is completed.
  /// If not specified, the node will return [return_status]
  std::function<NodeStatus(void)> complete_func;
};

/**
 * @brief The TestNode is a Node that can be configure to:
 *
 * 1. Return a specific status (SUCCESS / FAILURE)
 * 2. Execute a post condition script (unless halted)
 * 3. Either complete immediately (synchronous action), or after a
 *    given period of time (asynchronous action)
 *
 * This behavior is changed by the parameters passed with TestNodeConfig.
 *
 * This particular node is created by the factory when TestNodeConfig is
 * added as a substitution rule:
 *
 *    auto test_config = std::make_shared<TestNodeConfig>();
 *    // change fields of test_config
 *    factory.addSubstitutionRule(pattern, test_config);
 *
 * See tutorial 15 for more details.
 */
class TestNode : public BT::StatefulActionNode
{
public:
  // This constructor is deprecated, because it may cause problems if TestNodeConfig::complete_func is capturing
  // a reference to the TestNode, i.e. [this]. Use the constructor with std::shared_ptr<TestNodeConfig> instead.
  // For more details, see https://github.com/BehaviorTree/BehaviorTree.CPP/pull/967
  [[deprecated("prefer the constructor with std::shared_ptr<TestNodeConfig>")]] TestNode(
      const std::string& name, const NodeConfig& config, TestNodeConfig test_config);

  TestNode(const std::string& name, const NodeConfig& config,
           std::shared_ptr<TestNodeConfig> test_config);

  static PortsList providedPorts()
  {
    return {};
  }

protected:
  virtual NodeStatus onStart() override;

  virtual NodeStatus onRunning() override;

  virtual void onHalted() override;

  NodeStatus onCompleted();

  std::shared_ptr<TestNodeConfig> _config;
  ScriptFunction _success_executor;
  ScriptFunction _failure_executor;
  ScriptFunction _post_executor;
  TimerQueue<> _timer;
  std::atomic_bool _completed = false;
};

}  // namespace BT
