
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


#include <sequence_node.h>
#include <string>


BT::SequenceNode::SequenceNode(std::string name) : ControlNode::ControlNode(name) {}

BT::SequenceNode::~SequenceNode() {}

BT::ReturnStatus BT::SequenceNode::Tick()
{
    // gets the number of children. The number could change if, at runtime, one edits the tree.
    N_of_children_ = children_nodes_.size();

    // Routing the ticks according to the sequence node's logic:

    for (unsigned int i = 0; i < N_of_children_; i++)
    {
        /*      Ticking an action is different from ticking a condition. An action executed some portion of code in another thread.
                We want this thread detached so we can cancel its execution (when the action no longer receive ticks).
                Hence we cannot just call the method Tick() from the action as doing so will block the execution of the tree.
                For this reason if a child of this node is an action, then we send the tick using the tick engine. Otherwise we call the method Tick() and wait for the response.
        */
        if (children_nodes_[i]->get_type() == BT::ACTION_NODE)
        {
            // 1) If the child i is an action, read its state.
            child_i_status_ = children_nodes_[i]->get_status();

            if (child_i_status_ == BT::IDLE || child_i_status_ == BT::HALTED)
            {
                // 1.1) If the action status is not running, the sequence node sends a tick to it.
                DEBUG_STDOUT(get_name() << "NEEDS TO TICK " << children_nodes_[i]->get_name());
                children_nodes_[i]->tick_engine.Tick();

                // waits for the tick to arrive to the child
                do
                {
                    child_i_status_ = children_nodes_[i]->get_status();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                while (child_i_status_ != BT::RUNNING && child_i_status_ != BT::SUCCESS
                       && child_i_status_ != BT::FAILURE);
            }
        }
        else
        {
            // 2) if it's not an action:
            // Send the tick and wait for the response;
            child_i_status_ = children_nodes_[i]->Tick();
            children_nodes_[i]->set_status(child_i_status_);
        }
        // Ponderate on which status to send to the parent
        if (child_i_status_ != BT::SUCCESS)
        {
            // If the  child status is not success, halt the next children and return the status to your parent.
            if (child_i_status_ == BT::FAILURE)
            {
                children_nodes_[i]->set_status(BT::IDLE);  // the child goes in idle if it has returned failure.
            }

            DEBUG_STDOUT(get_name() << " is HALTING children from " << (i+1));
            HaltChildren(i+1);
            set_status(child_i_status_);
            return child_i_status_;
        }
        else
        {
            // the child returned success.
            children_nodes_[i]->set_status(BT::IDLE);

            if (i == N_of_children_ - 1)
            {
                // If the  child status is success, and it is the last child to be ticked,
                // then the sequence has succeeded.
                set_status(BT::SUCCESS);
                return BT::SUCCESS;
            }
        }
    }
    return BT::EXIT;
}

int BT::SequenceNode::DrawType()
{
    return BT::SEQUENCE;
}
