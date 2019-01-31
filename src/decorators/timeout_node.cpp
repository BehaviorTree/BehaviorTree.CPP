/*  Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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
#include "behaviortree_cpp/decorators/timeout_node.h"

namespace BT
{
TimeoutNode::TimeoutNode(const std::string& name, unsigned milliseconds)
    : DecoratorNode(name, {} ),
    child_halted_(false),
    timer_id_(0),
    msec_(milliseconds),
    read_parameter_from_ports_(false)
{
    setRegistrationID("Timeout");
}

TimeoutNode::TimeoutNode(const std::string& name, const NodeConfiguration& config)
  : DecoratorNode(name, config),
    child_halted_(false),
    timer_id_(0),
    msec_(0),
    read_parameter_from_ports_(true)
{
}

NodeStatus TimeoutNode::tick()
{
    if( read_parameter_from_ports_ )
    {
        if( !getInput("msec", msec_) )
        {
            throw RuntimeError("Missing parameter [msec] in TimeoutNode");
        }
    }

    setStatus(NodeStatus::RUNNING);

    if ( child()->status() != NodeStatus::RUNNING)
    {
        child_halted_ = false;

        if (msec_ > 0)
        {
            timer_id_ = timer().add(std::chrono::milliseconds(msec_),
                                    [this](bool aborted)
            {
                if (!aborted && child()->status() == NodeStatus::RUNNING)
                {
                    child_halted_ = true;
                }
            });
        }
    }

    if (child_halted_)
    {
        child()->halt();
        child()->setStatus(NodeStatus::IDLE);
        return NodeStatus::FAILURE;
    }
    else
    {
        auto child_status = child()->executeTick();
        if (child_status != NodeStatus::RUNNING)
        {
            timer().cancel(timer_id_);
        }
        setStatus(child_status);
    }

    return status();
}

}
