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

  DelayNode(const std::string& name, const NodeConfig& config);

  ~DelayNode() override
  {
    halt();
  }

  static PortsList providedPorts()
  {
    return { InputPort<unsigned>("delay_msec", "Tick the child after a few "
                                               "milliseconds") };
  }

  void halt() override;

private:
  TimerQueue<> timer_;
  uint64_t timer_id_;

  virtual BT::NodeStatus tick() override;

  bool delay_started_ = false;
  std::atomic_bool delay_complete_ = false;
  bool delay_aborted_ = false;
  unsigned msec_;
  bool read_parameter_from_ports_ = false;
  std::mutex delay_mutex_;
};

}  // namespace BT
