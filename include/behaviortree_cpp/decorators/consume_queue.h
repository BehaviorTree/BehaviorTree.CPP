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

#include <list>
#include "behaviortree_cpp/decorator_node.h"
#include "behaviortree_cpp/actions/pop_from_queue.hpp"

namespace BT
{
/**
 * Execute the child node as long as the queue is not empty.
 * At each iteration, an item of type T is popped from the "queue" and
 * inserted in "popped_item".
 *
 * An empty queue will return SUCCESS
 */

template <typename T>
class [[deprecated("You are encouraged to use the LoopNode instead")]] ConsumeQueue
  : public DecoratorNode
{
public:
  ConsumeQueue(const std::string& name, const NodeConfig& config)
    : DecoratorNode(name, config)
  {}

  NodeStatus tick() override
  {
    // by default, return SUCCESS, even if queue is empty
    NodeStatus status_to_be_returned = NodeStatus::SUCCESS;

    if(running_child_)
    {
      NodeStatus child_state = child_node_->executeTick();
      running_child_ = (child_state == NodeStatus::RUNNING);
      if(running_child_)
      {
        return NodeStatus::RUNNING;
      }
      else
      {
        haltChild();
        status_to_be_returned = child_state;
      }
    }

    std::shared_ptr<ProtectedQueue<T>> queue;
    if(getInput("queue", queue) && queue)
    {
      std::unique_lock<std::mutex> lk(queue->mtx);
      auto& items = queue->items;

      while(!items.empty())
      {
        setStatus(NodeStatus::RUNNING);

        T val = items.front();
        items.pop_front();
        setOutput("popped_item", val);

        lk.unlock();
        NodeStatus child_state = child_node_->executeTick();
        lk.lock();

        running_child_ = (child_state == NodeStatus::RUNNING);
        if(running_child_)
        {
          return NodeStatus::RUNNING;
        }
        else
        {
          haltChild();
          if(child_state == NodeStatus::FAILURE)
          {
            return NodeStatus::FAILURE;
          }
          status_to_be_returned = child_state;
        }
      }
    }

    return status_to_be_returned;
  }

  static PortsList providedPorts()
  {
    return { InputPort<std::shared_ptr<ProtectedQueue<T>>>("queue"), OutputPort<T>("poppe"
                                                                                   "d_"
                                                                                   "ite"
                                                                                   "m") };
  }

private:
  bool running_child_ = false;
};

}  // namespace BT
