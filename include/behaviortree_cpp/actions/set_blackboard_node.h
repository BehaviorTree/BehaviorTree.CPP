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
 *
 * Alternatively, you can use it to copy one port inside another port:
 *
 * <SetBlackboard value="{src_port}" output_key="dst_port" />
 *
 * This will copy the type and content of {src_port} into {dst_port}
 */
class SetBlackboardNode : public SyncActionNode
{
public:
  SetBlackboardNode(const std::string& name, const NodeConfig& config)
    : SyncActionNode(name, config)
  {
    setRegistrationID("SetBlackboard");
  }

  static PortsList providedPorts()
  {
    return { InputPort("value", "Value to be written into the output_key"),
             BidirectionalPort("output_key", "Name of the blackboard entry where the "
                                             "value should be written") };
  }

private:
  virtual BT::NodeStatus tick() override
  {
    std::string output_key;
    if(!getInput("output_key", output_key))
    {
      throw RuntimeError("missing port [output_key]");
    }

    const std::string value_str = config().input_ports.at("value");

    StringView stripped_key;
    BT::Any out_value;

    std::shared_ptr<Blackboard::Entry> dst_entry =
        config().blackboard->getEntry(output_key);

    if(isBlackboardPointer(value_str, &stripped_key))
    {
      const auto input_key = std::string(stripped_key);
      std::shared_ptr<Blackboard::Entry> src_entry =
          config().blackboard->getEntry(input_key);

      if(!src_entry)
      {
        throw RuntimeError("Can't find the port referred by [value]");
      }
      if(!dst_entry)
      {
        config().blackboard->createEntry(output_key, src_entry->info);
        dst_entry = config().blackboard->getEntry(output_key);
      }

      out_value = src_entry->value;
    }
    else
    {
      out_value = BT::Any(value_str);
    }

    if(out_value.empty())
      return NodeStatus::FAILURE;

    // avoid type issues when port is remapped: current implementation of the set might be a little bit problematic for initialized on the fly values
    // this still does not attack math issues
    if(dst_entry && dst_entry->info.type() != typeid(std::string) && out_value.isString())
    {
      try
      {
        out_value = dst_entry->info.parseString(out_value.cast<std::string>());
      }
      catch(const std::exception& e)
      {
        throw LogicError("Can't convert string [", out_value.cast<std::string>(),
                         "] to type [", BT::demangle(dst_entry->info.type()),
                         "]: ", e.what());
      }
    }

    config().blackboard->set(output_key, out_value);

    return NodeStatus::SUCCESS;
  }
};
}  // namespace BT

#endif
