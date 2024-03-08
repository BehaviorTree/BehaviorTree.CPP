/* Copyright (C) 2020 Davide Faconti -  All Rights Reserved
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
 * @brief WhileDoElse must have exactly 2 or 3 children.
 * It is a REACTIVE node of IfThenElseNode.
 *
 * The first child is the "statement" that is executed at each tick
 *
 * If result is SUCCESS, the second child is executed.
 *
 * If result is FAILURE, the third child is executed.
 *
 * If the 2nd or 3d child is RUNNING and the statement changes,
 * the RUNNING child will be stopped before starting the sibling.
 *
 */
class WhileDoElseNode : public ControlNode
{
public:
  WhileDoElseNode(const std::string& name);

  virtual ~WhileDoElseNode() override = default;

  virtual void halt() override;

private:
  virtual BT::NodeStatus tick() override;
};

}  // namespace BT
