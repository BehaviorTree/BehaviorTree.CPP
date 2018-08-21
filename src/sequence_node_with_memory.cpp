/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018 Davide Faconti -  All Rights Reserved
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

#include "behavior_tree_core/sequence_node_with_memory.h"

namespace BT
{
constexpr const char* SequenceNodeWithMemory::RESET_POLICY;

SequenceNodeWithMemory::SequenceNodeWithMemory(const std::string& name, ResetPolicy reset_policy)
  : ControlNode::ControlNode(name, {{RESET_POLICY, toStr(reset_policy)}})
  , current_child_idx_(0)
  , reset_policy_(reset_policy)
{
}

SequenceNodeWithMemory::SequenceNodeWithMemory(const std::string& name, const NodeParameters& params)
  : ControlNode::ControlNode(name, params), current_child_idx_(0), reset_policy_(ON_SUCCESS_OR_FAILURE)
{
    auto param = getParam<ResetPolicy>(RESET_POLICY);
    if(param){
        reset_policy_ = param.value();
    }
}

NodeStatus SequenceNodeWithMemory::tick()
{
    // Vector size initialization. N_of_children_ could change at runtime if you edit the tree
    const unsigned N_of_children = children_nodes_.size();

    setStatus(NodeStatus::RUNNING);

    // Routing the ticks according to the sequence node's (with memory) logic:
    while (current_child_idx_ < N_of_children)
    {
        /*      Ticking an action is different from ticking a condition. An action executed some portion of code in another thread.
                We want this thread detached so we can cancel its execution (when the action no longer receive ticks).
                Hence we cannot just call the method Tick() from the action as doing so will block the execution of the tree.
                For this reason if a child of this node is an action, then we send the tick using the tick engine. Otherwise we call the method Tick() and wait for the response.
        */
        TreeNode* current_child_node = children_nodes_[current_child_idx_];

        const NodeStatus child_status = current_child_node->executeTick();

        if (child_status != NodeStatus::SUCCESS)
        {
            // If the  child status is not success, return the status
            if (child_status == NodeStatus::FAILURE && reset_policy_ != ON_SUCCESS)
            {
                for (unsigned t = 0; t <= current_child_idx_; t++)
                {
                    children_nodes_[t]->setStatus(NodeStatus::IDLE);
                }
                current_child_idx_ = 0;
            }
            return child_status;
        }
        else if (current_child_idx_ < N_of_children - 1)
        {
            // If the  child status is success, continue to the next child
            // (if any, hence if(current_child_ != N_of_children_ - 1) ) in the for loop (if any).
            current_child_idx_++;
        }
        else
        {
            // if it the last child.
            if (child_status == NodeStatus::SUCCESS || reset_policy_ != ON_FAILURE)
            {
                // if it the last child and it has returned SUCCESS, reset the memory
                for (unsigned t = 0; t <= current_child_idx_; t++)
                {
                    children_nodes_[t]->setStatus(NodeStatus::IDLE);
                }
                current_child_idx_ = 0;
            }
            return child_status;
        }
    }
    throw std::runtime_error("This is not supposed to happen");
}

void SequenceNodeWithMemory::halt()
{
    current_child_idx_ = 0;
    ControlNode::halt();
}
}
