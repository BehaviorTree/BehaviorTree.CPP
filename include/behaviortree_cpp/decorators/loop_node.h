/*  Copyright (C) 2022 Davide Faconti -  All Rights Reserved
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

#pragma once

#include <deque>
#include "behaviortree_cpp/decorator_node.h"

namespace BT
{

// this object will allow us to modify the queue in place,
// when popping, in a thread safe-way and without copying the entire queue.
template <typename T>
using SharedQueue = std::shared_ptr<std::deque<T>>;

/**
 * @brief The LoopNode class is used to pop_front elements from a std::deque.
 * This element is copied into the port "value" and the child will be executed,
 * as long as we have elements in the queue.
 *
 * See Example 4: ex04_waypoints
 *
 * NOTE: unless T is `Any`, `string` or `double`, you must register the loop manually into
 * the factory.
 */
template <typename T = Any>
class LoopNode : public DecoratorNode
{
  bool child_running_ = false;
  SharedQueue<T> static_queue_;
  SharedQueue<T> current_queue_;

public:
  LoopNode(const std::string& name, const NodeConfig& config)
    : DecoratorNode(name, config)
  {
    auto raw_port = getRawPortValue("queue");
    if(!isBlackboardPointer(raw_port))
    {
      static_queue_ = convertFromString<SharedQueue<T>>(raw_port);
    }
  }

  NodeStatus tick() override
  {
    bool popped = false;
    if(status() == NodeStatus::IDLE)
    {
      child_running_ = false;
      // special case: the port contains a string that was converted to SharedQueue<T>
      if(static_queue_)
      {
        current_queue_ = std::make_shared<std::deque<T>>();
        *current_queue_ = *static_queue_;
      }
    }

    // Pop value from queue, if the child is not RUNNING
    if(!child_running_)
    {
      // if the port is static, any_ref is empty, otherwise it will keep access to
      // port locked for thread-safety
      AnyPtrLocked any_ref =
          static_queue_ ? AnyPtrLocked() : getLockedPortContent("queue");
      if(any_ref)
      {
        current_queue_ = any_ref.get()->cast<SharedQueue<T>>();
      }

      if(current_queue_ && !current_queue_->empty())
      {
        auto value = std::move(current_queue_->front());
        current_queue_->pop_front();
        popped = true;
        setOutput("value", value);
      }
    }

    if(!popped && !child_running_)
    {
      return getInput<NodeStatus>("if_empty").value();
    }

    if(status() == NodeStatus::IDLE)
    {
      setStatus(NodeStatus::RUNNING);
    }

    NodeStatus child_state = child_node_->executeTick();
    child_running_ = (child_state == NodeStatus::RUNNING);

    if(isStatusCompleted(child_state))
    {
      resetChild();
    }

    if(child_state == NodeStatus::FAILURE)
    {
      return NodeStatus::FAILURE;
    }
    return NodeStatus::RUNNING;
  }

  static PortsList providedPorts()
  {
    // we mark "queue" as BidirectionalPort, because the original element is modified
    return { BidirectionalPort<SharedQueue<T>>("queue"),
             InputPort<NodeStatus>("if_empty", NodeStatus::SUCCESS,
                                   "Status to return if queue is empty: "
                                   "SUCCESS, FAILURE, SKIPPED"),
             OutputPort<T>("value") };
  }
};

template <>
inline SharedQueue<int> convertFromString<SharedQueue<int>>(StringView str)
{
  auto parts = splitString(str, ';');
  SharedQueue<int> output = std::make_shared<std::deque<int>>();
  for(const StringView& part : parts)
  {
    output->push_back(convertFromString<int>(part));
  }
  return output;
}

template <>
inline SharedQueue<bool> convertFromString<SharedQueue<bool>>(StringView str)
{
  auto parts = splitString(str, ';');
  SharedQueue<bool> output = std::make_shared<std::deque<bool>>();
  for(const StringView& part : parts)
  {
    output->push_back(convertFromString<bool>(part));
  }
  return output;
}

template <>
inline SharedQueue<double> convertFromString<SharedQueue<double>>(StringView str)
{
  auto parts = splitString(str, ';');
  SharedQueue<double> output = std::make_shared<std::deque<double>>();
  for(const StringView& part : parts)
  {
    output->push_back(convertFromString<double>(part));
  }
  return output;
}

template <>
inline SharedQueue<std::string>
convertFromString<SharedQueue<std::string>>(StringView str)
{
  auto parts = splitString(str, ';');
  SharedQueue<std::string> output = std::make_shared<std::deque<std::string>>();
  for(const StringView& part : parts)
  {
    output->push_back(convertFromString<std::string>(part));
  }
  return output;
}

}  // namespace BT
