/* Copyright (C) 2015-2017 Michele Colledanchise - All Rights Reserved
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


#include <action_test_node.h>
#include <string>


BT::ActionTestNode::ActionTestNode(std::string name) : ActionNode::ActionNode(name)
{
    boolean_value_ = true;
    time_ = 3;
}

BT::ActionTestNode::~ActionTestNode() {}

BT::ReturnStatus BT::ActionTestNode::Tick()
{

    int i = 0;
    while (get_status() != BT::HALTED && i++ < time_)
    {
        DEBUG_STDOUT(" Action " << get_name() << "running! Thread id:" << std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (get_status() != BT::HALTED)
    {
        if (boolean_value_)
        {
            DEBUG_STDOUT(" Action " << get_name() << " Done!");
            return BT::SUCCESS;
        }
        else
        {
            DEBUG_STDOUT(" Action " << get_name() << " FAILED!");
            return BT::FAILURE;
        }
    }
    else
    {
        return BT::HALTED;
    }
}

void BT::ActionTestNode::Halt()
{
    set_status(BT::HALTED);
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


