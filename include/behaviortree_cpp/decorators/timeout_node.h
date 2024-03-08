/*  Copyright (C) 2018-2023 Davide Faconti -  All Rights Reserved
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

#pragma once

#include "behaviortree_cpp/decorator_node.h"
#include "behaviortree_cpp/utils/timer_queue.h"
#include <atomic>

namespace BT
{
/**
 * @brief The TimeoutNode will halt() a running child if
 * the latter has been RUNNING longer than a given time.
 * The timeout is in milliseconds and it is passed using the port "msec".
 *
 * If timeout is reached, the node returns FAILURE.
 *
 * Example:
 *
 * <Timeout msec="5000">
 *    <KeepYourBreath/>
 * </Timeout>
 */

class TimeoutNode : public DecoratorNode
{
public:
  TimeoutNode(const std::string& name, unsigned milliseconds)
    : DecoratorNode(name, {})
    , child_halted_(false)
    , timer_id_(0)
    , msec_(milliseconds)
    , read_parameter_from_ports_(false)
    , timeout_started_(false)
  {
    setRegistrationID("Timeout");
  }

  TimeoutNode(const std::string& name, const NodeConfig& config)
    : DecoratorNode(name, config)
    , child_halted_(false)
    , timer_id_(0)
    , msec_(0)
    , read_parameter_from_ports_(true)
    , timeout_started_(false)
  {}

  ~TimeoutNode() override
  {
    timer_.cancelAll();
  }

  static PortsList providedPorts()
  {
    return { InputPort<unsigned>("msec", "After a certain amount of time, "
                                         "halt() the child if it is still running.") };
  }

private:
  virtual BT::NodeStatus tick() override;

  void halt() override;

  TimerQueue<> timer_;
  std::atomic_bool child_halted_ = false;
  uint64_t timer_id_;

  unsigned msec_;
  bool read_parameter_from_ports_;
  std::atomic_bool timeout_started_ = false;
  std::mutex timeout_mutex_;
};

}  // namespace BT
