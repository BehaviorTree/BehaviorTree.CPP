/*  Copyright (C) 2023 Davide Faconti -  All Rights Reserved
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

namespace BT
{
/**
 * Action that removes an entry from the blackboard and return SUCCESS.
 */
class UnsetBlackboardNode : public SyncActionNode
{
public:
  UnsetBlackboardNode(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {
    setRegistrationID("UnsetBlackboard");
  }

  static PortsList providedPorts()
  {
    return { InputPort<std::string>("key", "Key of the entry to remove") };
  }

private:
  virtual BT::NodeStatus tick() override
  {
    std::string key;
    if(!getInput("key", key))
    {
      throw RuntimeError("missing input port [key]");
    }
    config().blackboard->unset(key);
    return NodeStatus::SUCCESS;
  }
};
}  // namespace BT
