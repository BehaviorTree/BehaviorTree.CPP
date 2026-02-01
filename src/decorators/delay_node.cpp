/*  Contributed by Indraneel on 26/04/2020
 */

#include "behaviortree_cpp/decorators/delay_node.h"

namespace BT
{
DelayNode::DelayNode(const std::string& name, unsigned milliseconds)
  : DecoratorNode(name, {}), timer_id_(0), msec_(milliseconds)
{
  setRegistrationID("Delay");
}

DelayNode::DelayNode(const std::string& name, const NodeConfig& config)
  : DecoratorNode(name, config), timer_id_(0), msec_(0)
{}

void DelayNode::halt()
{
  delay_started_ = false;
  timer_.cancelAll();
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

  if(!delay_started_)
  {
    delay_complete_ = false;
    delay_aborted_ = false;
    delay_started_ = true;
    setStatus(NodeStatus::RUNNING);

    timer_id_ = timer_.add(std::chrono::milliseconds(msec_), [this](bool aborted) {
      const std::unique_lock<std::mutex> lk(delay_mutex_);
      delay_complete_ = (!aborted);
      if(!aborted)
      {
        emitWakeUpSignal();
      }
    });
  }

  const std::unique_lock<std::mutex> lk(delay_mutex_);

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
