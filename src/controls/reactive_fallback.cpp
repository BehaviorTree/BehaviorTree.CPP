/* Copyright (C) 2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/controls/reactive_fallback.h"

namespace BT
{

bool ReactiveFallback::throw_if_multiple_running = false;

void ReactiveFallback::EnableException(bool enable)
{
  ReactiveFallback::throw_if_multiple_running = enable;
}

NodeStatus ReactiveFallback::tick()
{
  bool all_skipped = true;
  if(status() == NodeStatus::IDLE)
  {
    running_child_ = -1;
  }
  setStatus(NodeStatus::RUNNING);

  for(size_t index = 0; index < childrenCount(); index++)
  {
    TreeNode* current_child_node = children_nodes_[index];
    const NodeStatus child_status = current_child_node->executeTick();

    // switch to RUNNING state as soon as you find an active child
    all_skipped &= (child_status == NodeStatus::SKIPPED);

    switch(child_status)
    {
      case NodeStatus::RUNNING: {
        // reset the previous children, to make sure that they are
        // in IDLE state the next time we tick them
        for(size_t i = 0; i < childrenCount(); i++)
        {
          if(i != index)
          {
            haltChild(i);
          }
        }
        if(running_child_ == -1)
        {
          running_child_ = int(index);
        }
        else if(throw_if_multiple_running && running_child_ != int(index))
        {
          throw LogicError("[ReactiveFallback]: only a single child can return RUNNING.\n"
                           "This throw can be disabled with "
                           "ReactiveFallback::EnableException(false)");
        }
        return NodeStatus::RUNNING;
      }

      case NodeStatus::FAILURE:
        break;

      case NodeStatus::SUCCESS: {
        resetChildren();
        return NodeStatus::SUCCESS;
      }

      case NodeStatus::SKIPPED: {
        // to allow it to be skipped again, we must reset the node
        haltChild(index);
      }
      break;

      case NodeStatus::IDLE: {
        throw LogicError("[", name(), "]: A children should not return IDLE");
      }
    }  // end switch
  }    //end for

  resetChildren();

  // Skip if ALL the nodes have been skipped
  return all_skipped ? NodeStatus::SKIPPED : NodeStatus::FAILURE;
}

void ReactiveFallback::halt()
{
  running_child_ = -1;
  ControlNode::halt();
}

}  // namespace BT
