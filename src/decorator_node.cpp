/* Copyright (C) 2015-2017 Michele Colledanchise - All Rights Reserved
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

#include "behaviortree_cpp/decorator_node.h"

namespace BT
{
DecoratorNode::DecoratorNode(const std::string& name, const NodeConfig& config)
  : TreeNode::TreeNode(name, config), child_node_(nullptr)
{}

void DecoratorNode::setChild(TreeNode* child)
{
  if(child_node_)
  {
    throw BehaviorTreeException("Decorator [", name(), "] has already a child assigned");
  }

  child_node_ = child;
}

void DecoratorNode::halt()
{
  resetChild();
  resetStatus();  // might be redundant
}

const TreeNode* DecoratorNode::child() const
{
  return child_node_;
}

TreeNode* DecoratorNode::child()
{
  return child_node_;
}

void DecoratorNode::haltChild()
{
  resetChild();
}

void DecoratorNode::resetChild()
{
  if(!child_node_)
  {
    return;
  }
  if(child_node_->status() == NodeStatus::RUNNING)
  {
    child_node_->haltNode();
  }
  child_node_->resetStatus();
}

SimpleDecoratorNode::SimpleDecoratorNode(const std::string& name,
                                         TickFunctor tick_functor,
                                         const NodeConfig& config)
  : DecoratorNode(name, config), tick_functor_(std::move(tick_functor))
{}

NodeStatus SimpleDecoratorNode::tick()
{
  return tick_functor_(child()->executeTick(), *this);
}

NodeStatus DecoratorNode::executeTick()
{
  NodeStatus status = TreeNode::executeTick();
  NodeStatus child_status = child()->status();
  if(child_status == NodeStatus::SUCCESS || child_status == NodeStatus::FAILURE)
  {
    child()->resetStatus();
  }
  return status;
}

}  // namespace BT
