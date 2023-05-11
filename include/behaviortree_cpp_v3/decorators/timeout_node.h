#pragma once

#include "behaviortree_cpp_v3/decorator_node.h"
#include <atomic>
#include "behaviortree_cpp_v3/decorators/timer_queue.h"

namespace BT
{
/**
 * @brief The TimeoutNode will halt() a running child if
 * the latter has been RUNNING for more than a give time.
 * The timeout is in milliseconds and it is passed using the port "msec".
 *
 * If timeout is reached it returns FAILURE.
 *
 * Example:
 *
 * <Timeout msec="5000">
 *    <KeepYourBreath/>
 * </Timeout>
 */
template <typename _Clock = std::chrono::steady_clock,
          typename _Duration = std::chrono::steady_clock::duration>
class TimeoutNode : public DecoratorNode
{
public:
  TimeoutNode(const std::string& name, unsigned milliseconds) :
    DecoratorNode(name, {}),
    child_halted_(false),
    timer_id_(0),
    msec_(milliseconds),
    read_parameter_from_ports_(false),
    timeout_started_(false)
  {
    setRegistrationID("Timeout");
  }

  TimeoutNode(const std::string& name, const NodeConfiguration& config) :
    DecoratorNode(name, config),
    child_halted_(false),
    timer_id_(0),
    msec_(0),
    read_parameter_from_ports_(true),
    timeout_started_(false)
  {}

  ~TimeoutNode() override
  {
    timer_.cancelAll();
  }

  static PortsList providedPorts()
  {
    return {InputPort<unsigned>("msec", "After a certain amount of time, "
                                        "halt() the child if it is still running.")};
  }

private:
  TimerQueue<_Clock, _Duration> timer_;

  virtual BT::NodeStatus tick() override
  {
    if (read_parameter_from_ports_)
    {
      if (!getInput("msec", msec_))
      {
        throw RuntimeError("Missing parameter [msec] in TimeoutNode");
      }
    }

    if (!timeout_started_)
    {
      timeout_started_ = true;
      setStatus(NodeStatus::RUNNING);
      child_halted_ = false;

      if (msec_ > 0)
      {
        timer_id_ = timer_.add(std::chrono::milliseconds(msec_), [this](bool aborted) {
          // Return immediately if the timer was aborted.
          // This function could be invoked during destruction of this object and
          // we don't want to access member variables if not needed.
          if (aborted)
          {
            return;
          }
          std::unique_lock<std::mutex> lk(timeout_mutex_);
          if (child()->status() == NodeStatus::RUNNING)
          {
            child_halted_ = true;
            haltChild();
            emitStateChanged();
          }
        });
      }
    }

    std::unique_lock<std::mutex> lk(timeout_mutex_);

    if (child_halted_)
    {
      timeout_started_ = false;
      return NodeStatus::FAILURE;
    }
    else
    {
      const NodeStatus child_status = child()->executeTick();
      if(StatusCompleted(child_status))
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

  std::atomic<bool> child_halted_;
  uint64_t timer_id_;

  unsigned msec_;
  bool read_parameter_from_ports_;
  bool timeout_started_;
  std::mutex timeout_mutex_;
};
}   // namespace BT
