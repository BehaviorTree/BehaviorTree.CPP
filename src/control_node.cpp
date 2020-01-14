/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp_v3/control_node.h"

namespace BT
{
ControlNode::ControlNode(const std::string& name, const NodeConfiguration& config)
  : TreeNode::TreeNode(name, config)
{
}

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
    haltChildren(0);
    setStatus(NodeStatus::IDLE);
}

const std::vector<TreeNode*>& ControlNode::children() const
{
    return children_nodes_;
}

void ControlNode::haltChildren(size_t i)
{
    for (size_t j = i; j < children_nodes_.size(); j++)
    {
        auto child = children_nodes_[j];
        if (child->status() == NodeStatus::RUNNING)
        {
            child->halt();
        }
        child->setStatus(NodeStatus::IDLE);
    }
}

} // end namespace
