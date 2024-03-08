/* Copyright (C) 2020-2022 Davide Faconti -  All Rights Reserved
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
 * @brief Use a Terminal User Interface (ncurses) to select a certain child manually.
 */
class ManualSelectorNode : public ControlNode
{
public:
  ManualSelectorNode(const std::string& name, const NodeConfig& config);

  virtual ~ManualSelectorNode() override = default;

  virtual void halt() override;

  static PortsList providedPorts()
  {
    return { InputPort<bool>(REPEAT_LAST_SELECTION, false,
                             "If true, execute again the same child that was selected "
                             "the "
                             "last "
                             "time") };
  }

private:
  static constexpr const char* REPEAT_LAST_SELECTION = "repeat_last_selection";

  virtual BT::NodeStatus tick() override;
  int running_child_idx_;
  int previously_executed_idx_;

  enum NumericalStatus
  {
    NUM_SUCCESS = 253,
    NUM_FAILURE = 254,
    NUM_RUNNING = 255,
  };

  NodeStatus selectStatus() const;

  uint8_t selectChild() const;
};

}  // namespace BT
