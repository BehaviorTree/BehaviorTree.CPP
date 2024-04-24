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

#include "behaviortree_cpp/decorators/updated_decorator.h"
#include "behaviortree_cpp/bt_factory.h"

namespace BT
{

EntryUpdatedDecorator::EntryUpdatedDecorator(const std::string& name,
                                             const NodeConfig& config,
                                             NodeStatus if_not_updated)
  : DecoratorNode(name, config), if_not_updated_(if_not_updated)
{
  auto it = config.input_ports.find("entry");
  if(it == config.input_ports.end() || it->second.empty())
  {
    throw LogicError("Missing port 'entry' in ", name);
  }
  const auto entry_str = it->second;
  StringView stripped_key;
  if(isBlackboardPointer(entry_str, &stripped_key))
  {
    entry_key_ = stripped_key;
  }
  else
  {
    entry_key_ = entry_str;
  }
}

NodeStatus EntryUpdatedDecorator::tick()
{
  // continue executing an asynchronous child
  if(still_executing_child_)
  {
    auto status = child()->executeTick();
    still_executing_child_ = (status == NodeStatus::RUNNING);
    return status;
  }

  if(auto entry = config().blackboard->getEntry(entry_key_))
  {
    std::unique_lock lk(entry->entry_mutex);
    const uint64_t current_id = entry->sequence_id;
    const uint64_t previous_id = sequence_id_;
    sequence_id_ = current_id;

    if(previous_id == current_id)
    {
      return if_not_updated_;
    }
  }
  else
  {
    return if_not_updated_;
  }

  auto status = child()->executeTick();
  still_executing_child_ = (status == NodeStatus::RUNNING);
  return status;
}

void EntryUpdatedDecorator::halt()
{
  still_executing_child_ = false;
}

}  // namespace BT
