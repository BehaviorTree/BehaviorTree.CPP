#include "behaviortree_cpp_v3/blackboard.h"

namespace BT
{
void Blackboard::setPortInfo(std::string key, const PortInfo& info)
{
  std::unique_lock<std::mutex> lock(mutex_);

  if (auto parent = parent_bb_.lock())
  {
    auto remapping_it = internal_to_external_.find(key);
    if (remapping_it != internal_to_external_.end())
    {
      parent->setPortInfo(remapping_it->second, info);
      return;
    }
  }

  auto it = storage_.find(key);
  if (it == storage_.end())
  {
    storage_.emplace(key, Entry(info));
  }
  else
  {
    auto old_type = it->second.port_info.type();
    if (old_type && old_type != info.type())
    {
      throw LogicError("Blackboard::set() failed: once declared, the type of a "
                       "port shall "
                       "not change. "
                       "Declared type [",
                       BT::demangle(old_type), "] != current type [",
                       BT::demangle(info.type()), "]");
    }
  }
}

const PortInfo* Blackboard::portInfo(const std::string& key)
{
  std::unique_lock<std::mutex> lock(mutex_);

  if (auto parent = parent_bb_.lock())
  {
    auto remapping_it = internal_to_external_.find(key);
    if (remapping_it != internal_to_external_.end())
    {
      return parent->portInfo(remapping_it->second);
    }
  }

  auto it = storage_.find(key);
  if (it == storage_.end())
  {
    return nullptr;
  }
  return &(it->second.port_info);
}

void Blackboard::addSubtreeRemapping(StringView internal, StringView external)
{
  internal_to_external_.insert(
      {static_cast<std::string>(internal), static_cast<std::string>(external)});
}

void Blackboard::debugMessage() const
{
  for (const auto& entry_it : storage_)
  {
    auto port_type = entry_it.second.port_info.type();
    if (!port_type)
    {
      port_type = &(entry_it.second.value.type());
    }

    std::cout << entry_it.first << " (" << demangle(port_type) << ") -> ";

    if (auto parent = parent_bb_.lock())
    {
      auto remapping_it = internal_to_external_.find(entry_it.first);
      if (remapping_it != internal_to_external_.end())
      {
        std::cout << "remapped to parent [" << remapping_it->second << "]" << std::endl;
        continue;
      }
    }
    std::cout << ((entry_it.second.value.empty()) ? "empty" : "full") << std::endl;
  }
}

std::vector<StringView> Blackboard::getKeys() const
{
  if (storage_.empty())
  {
    return {};
  }
  std::vector<StringView> out;
  out.reserve(storage_.size());
  for (const auto& entry_it : storage_)
  {
    out.push_back(entry_it.first);
  }
  return out;
}

}   // namespace BT
