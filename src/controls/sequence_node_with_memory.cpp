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

#include "behavior_tree_core/controls/sequence_node_with_memory.h"

namespace BT
{

SequenceNodeWithMemory::SequenceNodeWithMemory(const std::string& name, bool reset_on_failure)
  : ControlNode::ControlNode(name, SequenceNodeWithMemory::requiredNodeParameters() )
  , current_child_idx_(0)
  , reset_on_failure_(reset_on_failure)
{
}

SequenceNodeWithMemory::SequenceNodeWithMemory(const std::string& name, const NodeParameters& params)
  : ControlNode::ControlNode(name, params)
  ,  current_child_idx_(0)
  , reset_on_failure_(false)
{
    auto param = getParam<bool>("reset_on_failure");
    if(param){
        reset_on_failure_ = param.value();
    }
}

NodeStatus SequenceNodeWithMemory::tick()
{
    // Vector size initialization. N_of_children_ could change at runtime if you edit the tree
    const unsigned N_of_children = children_nodes_.size();

    setStatus(NodeStatus::RUNNING);

    while (current_child_idx_ < N_of_children)
    {
        TreeNode* current_child_node = children_nodes_[current_child_idx_];
        const NodeStatus child_status = current_child_node->executeTick();

        switch( child_status )
        {
            case NodeStatus::RUNNING:{
                return child_status;
            }
            case NodeStatus::FAILURE:
            {
                if (reset_on_failure_)
                {
                    for (unsigned t = 0; t <= current_child_idx_; t++)
                    {
                        children_nodes_[t]->setStatus(NodeStatus::IDLE);
                    }
                    current_child_idx_ = 0;
                }
                else{ // just reset this child to try again
                    current_child_node->setStatus(NodeStatus::IDLE);
                }
                return child_status;
            }
            case NodeStatus::SUCCESS:
            {
                current_child_idx_++;
            }break;

            case NodeStatus::IDLE:
            {
                throw std::runtime_error("This is not supposed to happen");
            }
        } // end switch
    }// end while loop


    // The entire while loop completed. This means that all the children returned SUCCESS.
    if (current_child_idx_ == N_of_children)
    {
        for (unsigned t = 0; t < N_of_children; t++)
        {
            children_nodes_[t]->setStatus(NodeStatus::IDLE);
        }
        current_child_idx_ = 0;
    }
    return NodeStatus::SUCCESS;
}

void SequenceNodeWithMemory::halt()
{
    current_child_idx_ = 0;
    ControlNode::halt();
}
}
