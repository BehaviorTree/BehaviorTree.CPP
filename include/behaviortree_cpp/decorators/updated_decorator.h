/* Copyright (C) 2024 Davide Faconti -  All Rights Reserved
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
 * @brief The EntryUpdatedDecorator checks the Timestamp in an entry
 * to determine if the value was updated since the last time (true,
 * the first time).
 *
 * If it is, the child will be executed, otherwise [if_not_updated] value is returned.
 */
class EntryUpdatedDecorator : public DecoratorNode
{
public:
  EntryUpdatedDecorator(const std::string& name, const NodeConfig& config,
                        NodeStatus if_not_updated);

  ~EntryUpdatedDecorator() override = default;

  static PortsList providedPorts()
  {
    return { InputPort<BT::Any>("entry", "Entry to check") };
  }

private:
  uint64_t sequence_id_ = 0;
  std::string entry_key_;
  bool still_executing_child_ = false;
  NodeStatus if_not_updated_;

  NodeStatus tick() override;

  void halt() override;
};

}  // namespace BT
