#pragma once

#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/utils/timer_queue.h"
#include <atomic>

namespace BT
{
/**
 * @brief Sleep for a certain amount of time.
 * Consider also using the decorator <Delay/>
 *
 * <Sleep msec="5000"/>
 */
class SleepNode : public StatefulActionNode
{
public:
  SleepNode(const std::string& name, const NodeConfig& config);

  ~SleepNode() override
  {
    halt();
  }

  NodeStatus onStart() override;

  NodeStatus onRunning() override;

  void onHalted() override;

  static PortsList providedPorts()
  {
    return { InputPort<unsigned>("msec") };
  }

private:
  TimerQueue<> timer_;
  uint64_t timer_id_;

  std::atomic_bool timer_waiting_ = false;
  std::mutex delay_mutex_;
};

}  // namespace BT
