/*  Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

namespace BT
{
/**
 * @brief The ForceSuccessNode returns always SUCCESS or RUNNING.
 */
class ForceSuccessNode : public DecoratorNode
{
public:
  ForceSuccessNode(const std::string& name) : DecoratorNode(name, {})
  {
    setRegistrationID("ForceSuccess");
  }

private:
  virtual BT::NodeStatus tick() override;
};

//------------ implementation ----------------------------

inline NodeStatus ForceSuccessNode::tick()
{
  setStatus(NodeStatus::RUNNING);

  const NodeStatus child_status = child_node_->executeTick();

  if(isStatusCompleted(child_status))
  {
    resetChild();
    return NodeStatus::SUCCESS;
  }

  // RUNNING or skipping
  return child_status;
}
}  // namespace BT
