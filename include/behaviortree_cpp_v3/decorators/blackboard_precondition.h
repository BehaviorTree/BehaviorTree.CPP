/*  Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#ifndef DECORATOR_BLACKBOARD_PRECONDITION_NODE_H
#define DECORATOR_BLACKBOARD_PRECONDITION_NODE_H

#include "behaviortree_cpp_v3/decorator_node.h"
#include <type_traits>

namespace BT
{
/**
 * This node excute its child only if the value of a given input port
 * is equal to the expected one.
 * If this precondition is met, this node will return the same status of the
 * child, otherwise it will return the value specified in "return_on_mismatch".
 *
 * Example:
 *
 * <BlackboardCheckInt value_A="{the_answer}"
 *                     value_B="42"
 *                     return_on_mismatch="FAILURE" />
 */
template <typename T>
class BlackboardPreconditionNode : public DecoratorNode
{
public:
  BlackboardPreconditionNode(const std::string& name, const NodeConfiguration& config) :
    DecoratorNode(name, config)
  {
    if (std::is_same<T, int>::value)
      setRegistrationID("BlackboardCheckInt");
    else if (std::is_same<T, double>::value)
      setRegistrationID("BlackboardCheckDouble");
    else if (std::is_same<T, std::string>::value)
      setRegistrationID("BlackboardCheckString");
    else if (std::is_same<T, bool>::value)
      setRegistrationID("BlackboardCheckBool");
  }

  virtual ~BlackboardPreconditionNode() override = default;

  static PortsList providedPorts()
  {
    return {InputPort("value_A"), InputPort("value_B"),
            InputPort<NodeStatus>("return_on_mismatch")};
  }

private:
  virtual BT::NodeStatus tick() override;
};

//----------------------------------------------------

template <typename T>
inline bool IsSame(const T& a, const T& b)
{
  return a == b;
}

inline bool IsSame(const double& a, const double& b)
{
  constexpr double EPS = static_cast<double>(std::numeric_limits<float>::epsilon());
  return std::abs(a - b) <= EPS;
}

template <typename T>
inline NodeStatus BlackboardPreconditionNode<T>::tick()
{
  T value_A;
  T value_B;
  NodeStatus default_return_status = NodeStatus::FAILURE;

  setStatus(NodeStatus::RUNNING);

  if (getInput("value_A", value_A) && getInput("value_B", value_B) &&
      IsSame(value_A, value_B))
  {
    return child_node_->executeTick();
  }

  if (child()->status() == NodeStatus::RUNNING)
  {
    haltChild();
  }
  getInput("return_on_mismatch", default_return_status);
  return default_return_status;
}

}   // namespace BT

#endif
