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
    // Join the timer thread first: timer_ is the first member, so it is destroyed
    // after delay_mutex_ and timer_waiting_, which its handler touches.
    timer_.stop();
    halt();
  }

  SleepNode(const SleepNode&) = delete;
  SleepNode& operator=(const SleepNode&) = delete;
  SleepNode(SleepNode&&) = delete;
  SleepNode& operator=(SleepNode&&) = delete;

  NodeStatus onStart() override;

  NodeStatus onRunning() override;

  void onHalted() override;

  static PortsList providedPorts()
  {
    return { InputPort<unsigned>("msec") };
  }

private:
  // Member order is frozen for ABI compatibility: do not reorder.
  TimerQueue<> timer_;
  uint64_t timer_id_ = 0;

  std::atomic_bool timer_waiting_ = false;
  std::mutex delay_mutex_;
};

}  // namespace BT
