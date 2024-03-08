/* Copyright (C) 2020 Davide Faconti -  All Rights Reserved
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

#include "behaviortree_cpp/controls/while_do_else_node.h"

namespace BT
{
WhileDoElseNode::WhileDoElseNode(const std::string& name)
  : ControlNode::ControlNode(name, {})
{
  setRegistrationID("WhileDoElse");
}

void WhileDoElseNode::halt()
{
  ControlNode::halt();
}

NodeStatus WhileDoElseNode::tick()
{
  const size_t children_count = children_nodes_.size();

  if(children_count != 2 && children_count != 3)
  {
    throw std::logic_error("WhileDoElseNode must have either 2 or 3 children");
  }

  setStatus(NodeStatus::RUNNING);

  NodeStatus condition_status = children_nodes_[0]->executeTick();

  if(condition_status == NodeStatus::RUNNING)
  {
    return condition_status;
  }

  NodeStatus status = NodeStatus::IDLE;

  if(condition_status == NodeStatus::SUCCESS)
  {
    if(children_count == 3)
    {
      haltChild(2);
    }
    status = children_nodes_[1]->executeTick();
  }
  else if(condition_status == NodeStatus::FAILURE)
  {
    if(children_count == 3)
    {
      haltChild(1);
      status = children_nodes_[2]->executeTick();
    }
    else if(children_count == 2)
    {
      status = NodeStatus::FAILURE;
    }
  }

  if(status == NodeStatus::RUNNING)
  {
    return NodeStatus::RUNNING;
  }
  else
  {
    resetChildren();
    return status;
  }
}

}  // namespace BT
