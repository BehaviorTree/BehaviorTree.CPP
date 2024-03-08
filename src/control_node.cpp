/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/control_node.h"

namespace BT
{
ControlNode::ControlNode(const std::string& name, const NodeConfig& config)
  : TreeNode::TreeNode(name, config)
{}

void ControlNode::addChild(TreeNode* child)
{
  children_nodes_.push_back(child);
}

size_t ControlNode::childrenCount() const
{
  return children_nodes_.size();
}

void ControlNode::halt()
{
  resetChildren();
  resetStatus();  // might be redundant
}

void ControlNode::resetChildren()
{
  for(auto child : children_nodes_)
  {
    if(child->status() == NodeStatus::RUNNING)
    {
      child->haltNode();
    }
    child->resetStatus();
  }
}

const std::vector<TreeNode*>& ControlNode::children() const
{
  return children_nodes_;
}

void ControlNode::haltChild(size_t i)
{
  auto child = children_nodes_[i];
  if(child->status() == NodeStatus::RUNNING)
  {
    child->haltNode();
  }
  child->resetStatus();
}

void ControlNode::haltChildren()
{
  for(size_t i = 0; i < children_nodes_.size(); i++)
  {
    haltChild(i);
  }
}

void ControlNode::haltChildren(size_t first)
{
  for(size_t i = first; i < children_nodes_.size(); i++)
  {
    haltChild(i);
  }
}

}  // namespace BT
