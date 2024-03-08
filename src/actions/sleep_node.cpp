#include "behaviortree_cpp/actions/sleep_node.h"

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

  timer_waiting_ = true;

  timer_id_ = timer_.add(std::chrono::milliseconds(msec), [this](bool aborted) {
    std::unique_lock<std::mutex> lk(delay_mutex_);
    if(!aborted)
    {
      emitWakeUpSignal();
    }
    timer_waiting_ = false;
  });

  return NodeStatus::RUNNING;
}

NodeStatus SleepNode::onRunning()
{
  return timer_waiting_ ? NodeStatus::RUNNING : NodeStatus::SUCCESS;
}

void SleepNode::onHalted()
{
  timer_waiting_ = false;
  timer_.cancel(timer_id_);
}

}  // namespace BT
