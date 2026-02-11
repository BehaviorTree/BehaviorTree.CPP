/* Copyright (C) 2018-2025 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp/control_node.h"

namespace BT
{
/**
 * @brief The TryCatch node executes children 1..N-1 as a Sequence ("try" block).
 *
 * If all children in the try-block succeed, this node returns SUCCESS.
 *
 * If any child in the try-block fails, the last child N is executed as a
 * "catch" (cleanup) action, and this node returns FAILURE regardless of
 * the catch child's result.
 *
 * - If a try-child returns RUNNING, this node returns RUNNING.
 * - If a try-child returns SUCCESS, continue to the next try-child.
 * - If a try-child returns FAILURE, enter catch mode and tick the last child.
 * - If the catch child returns RUNNING, this node returns RUNNING.
 * - When the catch child finishes (SUCCESS or FAILURE), this node returns FAILURE.
 * - SKIPPED try-children are skipped over (not treated as failure).
 *
 * Port "catch_on_halt" (default false): if true, the catch child is also
 * executed when the TryCatch node is halted while the try-block is RUNNING.
 *
 * Requires at least 2 children.
 */
class TryCatchNode : public ControlNode
{
public:
  TryCatchNode(const std::string& name, const NodeConfig& config);

  ~TryCatchNode() override = default;

  TryCatchNode(const TryCatchNode&) = delete;
  TryCatchNode& operator=(const TryCatchNode&) = delete;
  TryCatchNode(TryCatchNode&&) = delete;
  TryCatchNode& operator=(TryCatchNode&&) = delete;

  static PortsList providedPorts()
  {
    return { InputPort<bool>("catch_on_halt", false,
                             "If true, execute the catch child when "
                             "the node is halted during the try-block") };
  }

  void halt() override;

private:
  size_t current_child_idx_ = 0;
  size_t skipped_count_ = 0;
  bool in_catch_ = false;

  BT::NodeStatus tick() override;
};

}  // namespace BT
