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

#include "behavior_tree_core/decorator_retry_node.h"

BT::DecoratorRetryNode::DecoratorRetryNode(std::string name, unsigned int NTries)
  : DecoratorNode(name), NTries_(NTries), TryIndx_(0)
{
}

BT::DecoratorRetryNode::DecoratorRetryNode(std::string name, const BT::NodeParameters& params)
  : DecoratorNode(name), NTries_(1), TryIndx_(0)
{
    auto it = params.find("num_attempts");
    if (it == params.end())
    {
        throw std::runtime_error("[DecoratorRetryNode] requires a parameter callen 'num_attempts'");
    }
    NTries_ = std::stoul(it->second);
}

BT::NodeStatus BT::DecoratorRetryNode::tick()
{
    setStatus(NodeStatus::RUNNING);
    BT::NodeStatus child_state = child_node_->executeTick();

    switch (child_state)
    {
        case NodeStatus::SUCCESS:
        {
            TryIndx_ = 0;
            setStatus(NodeStatus::SUCCESS);
        }
        break;

        case NodeStatus::FAILURE:
        {
            TryIndx_++;
            if (TryIndx_ >= NTries_)
            {
                TryIndx_ = 0;
                setStatus(NodeStatus::FAILURE);
            }
        }
        break;

        case NodeStatus::RUNNING:
        {
            setStatus(NodeStatus::RUNNING);
        }
        break;

        default:
        {
            // TODO throw?
        }
    }

    return status();
}
