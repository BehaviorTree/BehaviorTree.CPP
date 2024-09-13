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
 * @brief The RetryNode is used to execute a child several times if it fails.
 *
 * If the child returns SUCCESS, the loop is stopped and this node
 * returns SUCCESS.
 *
 * If the child returns FAILURE, this node will try again up to N times
 * (N is read from port "num_attempts").
 *
 * Example:
 *
 * <RetryUntilSuccessful num_attempts="3">
 *     <OpenDoor/>
 * </RetryUntilSuccessful>
 *
 * Note:
 * RetryNodeTypo is only included to support the deprecated typo
 * "RetryUntilSuccesful" (note the single 's' in Succesful)
 */
class RetryNode : public DecoratorNode
{
public:
  RetryNode(const std::string& name, int NTries);

  RetryNode(const std::string& name, const NodeConfig& config);

  virtual ~RetryNode() override = default;

  static PortsList providedPorts()
  {
    return { InputPort<int>(NUM_ATTEMPTS, "Execute again a failing child up to N times. "
                                          "Use -1 to create an infinite loop.") };
  }

  virtual void halt() override;

private:
  int max_attempts_;
  int try_count_;

  bool read_parameter_from_ports_;
  static constexpr const char* NUM_ATTEMPTS = "num_attempts";

  virtual BT::NodeStatus tick() override;
};

class [[deprecated("RetryUntilSuccesful was a typo and deprecated, use "
                   "RetryUntilSuccessful "
                   "instead.")]] RetryNodeTypo : public RetryNode
{
public:
  RetryNodeTypo(const std::string& name, int NTries) : RetryNode(name, NTries){};

  RetryNodeTypo(const std::string& name, const NodeConfig& config)
    : RetryNode(name, config){};

  virtual ~RetryNodeTypo() override = default;
};

}  // namespace BT
