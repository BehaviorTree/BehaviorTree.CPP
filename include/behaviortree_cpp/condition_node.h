/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#ifndef CONDITIONNODE_H
#define CONDITIONNODE_H

#include "leaf_node.h"

namespace BT
{
class ConditionNode : public LeafNode
{
public:
  ConditionNode(const std::string& name, const NodeConfig& config);

  virtual ~ConditionNode() override = default;

  //Do nothing
  virtual void halt() override final
  {
    resetStatus();
  }

  virtual NodeType type() const override final
  {
    return NodeType::CONDITION;
  }
};

/**
 * @brief The SimpleConditionNode provides an easy to use ConditionNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus functionName(void)
 *
 * This avoids the hassle of inheriting from a ActionNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method.
 */
class SimpleConditionNode : public ConditionNode
{
public:
  using TickFunctor = std::function<NodeStatus(TreeNode&)>;

  // You must provide the function to call when tick() is invoked
  SimpleConditionNode(const std::string& name, TickFunctor tick_functor,
                      const NodeConfig& config);

  ~SimpleConditionNode() override = default;

protected:
  virtual NodeStatus tick() override;

  TickFunctor tick_functor_;
};
}  // namespace BT

#endif
