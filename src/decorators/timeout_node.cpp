/*  Copyright (C) 2018-2023 Davide Faconti -  All Rights Reserved
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

#include "behaviortree_cpp/decorators/timeout_node.h"

namespace BT
{

NodeStatus TimeoutNode::tick()
{
  if(read_parameter_from_ports_)
  {
    if(!getInput("msec", msec_))
    {
      throw RuntimeError("Missing parameter [msec] in TimeoutNode");
    }
  }

  if(!timeout_started_)
  {
    timeout_started_ = true;
    setStatus(NodeStatus::RUNNING);
    child_halted_ = false;

    if(msec_ > 0)
    {
      timer_id_ = timer_.add(std::chrono::milliseconds(msec_), [this](bool aborted) {
        // Return immediately if the timer was aborted.
        // This function could be invoked during destruction of this object and
        // we don't want to access member variables if not needed.
        if(aborted)
        {
          return;
        }
        std::unique_lock<std::mutex> lk(timeout_mutex_);
        if(child()->status() == NodeStatus::RUNNING)
        {
          child_halted_ = true;
          haltChild();
          emitWakeUpSignal();
        }
      });
    }
  }

  std::unique_lock<std::mutex> lk(timeout_mutex_);

  if(child_halted_)
  {
    timeout_started_ = false;
    return NodeStatus::FAILURE;
  }
  else
  {
    const NodeStatus child_status = child()->executeTick();
    if(isStatusCompleted(child_status))
    {
      timeout_started_ = false;
      timeout_mutex_.unlock();
      timer_.cancel(timer_id_);
      timeout_mutex_.lock();
      resetChild();
    }
    return child_status;
  }
}

void TimeoutNode::halt()
{
  timeout_started_ = false;
  timer_.cancelAll();
  DecoratorNode::halt();
}

}  // namespace BT
