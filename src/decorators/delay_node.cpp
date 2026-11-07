/*  Contributed by Indraneel on 26/04/2020
 */

#include "behaviortree_cpp/decorators/delay_node.h"

#include <memory>

namespace BT
{
DelayNode::DelayNode(const std::string& name, unsigned milliseconds)
  : DecoratorNode(name, {})
  , timer_id_(0)
  , msec_(milliseconds)
  , read_parameter_from_ports_(false)
{
  setRegistrationID("Delay");
}

DelayNode::DelayNode(const std::string& name, const NodeConfig& config)
  : DecoratorNode(name, config), timer_id_(0), msec_(0), read_parameter_from_ports_(true)
{}

void DelayNode::halt()
{
  {
    const std::unique_lock<std::mutex> lk(delay_mutex_);
    delay_started_ = false;
    timer_.cancelAll();
    // a handler that the worker thread already picked up must not complete
    // the delay
    timer_id_ = 0;
  }
  DecoratorNode::halt();
}

NodeStatus DelayNode::tick()
{
  if(read_parameter_from_ports_)
  {
    if(!getInput("delay_msec", msec_))
    {
      throw RuntimeError("Missing parameter [delay_msec] in DelayNode");
    }
  }

  std::unique_lock<std::mutex> lk(delay_mutex_);

  if(!delay_started_)
  {
    delay_complete_ = false;
    delay_aborted_ = false;
    delay_started_ = true;
    setStatus(NodeStatus::RUNNING);

    // TimerQueue::cancel() cannot stop a handler that the worker thread
    // already picked up: after a halt() and a restart, the handler of the
    // previous timer may still run, with aborted == false. Each handler
    // therefore checks that it belongs to the current timer. Holding
    // delay_mutex_ across add() and the assignment of timer_id_ means the
    // handler, which locks it too, can only compare after both are set.
    auto id = std::make_shared<uint64_t>(0);
    timer_id_ = timer_.add(std::chrono::milliseconds(msec_), [this, id](bool aborted) {
      const std::unique_lock<std::mutex> lk(delay_mutex_);
      if(aborted || *id != timer_id_)
      {
        return;
      }
      delay_complete_ = true;
      emitWakeUpSignal();
    });
    *id = timer_id_;
  }

  if(delay_aborted_)
  {
    delay_aborted_ = false;
    delay_started_ = false;
    return NodeStatus::FAILURE;
  }
  if(delay_complete_)
  {
    const NodeStatus child_status = child()->executeTick();
    if(isStatusCompleted(child_status))
    {
      delay_started_ = false;
      delay_aborted_ = false;
      resetChild();
    }
    return child_status;
  }
  return NodeStatus::RUNNING;
}

}  // namespace BT
