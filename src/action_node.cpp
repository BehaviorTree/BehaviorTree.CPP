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

#include "behavior_tree_core/action_node.h"

BT::ActionNode::ActionNode(std::string name) : LeafNode::LeafNode(name), loop_(true)
{
    thread_ = std::thread(&ActionNode::waitForTick, this);
}

void BT::ActionNode::waitForTick()
{
    while (loop_.load())
    {
        DEBUG_STDOUT(name() << " WAIT FOR TICK");
        tick_engine_.wait();
        DEBUG_STDOUT(name() << " TICK RECEIVED");

        // check this again because the tick_engine_ could be
        // notified from the method stopAndJoinThread
        if (loop_.load())
        {
            setStatus(BT::RUNNING);
            BT::NodeStatus status = asyncTick();
            setStatus(status);
        }
    }
}

BT::NodeStatus BT::ActionNode::tick()
{
    NodeStatus stat = status();

    if (stat == BT::IDLE || stat == BT::HALTED)
    {
        DEBUG_STDOUT("NEEDS TO TICK " << name());
        tick_engine_.notify();
        stat = waitValidStatus();
    }
    return stat;
}

void BT::ActionNode::stopAndJoinThread()
{
    loop_ = false;
    tick_engine_.notify();
    thread_.join();
}
