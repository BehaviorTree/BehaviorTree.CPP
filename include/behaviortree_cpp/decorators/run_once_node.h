/*  Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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
 * @brief The RunOnceNode returns its child status but calls the child's tick() function only once.
 *
 * The pre- and postconditions are not checked again.
 */
class RunOnceNode : public DecoratorNode
{
public:
  RunOnceNode(const std::string& name) : DecoratorNode(name, {})
  {
    setRegistrationID("RunOnce");
  }

private:
  virtual BT::NodeStatus tick() override;
  bool already_ticked_{false};
};

//------------ implementation ----------------------------

inline NodeStatus RunOnceNode::tick()
{
  setStatus(NodeStatus::RUNNING);

  if (already_ticked_)
  {
    return child_node_->status();
  }
  else
  {
    already_ticked_ = true;
    return child_node_->executeTick();
  }
}
}   // namespace BT
