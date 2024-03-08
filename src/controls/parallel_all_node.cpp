/* Copyright (C) 2023 Davide Faconti -  All Rights Reserved
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

#include <algorithm>
#include <cstddef>

#include "behaviortree_cpp/controls/parallel_all_node.h"

namespace BT
{

ParallelAllNode::ParallelAllNode(const std::string& name, const NodeConfig& config)
  : ControlNode::ControlNode(name, config), failure_threshold_(1)
{}

NodeStatus ParallelAllNode::tick()
{
  int max_failures = 0;
  if(!getInput("max_failures", max_failures))
  {
    throw RuntimeError("Missing parameter [max_failures] in ParallelNode");
  }
  const size_t children_count = children_nodes_.size();
  setFailureThreshold(max_failures);

  size_t skipped_count = 0;

  if(children_count < failure_threshold_)
  {
    throw LogicError("Number of children is less than threshold. Can never fail.");
  }

  setStatus(NodeStatus::RUNNING);

  // Routing the tree according to the sequence node's logic:
  for(size_t index = 0; index < children_count; index++)
  {
    TreeNode* child_node = children_nodes_[index];

    // already completed
    if(completed_list_.count(index) != 0)
    {
      continue;
    }

    NodeStatus const child_status = child_node->executeTick();

    switch(child_status)
    {
      case NodeStatus::SUCCESS: {
        completed_list_.insert(index);
      }
      break;

      case NodeStatus::FAILURE: {
        completed_list_.insert(index);
        failure_count_++;
      }
      break;

      case NodeStatus::RUNNING: {
        // Still working. Check the next
      }
      break;

      case NodeStatus::SKIPPED: {
        skipped_count++;
      }
      break;

      case NodeStatus::IDLE: {
        throw LogicError("[", name(), "]: A children should not return IDLE");
      }
    }
  }

  if(skipped_count == children_count)
  {
    return NodeStatus::SKIPPED;
  }
  if(skipped_count + completed_list_.size() >= children_count)
  {
    // DONE
    haltChildren();
    completed_list_.clear();
    auto const status = (failure_count_ >= failure_threshold_) ? NodeStatus::FAILURE :
                                                                 NodeStatus::SUCCESS;
    failure_count_ = 0;
    return status;
  }

  // Some children haven't finished, yet.
  return NodeStatus::RUNNING;
}

void ParallelAllNode::halt()
{
  completed_list_.clear();
  failure_count_ = 0;
  ControlNode::halt();
}

size_t ParallelAllNode::failureThreshold() const
{
  return failure_threshold_;
}

void ParallelAllNode::setFailureThreshold(int threshold)
{
  if(threshold < 0)
  {
    failure_threshold_ = size_t(std::max(int(children_nodes_.size()) + threshold + 1, 0));
  }
  else
  {
    failure_threshold_ = size_t(threshold);
  }
}

}  // namespace BT
