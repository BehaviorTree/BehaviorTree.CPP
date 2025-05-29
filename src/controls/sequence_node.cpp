/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/controls/sequence_node.h"

namespace BT
{
SequenceNode::SequenceNode(const std::string& name, bool make_async)
  : ControlNode::ControlNode(name, {}), current_child_idx_(0), asynch_(make_async)
{
  if(asynch_)
    setRegistrationID("AsyncSequence");
  else
    setRegistrationID("Sequence");
}

void SequenceNode::halt()
{
  current_child_idx_ = 0;
  skipped_count_ = 0;
  ControlNode::halt();
}

NodeStatus SequenceNode::tick()
{
  const size_t children_count = children_nodes_.size();

  if(!isStatusActive(status()))
  {
    skipped_count_ = 0;
  }

  setStatus(NodeStatus::RUNNING);

  while(current_child_idx_ < children_count)
  {
    TreeNode* current_child_node = children_nodes_[current_child_idx_];

    auto prev_status = current_child_node->status();
    const NodeStatus child_status = current_child_node->executeTick();

    switch(child_status)
    {
      case NodeStatus::RUNNING: {
        return NodeStatus::RUNNING;
      }
      case NodeStatus::FAILURE: {
        // Reset on failure
        resetChildren();
        current_child_idx_ = 0;
        return child_status;
      }
      case NodeStatus::SUCCESS: {
        current_child_idx_++;
        // Return the execution flow if the child is async,
        // to make this interruptible.
        if(asynch_ && requiresWakeUp() && prev_status == NodeStatus::IDLE &&
           current_child_idx_ < children_count)
        {
          emitWakeUpSignal();
          return NodeStatus::RUNNING;
        }
      }
      break;

      case NodeStatus::SKIPPED: {
        // It was requested to skip this node
        current_child_idx_++;
        skipped_count_++;
      }
      break;

      case NodeStatus::IDLE: {
        throw LogicError("[", name(), "]: A children should not return IDLE");
      }
    }  // end switch
  }    // end while loop

  // The entire while loop completed. This means that all the children returned SUCCESS.
  const bool all_children_skipped = (skipped_count_ == children_count);
  if(current_child_idx_ == children_count)
  {
    resetChildren();
    current_child_idx_ = 0;
    skipped_count_ = 0;
  }
  // Skip if ALL the nodes have been skipped
  return (all_children_skipped) ? NodeStatus::SKIPPED : NodeStatus::SUCCESS;
}

}  // namespace BT
