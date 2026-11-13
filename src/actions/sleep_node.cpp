#include "behaviortree_cpp/actions/sleep_node.h"

#include <memory>

namespace BT
{

SleepNode::SleepNode(const std::string& name, const NodeConfig& config)
  : StatefulActionNode(name, config), timer_waiting_(false)
{}

NodeStatus SleepNode::onStart()
{
  unsigned msec = 0;
  if(!getInput("msec", msec))
  {
    throw RuntimeError("Missing parameter [msec] in SleepNode");
  }

  if(msec <= 0)
  {
    return NodeStatus::SUCCESS;
  }

  setStatus(NodeStatus::RUNNING);

  // TimerQueue::cancel() cannot stop a handler that the worker thread already
  // picked up: after a halt() and a restart, the handler of the previous timer
  // may still run, with aborted == false. Each handler therefore checks that it
  // belongs to the current timer. Holding delay_mutex_ across add() and the
  // assignment of timer_id_ means the handler, which locks it too, can only
  // compare after both its own id and timer_id_ are set.
  auto id = std::make_shared<uint64_t>(0);
  const std::unique_lock<std::mutex> lk(delay_mutex_);
  timer_waiting_ = true;
  timer_id_ = timer_.add(std::chrono::milliseconds(msec), [this, id](bool aborted) {
    if(aborted)
    {
      return;
    }
    const std::unique_lock<std::mutex> lk(delay_mutex_);
    if(*id != timer_id_)
    {
      return;
    }
    timer_waiting_ = false;
    emitWakeUpSignal();
  });
  *id = timer_id_;

  return NodeStatus::RUNNING;
}

NodeStatus SleepNode::onRunning()
{
  return timer_waiting_ ? NodeStatus::RUNNING : NodeStatus::SUCCESS;
}

void SleepNode::onHalted()
{
  const std::unique_lock<std::mutex> lk(delay_mutex_);
  timer_waiting_ = false;
  timer_.cancel(timer_id_);
  // a handler that the worker thread already picked up must not wake us up
  timer_id_ = 0;
}

}  // namespace BT
