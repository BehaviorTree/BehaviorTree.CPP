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

#include "behavior_tree_core/fallback_node.h"

BT::FallbackNode::FallbackNode(std::string name) : ControlNode::ControlNode(name)
{
}

BT::NodeStatus BT::FallbackNode::Tick()
{
    {
        // gets the number of children. The number could change if, at runtime, one edits the tree.
        const unsigned N_of_children = children_nodes_.size();

        // Routing the ticks according to the fallback node's logic:

        for (unsigned i = 0; i < N_of_children; i++)
        {
            auto& child_node = children_nodes_[i];

            /*      Ticking an action is different from ticking a condition. An action executed some portion of code in another thread.
                    We want this thread detached so we can cancel its execution (when the action no longer receive ticks).
                    Hence we cannot just call the method Tick() from the action as doing so will block the execution of the tree.
                    For this reason if a child of this node is an action, then we send the tick using the tick engine. Otherwise we call the method Tick() and wait for the response.
            */
            if (child_node->Type() == BT::ACTION_NODE)
            {
                // 1) If the child i is an action, read its state.
                child_i_status_ = child_node->Status();

                if (child_i_status_ == BT::IDLE || child_i_status_ == BT::HALTED)
                {
                    // 1.1) If the action status is not running, the sequence node sends a tick to it.
                    DEBUG_STDOUT(Name() << "NEEDS TO TICK " << child_node->Name());
                    child_node->tick_engine.Tick();

                    child_i_status_ = child_node->waitValidStatus();
                }
            }
            else
            {
                // 2) if it's not an action:
                // Send the tick and wait for the response;
                child_i_status_ = child_node->Tick();
                child_node->SetStatus(child_i_status_);
            }
            // Ponderate on which status to send to the parent
            if (child_i_status_ != BT::FAILURE)
            {
                if (child_i_status_ == BT::SUCCESS)
                {
                    child_node->SetStatus(BT::IDLE);   // the child goes in idle if it has returned success.
                }
                // If the  child status is not failure, halt the next children and return the status to your parent.
                DEBUG_STDOUT(Name() << " is HALTING children from " << (i + 1));
                HaltChildren(i + 1);
                SetStatus(child_i_status_);
                return child_i_status_;
            }
            else
            {
                // the child returned failure.
                child_node->SetStatus(BT::IDLE);
                if (i == N_of_children - 1)
                {
                    // If the  child status is failure, and it is the last child to be ticked,
                    // then the sequence has failed.
                    SetStatus(BT::FAILURE);
                    return BT::FAILURE;
                }
            }
        }
    }
    return BT::EXIT;
}
