/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2022 Davide Faconti, Eurecat -  All Rights Reserved
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

namespace BT
{
/**
 * @brief The RepeatNode is used to execute a child several times, as long
 * as it succeed.
 *
 * To succeed, the child must return SUCCESS N times (port "num_cycles").
 *
 * If the child returns FAILURE, the loop is stopped and this node
 * returns FAILURE.
 *
 * Example:
 *
 * <Repeat num_cycles="3">
 *   <ClapYourHandsOnce/>
 * </Repeat>
 */
class RepeatNode : public DecoratorNode
{
public:
  RepeatNode(const std::string& name, int NTries);

  RepeatNode(const std::string& name, const NodeConfig& config);

  virtual ~RepeatNode() override = default;

  static PortsList providedPorts()
  {
    return { InputPort<int>(NUM_CYCLES, "Repeat a successful child up to N times. "
                                        "Use -1 to create an infinite loop.") };
  }

private:
  int num_cycles_;
  int repeat_count_;

  bool read_parameter_from_ports_;
  static constexpr const char* NUM_CYCLES = "num_cycles";

  virtual NodeStatus tick() override;

  void halt() override;
};

}  // namespace BT
