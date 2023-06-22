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

#include <algorithm>
#include <cstddef>

#include "behaviortree_cpp/controls/parallel_node.h"

namespace BT
{
constexpr const char* ParallelNode::THRESHOLD_FAILURE;
constexpr const char* ParallelNode::THRESHOLD_SUCCESS;

ParallelNode::ParallelNode(const std::string& name) :
  ControlNode::ControlNode(name, {}),
  success_threshold_(-1),
  failure_threshold_(1),
  read_parameter_from_ports_(false)
{
  setRegistrationID("Parallel");
}

ParallelNode::ParallelNode(const std::string& name, const NodeConfig& config) :
  ControlNode::ControlNode(name, config),
  success_threshold_(-1),
  failure_threshold_(1),
  read_parameter_from_ports_(true)
{}

NodeStatus ParallelNode::tick()
{
  if (read_parameter_from_ports_)
  {
    if (!getInput(THRESHOLD_SUCCESS, success_threshold_))
    {
      throw RuntimeError("Missing parameter [", THRESHOLD_SUCCESS, "] in ParallelNode");
    }

    if (!getInput(THRESHOLD_FAILURE, failure_threshold_))
    {
      throw RuntimeError("Missing parameter [", THRESHOLD_FAILURE, "] in ParallelNode");
    }
  }

  const size_t children_count = children_nodes_.size();

  if (children_count < successThreshold())
  {
    throw LogicError("Number of children is less than threshold. Can never succeed.");
  }

  if (children_count < failureThreshold())
  {
    throw LogicError("Number of children is less than threshold. Can never fail.");
  }

  setStatus(NodeStatus::RUNNING);

  size_t skipped_count = 0;

  // Routing the tree according to the sequence node's logic:
  for (size_t i = 0; i < children_count; i++)
  {
    if(completed_list_.count(i) == 0)
    {
      TreeNode* child_node = children_nodes_[i];
      NodeStatus const child_status = child_node->executeTick();

      switch (child_status)
      {
        case NodeStatus::SKIPPED: {
          skipped_count++;
        } break;

        case NodeStatus::SUCCESS: {
          completed_list_.insert(i);
          success_count_++;
        }
        break;

        case NodeStatus::FAILURE: {
          completed_list_.insert(i);
          failure_count_++;
        }
        break;

        case NodeStatus::RUNNING: {
          // Still working. Check the next
        }
        break;

        case NodeStatus::IDLE: {
          throw LogicError("[", name(), "]: A children should not return IDLE");
        }
      }
    }

    const size_t required_success_count = successThreshold();

    if(success_count_ >= required_success_count ||
        (success_threshold_ < 0 && (success_count_ + skipped_count) >= required_success_count))
    {
      clear();
      resetChildren();
      return NodeStatus::SUCCESS;
    }

    // It fails if it is not possible to succeed anymore or if
    // number of failures are equal to failure_threshold_
    if (((children_count - failure_count_) < required_success_count) ||
        (failure_count_ == failureThreshold()))
    {
      clear();
      resetChildren();
      return NodeStatus::FAILURE;
    }
  }
  // Skip if ALL the nodes have been skipped
  return (skipped_count == children_count) ? NodeStatus::SKIPPED : NodeStatus::RUNNING;
}

void ParallelNode::clear()
{
  completed_list_.clear();
  success_count_ = 0;
  failure_count_ = 0;
}

void ParallelNode::halt()
{
  clear();
  ControlNode::halt();
}

size_t ParallelNode::successThreshold() const
{
  if (success_threshold_ < 0)
  {
    return size_t(std::max(int(children_nodes_.size()) + success_threshold_ + 1, 0));
  }
  else
  {
    return size_t(success_threshold_);
  }
}

size_t ParallelNode::failureThreshold() const
{
  if (failure_threshold_ < 0)
  {
    return size_t(std::max(int(children_nodes_.size()) + failure_threshold_ + 1, 0));
  }
  else
  {
    return size_t(failure_threshold_);
  }
}

void ParallelNode::setSuccessThreshold(int threshold)
{
  success_threshold_ = threshold;
}

void ParallelNode::setFailureThreshold(int threshold)
{
  failure_threshold_ = threshold;
}

}   // namespace BT
