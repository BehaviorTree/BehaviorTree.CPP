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
  /// status to return when the action is completed
  NodeStatus return_status = NodeStatus::SUCCESS;

  /// script to execute when actions is completed
  std::string post_script;

  /// if async_delay > 0, this action become asynchronous and wait this amount of time
  std::chrono::milliseconds async_delay = std::chrono::milliseconds(0);

  /// C++ callback to execute at the beginning
  std::function<void()> pre_func;

  /// C++ callback to execute at the end
  std::function<void()> post_func;
};

/**
 * @brief The TestNode is a Node that can be configure to:
 *
 * 1. Return a specific status (SUCCESS / FAILURE)
 * 2. Execute a post condition script (unless halted)
 * 3. Either complete immediately (synchronous action), or after a
 *    given period of time (asynchronous action)
 *
 * This behavior is changed by the parameters pased with TestNodeConfig.
 *
 * This particular node is created by the factory when TestNodeConfig is
 * added as a substitution rule:
 *
 *    TestNodeConfig test_config;
 *    // change fields of test_config
 *    factory.addSubstitutionRule(pattern, test_config);
 *
 * See tutorial 11 for more details.
 */
class TestNode : public BT::StatefulActionNode
{
public:
  TestNode(const std::string& name, const NodeConfig& config,
           TestNodeConfig test_config = {}) :
    StatefulActionNode(name, config),
        _test_config(std::move(test_config))
  {
    setRegistrationID("TestNode");
  }

  static PortsList providedPorts()
  {
    return {};
  }

  void setConfig(const TestNodeConfig& config);

private:

  virtual NodeStatus onStart() override;

  virtual NodeStatus onRunning() override;

  virtual void onHalted() override;

  NodeStatus onCompleted();

  TestNodeConfig _test_config;
  ScriptFunction _executor;
  TimerQueue<> _timer;
  std::atomic_bool _completed;
};

}   // namespace BT
