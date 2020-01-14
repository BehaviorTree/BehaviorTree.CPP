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

#include "behaviortree_cpp_v3/decorators/retry_node.h"

namespace BT
{
constexpr const char* RetryNode::NUM_ATTEMPTS;

RetryNode::RetryNode(const std::string& name, int NTries)
    : DecoratorNode(name, {} ),
    max_attempts_(NTries),
    try_index_(0),
    read_parameter_from_ports_(false)
{
    setRegistrationID("RetryUntilSuccesful");
}

RetryNode::RetryNode(const std::string& name, const NodeConfiguration& config)
  : DecoratorNode(name, config),
    max_attempts_(0),
    try_index_(0),
    read_parameter_from_ports_(true)
{
}

void RetryNode::halt()
{
    try_index_ = 0;
    DecoratorNode::halt();
}

NodeStatus RetryNode::tick()
{
    if( read_parameter_from_ports_ )
    {
        if( !getInput(NUM_ATTEMPTS, max_attempts_) )
        {
            throw RuntimeError("Missing parameter [", NUM_ATTEMPTS,"] in RetryNode");
        }
    }

    setStatus(NodeStatus::RUNNING);

    while (try_index_ < max_attempts_)
    {
        NodeStatus child_state = child_node_->executeTick();

        switch (child_state)
        {
            case NodeStatus::SUCCESS:
            {
                try_index_ = 0;
                return (NodeStatus::SUCCESS);
            }

            case NodeStatus::FAILURE:
            {
                try_index_++;
            }
            break;

            case NodeStatus::RUNNING:
            {
                return NodeStatus::RUNNING;
            }

            default:
            {
                throw LogicError("A child node must never return IDLE");
            }
        }
    }

    try_index_ = 0;
    return NodeStatus::FAILURE;
}

}
