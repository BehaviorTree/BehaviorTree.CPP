/* Copyright (C) 2023 Davide Faconti -  All Rights Reserved
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
#include "behaviortree_cpp/control_node.h"

namespace BT
{
/**
 * @brief The ParallelAllNode execute all its children
 * __concurrently__, but not in separate threads!
 *
 * It differs in the way ParallelNode works because the latter may stop
 * and halt other children if a certain number of SUCCESS/FAILURES is reached,
 * whilst this one will always complete the execution of ALL its children.
 *
 * Note that threshold indexes work as in Python:
 * https://www.i2tutorials.com/what-are-negative-indexes-and-why-are-they-used/
 *
 * Therefore -1 is equivalent to the number of children.
 */
class ParallelAllNode : public ControlNode
{
public:
  ParallelAllNode(const std::string& name, const NodeConfig& config);

  static PortsList providedPorts()
  {
    return { InputPort<int>("max_failures", 1,
                            "If the number of children returning FAILURE exceeds this "
                            "value, "
                            "ParallelAll returns FAILURE") };
  }

  ~ParallelAllNode() override = default;

  virtual void halt() override;

  size_t failureThreshold() const;
  void setFailureThreshold(int threshold);

private:
  size_t failure_threshold_;

  std::set<size_t> completed_list_;
  size_t failure_count_ = 0;

  virtual BT::NodeStatus tick() override;
};

}  // namespace BT
