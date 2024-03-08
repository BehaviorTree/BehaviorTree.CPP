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
 * @brief IfThenElseNode must have exactly 2 or 3 children. This node is NOT reactive.
 *
 * The first child is the "statement" of the if.
 *
 * If that return SUCCESS, then the second child is executed.
 *
 * Instead, if it returned FAILURE, the third child is executed.
 *
 * If you have only 2 children, this node will return FAILURE whenever the
 * statement returns FAILURE.
 *
 * This is equivalent to add AlwaysFailure as 3rd child.
 *
 */
class IfThenElseNode : public ControlNode
{
public:
  IfThenElseNode(const std::string& name);

  virtual ~IfThenElseNode() override = default;

  virtual void halt() override;

private:
  size_t child_idx_;

  virtual BT::NodeStatus tick() override;
};

}  // namespace BT
