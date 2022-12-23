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
 * @brief The SwitchNode is equivalent to a switch statement, where a certain
 * branch (child) is executed according to the value of a blackboard entry.
 *
 * Note that the same behaviour can be achieved with multiple Sequences, Fallbacks and
 * Conditions reading the blackboard, but switch is shorter and more readable.
 *
 * Example usage:
 *

<Switch3 variable="{var}"  case_1="1" case_2="42" case_3="666" >
   <ActionA name="action_when_var_eq_1" />
   <ActionB name="action_when_var_eq_42" />
   <ActionC name="action_when_var_eq_666" />
   <ActionD name="default_action" />
 </Switch3>

When the SwitchNode is executed (Switch3 is a node with 3 cases)
the "variable" will be compared to the cases and execute the correct child
or the default one (last).

 *
 */
template <size_t NUM_CASES>
class SwitchNode : public ControlNode
{
public:
  SwitchNode(const std::string& name, const BT::NodeConfig& config) :
    ControlNode::ControlNode(name, config), running_child_(-1)
  {
    setRegistrationID("Switch");
  }

  virtual ~SwitchNode() override = default;

  void halt() override
  {
    running_child_ = -1;
    ControlNode::halt();
  }

  static PortsList providedPorts()
  {
    PortsList ports;
    ports.insert(BT::InputPort<std::string>("variable"));
    for (unsigned i = 0; i < NUM_CASES; i++)
    {
      char case_str[20];
      sprintf(case_str, "case_%d", i + 1);
      ports.insert(BT::InputPort<std::string>(case_str));
    }
    return ports;
  }

private:
  int running_child_;
  virtual BT::NodeStatus tick() override;
};

template <size_t NUM_CASES>
inline NodeStatus SwitchNode<NUM_CASES>::tick()
{
  if (childrenCount() != NUM_CASES + 1)
  {
    throw LogicError("Wrong number of children in SwitchNode; "
                     "must be (num_cases + default)");
  }

  std::string variable;
  std::string value;
  int match_index = int(NUM_CASES);   // default index;

  if (getInput("variable", variable))   // no variable? jump to default
  {
    // check each case until you find a match
    for (int index = 0; index < int(NUM_CASES); ++index)
    {
      char case_key[20];
      sprintf(case_key, "case_%d", int(index + 1));
      bool found = static_cast<bool>(getInput(case_key, value));

      if (found && variable == value)
      {
        match_index = index;
        break;
      }
    }
  }

  // if another one was running earlier, halt it
  if (running_child_ != -1 && running_child_ != match_index)
  {
    haltChild(running_child_);
  }

  auto& selected_child = children_nodes_[match_index];
  NodeStatus ret = selected_child->executeTick();
  if (ret == NodeStatus::SKIPPED)
  {
    // if the matching child is SKIPPED, should I jump to default or
    // be SKIPPED myself? Going with the former, for the time being.
    running_child_ = -1;
    return NodeStatus::SKIPPED;
  }
  else if (ret == NodeStatus::RUNNING)
  {
    running_child_ = match_index;
  }
  else
  {
    resetChildren();
    running_child_ = -1;
  }
  return ret;
}

}   // namespace BT
