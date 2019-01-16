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

#include "behaviortree_cpp/decorators/repeat_node.h"

namespace BT
{
constexpr const char* RepeatNode::NUM_CYCLES;

RepeatNode::RepeatNode(const std::string& name, unsigned int NTries)
    : DecoratorNode(name, {} ),
    num_cycles_(NTries),
    try_index_(0),
    read_parameter_from_ports_(false)
{
     setRegistrationID("Repeat");
}

RepeatNode::RepeatNode(const std::string& name, const NodeConfiguration& config)
  : DecoratorNode(name, config),
    try_index_(0),
    read_parameter_from_ports_(true)
{

}

NodeStatus RepeatNode::tick()
{
    if( read_parameter_from_ports_ )
    {
        if( !getInput(NUM_CYCLES, num_cycles_) )
        {
            throw std::runtime_error("Missing parameter [num_cycles] in RepeatNode");
        }
    }

    setStatus(NodeStatus::RUNNING);
    NodeStatus child_state = child_node_->executeTick();

    switch (child_state)
    {
        case NodeStatus::SUCCESS:
        {
            try_index_++;
            if (try_index_ >= num_cycles_)
            {
                try_index_ = 0;
                return (NodeStatus::SUCCESS);
            }
        }
        break;

        case NodeStatus::FAILURE:
        {
            try_index_ = 0;
            return (NodeStatus::FAILURE);
        }

        case NodeStatus::RUNNING:
        {
            return (NodeStatus::RUNNING);
        }

        default:
        {
            // TODO throw?
        }
    }
    return status();
}

void RepeatNode::halt()
{
    try_index_ = 0;
    DecoratorNode::halt();
}

}
