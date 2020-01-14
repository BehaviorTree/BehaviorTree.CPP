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

#include "behaviortree_cpp_v3/controls/parallel_node.h"

namespace BT
{

constexpr const char* ParallelNode::THRESHOLD_KEY;

ParallelNode::ParallelNode(const std::string& name, unsigned threshold)
    : ControlNode::ControlNode(name, {} ),
    threshold_(threshold),
    read_parameter_from_ports_(false)
{
    setRegistrationID("Parallel");
}

ParallelNode::ParallelNode(const std::string &name,
                               const NodeConfiguration& config)
    : ControlNode::ControlNode(name, config),
      threshold_(0),
      read_parameter_from_ports_(true)
{
}

NodeStatus ParallelNode::tick()
{
    if(read_parameter_from_ports_)
    {
        if( !getInput(THRESHOLD_KEY, threshold_) )
        {
            throw RuntimeError("Missing parameter [", THRESHOLD_KEY, "] in ParallelNode");
        }
    }

    size_t success_childred_num = 0;
    size_t failure_childred_num = 0;

    const size_t children_count = children_nodes_.size();

    if( children_count < threshold_)
    {
        throw LogicError("Number of children is less than threshold. Can never suceed.");
    }

    // Routing the tree according to the sequence node's logic:
    for (unsigned int i = 0; i < children_count; i++)
    {
        TreeNode* child_node = children_nodes_[i];

        bool in_skip_list = (skip_list_.count(i) != 0);

        NodeStatus child_status;
        if( in_skip_list )
        {
            child_status = child_node->status();
        }
        else {
            child_status = child_node->executeTick();
        }

        switch (child_status)
        {
            case NodeStatus::SUCCESS:
            {
                if( !in_skip_list )
                {
                    skip_list_.insert(i);
                }
                success_childred_num++;

                if (success_childred_num == threshold_)
                {
                    skip_list_.clear();
                    haltChildren(0);
                    return NodeStatus::SUCCESS;
                }
            } break;

            case NodeStatus::FAILURE:
            {
                if( !in_skip_list )
                {
                    skip_list_.insert(i);
                }
                failure_childred_num++;

                if (failure_childred_num > children_count - threshold_)
                {
                    skip_list_.clear();
                    haltChildren(0);
                    return NodeStatus::FAILURE;
                }
            } break;

            case NodeStatus::RUNNING:
            {
                // do nothing
            }  break;

            default:
            {
                throw LogicError("A child node must never return IDLE");
            }
        }
    }

    return NodeStatus::RUNNING;
}

void ParallelNode::halt()
{
    skip_list_.clear();
    ControlNode::halt();
}

unsigned int ParallelNode::thresholdM()
{
    return threshold_;
}

void ParallelNode::setThresholdM(unsigned int threshold_M)
{
    threshold_ = threshold_M;
}

}
