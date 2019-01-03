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

#include "behaviortree_cpp/controls/sequence_star_node.h"

namespace BT
{

constexpr const char* SequenceStarNode::RESET_PARAM;

SequenceStarNode::SequenceStarNode(const std::string& name, bool reset_on_failure)
    : ControlNode::ControlNode(name, {} )
  , current_child_idx_(0)
  , reset_on_failure_(reset_on_failure)
  , read_parameter_from_ports_(false)
{
    setRegistrationID("SequenceStar");
}

SequenceStarNode::SequenceStarNode(const std::string& name, const NodeConfiguration& config)
  : ControlNode::ControlNode(name, config), current_child_idx_(0),
    read_parameter_from_ports_(true)
{
}

NodeStatus SequenceStarNode::tick()
{
    if(read_parameter_from_ports_)
    {
        if( !getInput(RESET_PARAM, reset_on_failure_) )
        {
            throw std::runtime_error("Missing parameter [reset_on_failure] in SequenceStarNode");
        }
    }

    const size_t children_count = children_nodes_.size();

    setStatus(NodeStatus::RUNNING);

    while (current_child_idx_ < children_count)
    {
        TreeNode* current_child_node = children_nodes_[current_child_idx_];
        const NodeStatus child_status = current_child_node->executeTick();

        switch (child_status)
        {
            case NodeStatus::RUNNING:
            {
                return child_status;
            }
            case NodeStatus::FAILURE:
            {
                if (reset_on_failure_)
                {
                    haltChildren(0);
                    current_child_idx_ = 0;
                }
                else
                {
                    haltChildren(current_child_idx_);
                }
                return child_status;
            }
            case NodeStatus::SUCCESS:
            {
                current_child_idx_++;
            }
            break;

            case NodeStatus::IDLE:
            {
                throw std::runtime_error("This is not supposed to happen");
            }
        }   // end switch
    }       // end while loop

    // The entire while loop completed. This means that all the children returned SUCCESS.
    if (current_child_idx_ == children_count)
    {
        haltChildren(0);
        current_child_idx_ = 0;
    }
    return NodeStatus::SUCCESS;
}

void SequenceStarNode::halt()
{
    current_child_idx_ = 0;
    ControlNode::halt();
}
}
