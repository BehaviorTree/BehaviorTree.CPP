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

#include <set>
#include "behaviortree_cpp_v3/control_node.h"

namespace BT
{
/**
 * @brief The ParallelNode execute all its children
 * __concurrently__, but not in separate threads!
 *
 * Even if this may look similar to ReactiveSequence,
 * this Control Node is the __only__ one that can have
 * multiple children RUNNING at the same time.
 *
 * The Node is completed either when the THRESHOLD_SUCCESS
 * or THRESHOLD_FAILURE number is reached (both configured using ports).
 *
 * If any of the threaholds is reached, and other children are still running,
 * they will be halted.
 *
 * Note that threshold indexes work as in Python:
 * https://www.i2tutorials.com/what-are-negative-indexes-and-why-are-they-used/
 *
 * Therefore -1 is equivalent to the number of children.
 */
class ParallelNode : public ControlNode
{
public:
  ParallelNode(const std::string& name, int success_threshold, int failure_threshold = 1);

  ParallelNode(const std::string& name, const NodeConfiguration& config);

  static PortsList providedPorts()
  {
    return {InputPort<int>(THRESHOLD_SUCCESS, "number of childen which need to succeed "
                                              "to "
                                              "trigger a SUCCESS"),
            InputPort<int>(THRESHOLD_FAILURE, 1,
                           "number of childen which need to fail to trigger a FAILURE")};
  }

  ~ParallelNode() override = default;

  virtual void halt() override;

  size_t successThreshold() const;
  size_t failureThreshold() const;
  void setSuccessThreshold(int threshold_M);
  void setFailureThreshold(int threshold_M);

private:
  int success_threshold_;
  int failure_threshold_;

  std::set<int> skip_list_;

  bool read_parameter_from_ports_;
  static constexpr const char* THRESHOLD_SUCCESS = "success_threshold";
  static constexpr const char* THRESHOLD_FAILURE = "failure_threshold";

  virtual BT::NodeStatus tick() override;
};

}   // namespace BT
