/*  Contributed by Indraneel on 26/04/2020
*/
#include "behaviortree_cpp_v3/decorators/delay_node.h"
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
DelayNode::DelayNode(const std::string& name, unsigned milliseconds) :
  DecoratorNode(name, {}),
  delay_started_(false),
  delay_aborted_(false),
  msec_(milliseconds),
  read_parameter_from_ports_(false)
{
  setRegistrationID("Delay");
}

DelayNode::DelayNode(const std::string& name, const NodeConfiguration& config) :
  DecoratorNode(name, config),
  delay_started_(false),
  delay_aborted_(false),
  msec_(0),
  read_parameter_from_ports_(true)
{}

NodeStatus DelayNode::tick()
{
  if (read_parameter_from_ports_)
  {
    if (!getInput("delay_msec", msec_))
    {
      throw RuntimeError("Missing parameter [delay_msec] in DelayNode");
    }
  }

  if (!delay_started_)
  {
    delay_complete_ = false;
    delay_aborted_ = false;
    delay_started_ = true;
    setStatus(NodeStatus::RUNNING);

    timer_id_ = timer_.add(std::chrono::milliseconds(msec_), [this](bool aborted) {
      std::unique_lock<std::mutex> lk(delay_mutex_);
      if (!aborted)
      {
        delay_complete_ = true;
      }
      else
      {
        delay_aborted_ = true;
      }
    });
  }

  std::unique_lock<std::mutex> lk(delay_mutex_);

  if (delay_aborted_)
  {
    delay_aborted_ = false;
    delay_started_ = false;
    return NodeStatus::FAILURE;
  }
  else if (delay_complete_)
  {
    delay_started_ = false;
    delay_aborted_ = false;
    auto child_status = child()->executeTick();
    return child_status;
  }
  else
  {
    return NodeStatus::RUNNING;
  }
}

}   // namespace BT
