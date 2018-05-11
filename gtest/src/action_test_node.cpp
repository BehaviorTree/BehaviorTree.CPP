/* Copyright (C) 2015-2017 Michele Colledanchise -  All Rights Reserved
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

#include "action_test_node.h"
#include <string>

BT::ActionTestNode::ActionTestNode(std::string name) : ActionNode(name)
{
    boolean_value_ = true;
    time_ = 3;
    stop_loop_ = false;
}

BT::ActionTestNode::~ActionTestNode()
{
    halt();
}

BT::NodeStatus BT::ActionTestNode::tick()
{
    stop_loop_ = false;
    int i = 0;
    while ( !stop_loop_ && i++ < time_)
    {
        DEBUG_STDOUT(" Action " << name() << "running! Thread id:" << std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if ( !stop_loop_ )
    {
        if (boolean_value_)
        {
            DEBUG_STDOUT(" Action " << name() << " Done!");
            return BT::SUCCESS;
        }
        else
        {
            DEBUG_STDOUT(" Action " << name() << " FAILED!");
            return BT::FAILURE;
        }
    }
    else
    {
        return BT::IDLE;
    }
}

void BT::ActionTestNode::halt()
{
    stop_loop_ = true;
    setStatus(BT::IDLE);
    DEBUG_STDOUT("HALTED state set!");
}

void BT::ActionTestNode::set_time(int time)
{
    time_ = time;
}

void BT::ActionTestNode::set_boolean_value(bool boolean_value)
{
    boolean_value_ = boolean_value;
}
