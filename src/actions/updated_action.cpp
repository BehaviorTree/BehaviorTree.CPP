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

#include "behaviortree_cpp/actions/updated_action.h"
#include "behaviortree_cpp/bt_factory.h"

namespace BT
{

EntryUpdatedAction::EntryUpdatedAction(const std::string& name, const NodeConfig& config)
  : SyncActionNode(name, config)
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

NodeStatus EntryUpdatedAction::tick()
{
  if(auto entry = config().blackboard->getEntry(entry_key_))
  {
    std::unique_lock lk(entry->entry_mutex);
    const uint64_t current_id = entry->sequence_id;
    const uint64_t previous_id = sequence_id_;
    sequence_id_ = current_id;
    /*
    uint64_t previous_id = 0;
    auto& previous_id_registry = details::GlobalSequenceRegistry();

    // find the previous id in the registry.
    auto it = previous_id_registry.find(entry.get());
    if(it != previous_id_registry.end())
    {
      previous_id = it->second;
    }
    if(previous_id != current_id)
    {
      previous_id_registry[entry.get()] = current_id;
    }*/
    return (previous_id != current_id) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
  }
  else
  {
    return NodeStatus::FAILURE;
  }
}

}  // namespace BT
