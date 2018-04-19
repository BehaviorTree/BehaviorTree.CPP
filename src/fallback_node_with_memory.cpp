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

#include "behavior_tree_core/fallback_node_with_memory.h"
#include <string>

BT::FallbackNodeWithMemory::FallbackNodeWithMemory(std::string name) : ControlNode::ControlNode(name)
{
    reset_policy_ = BT::ON_SUCCESS_OR_FAILURE;
    current_child_idx_ = 0;  // initialize the current running child
}


BT::FallbackNodeWithMemory::FallbackNodeWithMemory(std::string name, int reset_policy) : ControlNode::ControlNode(name)
{
    reset_policy_ = reset_policy;
    current_child_idx_ = 0;  // initialize the current running child
}


BT::ReturnStatus BT::FallbackNodeWithMemory::Tick()
{
    DEBUG_STDOUT(Name() << " ticked, memory counter: "<< current_child_idx_);

    // Vector size initialization. N_of_children_ could change at runtime if you edit the tree
    N_of_children_ = children_nodes_.size();

    // Routing the ticks according to the fallback node's (with memory) logic:
    while (current_child_idx_ < N_of_children_)
    {
        /*      Ticking an action is different from ticking a condition. An action executed some portion of code in another thread.
                We want this thread detached so we can cancel its execution (when the action no longer receive ticks).
                Hence we cannot just call the method Tick() from the action as doing so will block the execution of the tree.
                For this reason if a child of this node is an action, then we send the tick using the tick engine. Otherwise we call the method Tick() and wait for the response.
        */

        if (children_nodes_[current_child_idx_]->Type() == BT::ACTION_NODE)
        {
            // 1) If the child i is an action, read its state.
            // Action nodes runs in another thread, hence you cannot retrieve the status just by executing it.

            child_i_status_ = children_nodes_[current_child_idx_]->Status();
            DEBUG_STDOUT(Name() << " It is an action " << children_nodes_[current_child_idx_]->Name()
                         << " with status: " << child_i_status_);

            if (child_i_status_ == BT::IDLE || child_i_status_ == BT::HALTED)
            {
                // 1.1) If the action status is not running, the sequence node sends a tick to it.
                DEBUG_STDOUT(Name() << "NEEDS TO TICK " << children_nodes_[current_child_idx_]->Name());
                children_nodes_[current_child_idx_]->tick_engine.Tick();

                // waits for the tick to arrive to the child
                do
                {
                    child_i_status_ = children_nodes_[current_child_idx_]->Status();
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
            child_i_status_ = children_nodes_[current_child_idx_]->Tick();
            children_nodes_[current_child_idx_]->SetStatus(child_i_status_);

        }


        if (child_i_status_ == BT::SUCCESS ||child_i_status_ == BT::FAILURE )
        {
             // the child goes in idle if it has returned success or failure.
            children_nodes_[current_child_idx_]->SetStatus(BT::IDLE);
        }

        if (child_i_status_ != BT::FAILURE)
        {
            // If the  child status is not success, return the status
            DEBUG_STDOUT("the status of: " << Name() << " becomes " << child_i_status_);
            if (child_i_status_ == BT::SUCCESS && (reset_policy_ == BT::ON_SUCCESS
                                                  || reset_policy_ == BT::ON_SUCCESS_OR_FAILURE))
            {
                current_child_idx_ = 0;
            }
            SetStatus(child_i_status_);
            return child_i_status_;
        }
        else if (current_child_idx_ != N_of_children_ - 1)
        {
            // If the  child status is failure, continue to the next child
            // (if any, hence if(current_child_ != N_of_children_ - 1) ) in the for loop (if any).
            current_child_idx_++;
        }
        else
        {
            // If it the last child.
            if (child_i_status_ == BT::FAILURE)
            {
                // if it the last child and it has returned failure, reset the memory
                current_child_idx_ = 0;
            }
            SetStatus(child_i_status_);
            return child_i_status_;
        }
    }
    return BT::EXIT;
}


void BT::FallbackNodeWithMemory::Halt()
{
    current_child_idx_ = 0;
    BT::ControlNode::Halt();
}


