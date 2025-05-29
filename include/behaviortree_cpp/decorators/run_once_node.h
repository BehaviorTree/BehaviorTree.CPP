/*  Copyright (C) 2023 Davide Faconti -  All Rights Reserved
*   Copyright (C) 2022 Gaël Écorchard, Czech Institute of Informatics, Robotics, and Cybernetics (ciirc) <gael.ecorchard@cvut.cz>
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
 * @brief The RunOnceNode is used when you want to execute the child
 * only once.
 * If the child is asynchronous, we will tick until either SUCCESS or FAILURE is
 * returned.
 *
 * After that first execution, you can set value of the port "then_skip" to:
 *
 * - if TRUE (default), the node will be skipped in the future.
 * - if FALSE, return synchronously the same status returned by the child, forever.
 */
class RunOnceNode : public DecoratorNode
{
public:
  RunOnceNode(const std::string& name, const NodeConfig& config)
    : DecoratorNode(name, config)
  {
    setRegistrationID("RunOnce");
  }

  static PortsList providedPorts()
  {
    return { InputPort<bool>("then_skip", true,
                             "If true, skip after the first execution, "
                             "otherwise return the same NodeStatus returned once by the "
                             "child.") };
  }

private:
  virtual BT::NodeStatus tick() override;

  bool already_ticked_ = false;
  NodeStatus returned_status_ = NodeStatus::IDLE;
};

//------------ implementation ----------------------------

inline NodeStatus RunOnceNode::tick()
{
  bool skip = true;
  if(auto const res = getInput<bool>("then_skip"))
  {
    skip = res.value();
  }

  if(already_ticked_)
  {
    return skip ? NodeStatus::SKIPPED : returned_status_;
  }

  setStatus(NodeStatus::RUNNING);
  const NodeStatus status = child_node_->executeTick();

  if(isStatusCompleted(status))
  {
    already_ticked_ = true;
    returned_status_ = status;
    resetChild();
  }
  return status;
}

}  // namespace BT
