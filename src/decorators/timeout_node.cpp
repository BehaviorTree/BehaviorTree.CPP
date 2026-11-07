/*  Copyright (C) 2018-2025 Davide Faconti -  All Rights Reserved
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

#include <memory>

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

  std::unique_lock<std::mutex> lk(timeout_mutex_);

  if(!timeout_started_)
  {
    timeout_started_ = true;
    setStatus(NodeStatus::RUNNING);
    child_halted_ = false;

    if(msec_ > 0)
    {
      // TimerQueue::cancel() cannot stop a handler that the worker thread
      // already picked up: after the child completed (or a halt()) and a
      // restart, the handler of the previous timer may still run, with
      // aborted == false. Each handler therefore checks that it belongs to
      // the current timer. Holding timeout_mutex_ across add() and the
      // assignment of timer_id_ means the handler, which locks it too, can
      // only compare after both are set.
      auto id = std::make_shared<uint64_t>(0);
      timer_id_ = timer_.add(std::chrono::milliseconds(msec_), [this, id](bool aborted) {
        // Return immediately if the timer was aborted.
        // This function could be invoked during destruction of this object and
        // we don't want to access member variables if not needed.
        if(aborted)
        {
          return;
        }
        const std::unique_lock<std::mutex> lk(timeout_mutex_);
        // halt() may have run between the timer firing and this lock: the
        // child was already halted and may not exist anymore.
        if(!timeout_started_ || *id != timer_id_)
        {
          return;
        }
        if(child()->status() == NodeStatus::RUNNING)
        {
          child_halted_ = true;
          haltChild();
          emitWakeUpSignal();
        }
      });
      *id = timer_id_;
    }
  }

  if(child_halted_)
  {
    timeout_started_ = false;
    return NodeStatus::FAILURE;
  }
  const NodeStatus child_status = child()->executeTick();
  if(isStatusCompleted(child_status))
  {
    timeout_started_ = false;
    timer_.cancel(timer_id_);
    timer_id_ = 0;
    resetChild();
  }
  return child_status;
}

void TimeoutNode::halt()
{
  {
    // The timer handler halts the child on the worker thread while holding
    // this mutex: wait for it, and make a handler that fires later return
    // before it touches the child.
    const std::unique_lock<std::mutex> lk(timeout_mutex_);
    timeout_started_ = false;
    timer_.cancelAll();
    timer_id_ = 0;
  }
  DecoratorNode::halt();
}

}  // namespace BT
