#pragma once

#include "behaviortree_cpp_v3/decorator_node.h"
#include <atomic>
#include "timer_queue.h"

namespace BT
{
/**
 * @brief The delay node will introduce a delay and then tick the
 * child returning the status of the child as it is upon completion
 * The delay is in milliseconds and it is passed using the port "delay_msec".
 *
 * During the delay the node changes status to RUNNING
 *
 * Example:
 *
 * <Delay delay_msec="5000">
 *    <KeepYourBreath/>
 * </Delay>
 */
class DelayNode : public DecoratorNode
{
public:
  DelayNode(const std::string& name, unsigned milliseconds);

  DelayNode(const std::string& name, const NodeConfiguration& config);

  ~DelayNode() override
  {
    halt();
  }

  static PortsList providedPorts()
  {
    return {InputPort<unsigned>("delay_msec", "Tick the child after a few milliseconds")};
  }
  void halt() override
  {
    delay_started_ = false;
    timer_.cancelAll();
    DecoratorNode::halt();
  }

private:
  TimerQueue<> timer_;
  uint64_t timer_id_;

  virtual BT::NodeStatus tick() override;

  bool delay_started_;
  bool delay_complete_;
  bool delay_aborted_;
  unsigned msec_;
  bool read_parameter_from_ports_;
  std::mutex delay_mutex_;
};

}   // namespace BT
