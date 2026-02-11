/* Copyright (C) 2018-2025 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/controls/try_catch_node.h"

namespace BT
{
TryCatchNode::TryCatchNode(const std::string& name, const NodeConfig& config)
  : ControlNode::ControlNode(name, config)
{
  setRegistrationID("TryCatch");
}

void TryCatchNode::halt()
{
  bool catch_on_halt = false;
  getInput("catch_on_halt", catch_on_halt);

  // If catch_on_halt is enabled and we were in the try-block (not already in catch),
  // execute the catch child synchronously before halting.
  if(catch_on_halt && !in_catch_ && isStatusActive(status()) &&
     children_nodes_.size() >= 2)
  {
    // Halt all try-block children first
    for(size_t i = 0; i < children_nodes_.size() - 1; i++)
    {
      haltChild(i);
    }

    // Tick the catch child. If it returns RUNNING, halt it too
    // (best-effort cleanup during halt).
    TreeNode* catch_child = children_nodes_.back();
    const NodeStatus catch_status = catch_child->executeTick();
    if(catch_status == NodeStatus::RUNNING)
    {
      haltChild(children_nodes_.size() - 1);
    }
  }

  current_child_idx_ = 0;
  skipped_count_ = 0;
  in_catch_ = false;
  ControlNode::halt();
}

NodeStatus TryCatchNode::tick()
{
  const size_t children_count = children_nodes_.size();

  if(children_count < 2)
  {
    throw LogicError("[", name(), "]: TryCatch requires at least 2 children");
  }

  if(!isStatusActive(status()))
  {
    skipped_count_ = 0;
    in_catch_ = false;
  }

  setStatus(NodeStatus::RUNNING);

  const size_t try_count = children_count - 1;

  // If we are in catch mode, tick the last child (cleanup)
  if(in_catch_)
  {
    TreeNode* catch_child = children_nodes_.back();
    const NodeStatus catch_status = catch_child->executeTick();

    if(catch_status == NodeStatus::RUNNING)
    {
      return NodeStatus::RUNNING;
    }

    // Catch child finished (SUCCESS or FAILURE): return FAILURE
    resetChildren();
    current_child_idx_ = 0;
    in_catch_ = false;
    return NodeStatus::FAILURE;
  }

  // Try-block: execute children 0..N-2 as a Sequence
  while(current_child_idx_ < try_count)
  {
    TreeNode* current_child_node = children_nodes_[current_child_idx_];
    const NodeStatus child_status = current_child_node->executeTick();

    switch(child_status)
    {
      case NodeStatus::RUNNING: {
        return NodeStatus::RUNNING;
      }
      case NodeStatus::FAILURE: {
        // Enter catch mode: halt try-block children, then tick catch child
        resetChildren();
        current_child_idx_ = 0;
        in_catch_ = true;
        return tick();  // re-enter to tick the catch child
      }
      case NodeStatus::SUCCESS: {
        current_child_idx_++;
      }
      break;
      case NodeStatus::SKIPPED: {
        current_child_idx_++;
        skipped_count_++;
      }
      break;
      case NodeStatus::IDLE: {
        throw LogicError("[", name(), "]: A child should not return IDLE");
      }
    }
  }

  // All try-children completed successfully (or were skipped)
  const bool all_skipped = (skipped_count_ == try_count);
  resetChildren();
  current_child_idx_ = 0;
  skipped_count_ = 0;

  return all_skipped ? NodeStatus::SKIPPED : NodeStatus::SUCCESS;
}

}  // namespace BT
