#include "behaviortree_cpp/blackboard.h"

namespace BT
{
void Blackboard::setPortInfo(const std::string& key, const PortInfo& info)
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
    if (old_type != info.type())
    {
      throw LogicError("Blackboard::set() failed: once declared, the type of a port "
                       "shall not change. "
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
  for (const auto& [key, entry] : storage_)
  {
    auto port_type = entry.port_info.type();
    if (port_type == typeid(void))
    {
      port_type = entry.value.type();
    }

    std::cout << key << " (" << BT::demangle(port_type) << ")" << std::endl;
  }

  for (const auto& [from, to] : internal_to_external_)
  {
    std::cout << "[" << from << "] remapped to port of parent tree [" << to << "]"
              << std::endl;
    continue;
  }
}

std::vector<StringView> Blackboard::getKeys(bool include_remapped) const
{
  const size_t N = storage_.size() + (include_remapped ? internal_to_external_.size() : 0 );
  if (N == 0)
  {
    return {};
  }
  std::vector<StringView> out;
  out.reserve(N);
  for (const auto& entry_it : storage_)
  {
    out.push_back(entry_it.first);
  }
  if(include_remapped)
  {
    for (const auto& [key, remapped] : internal_to_external_)
    {
      out.push_back(key);
    }
  }
  return out;
}

}   // namespace BT
