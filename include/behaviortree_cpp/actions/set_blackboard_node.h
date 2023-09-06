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

#ifndef ACTION_SETBLACKBOARD_NODE_H
#define ACTION_SETBLACKBOARD_NODE_H

#include "behaviortree_cpp/action_node.h"

namespace BT
{
/**
 * @brief The SetBlackboard is action used to store a string
 * into an entry of the Blackboard specified in "output_key".
 *
 * Example usage:
 *
 *  <SetBlackboard value="42" output_key="the_answer" />
 *
 * Will store the string "42" in the entry with key "the_answer".
 */
class SetBlackboard : public SyncActionNode
{
public:
  SetBlackboard(const std::string& name, const NodeConfig& config) :
    SyncActionNode(name, config)
  {
    setRegistrationID("SetBlackboard");
  }

  static PortsList providedPorts()
  {
    return {InputPort("value", "Value to be written int othe output_key"),
            BidirectionalPort("output_key", "Name of the blackboard entry where the "
                                            "value should be written")};
  }

private:
  virtual BT::NodeStatus tick() override
  {
    std::string key, value;
    if (!getInput("output_key", key))
    {
      throw RuntimeError("missing port [output_key]");
    }
    if (!getInput("value", value))
    {
      throw RuntimeError("missing port [value]");
    }
    setOutput("output_key", value);
    return NodeStatus::SUCCESS;
  }
};
}   // namespace BT

#endif
