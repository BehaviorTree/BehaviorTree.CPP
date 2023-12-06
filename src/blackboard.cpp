#include "behaviortree_cpp/blackboard.h"

namespace BT
{

bool IsPrivateKey(StringView str)
{
  return str.size() >= 1 && str.data()[0] == '_';
}

void Blackboard::enableAutoRemapping(bool remapping)
{
  autoremapping_ = remapping;
}

AnyPtrLocked Blackboard::getAnyLocked(const std::string &key)
{
  if(auto entry = getEntry(key))
  {
    return AnyPtrLocked(&entry->value, &entry->entry_mutex);
  }
  return {};
}

AnyPtrLocked Blackboard::getAnyLocked(const std::string &key) const
{
  if(auto entry = getEntry(key))
  {
    return AnyPtrLocked(&entry->value, const_cast<std::mutex*>(&entry->entry_mutex));
  }
  return {};
}

const Any *Blackboard::getAny(const std::string &key) const
{
  return getAnyLocked(key).get();
}

Any *Blackboard::getAny(const std::string &key)
{
  return const_cast<Any*>(getAnyLocked(key).get());
}

const std::shared_ptr<Blackboard::Entry> Blackboard::getEntry(const std::string &key) const
{
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = storage_.find(key);
  if(it != storage_.end()) {
    return it->second;
  }
  // not found. Try autoremapping
  if (auto parent = parent_bb_.lock())
  {
    auto remap_it = internal_to_external_.find(key);
    if (remap_it != internal_to_external_.cend())
    {
      auto const& new_key = remap_it->second;
      return parent->getEntry(new_key);
    }
    if(autoremapping_ && !IsPrivateKey(key))
    {
      return parent->getEntry(key);
    }
  }
  return {};
}


std::shared_ptr<Blackboard::Entry> Blackboard::getEntry(const std::string &key)
{
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = storage_.find(key);
  if(it != storage_.end()) {
    return it->second;
  }

  // not found. Try autoremapping
  if (auto parent = parent_bb_.lock())
  {
    auto remap_it = internal_to_external_.find(key);
    if (remap_it != internal_to_external_.cend())
    {
      auto const& new_key = remap_it->second;
      auto entry = parent->getEntry(new_key);
      if(entry)
      {
        storage_.insert({key, entry});
      }
      return entry;
    }
    if(autoremapping_ && !IsPrivateKey(key))
    {
      auto entry = parent->getEntry(key);
      if(entry)
      {
        storage_.insert({key, entry});
      }
      return entry;
    }
  }
  return {};
}


const TypeInfo* Blackboard::entryInfo(const std::string& key)
{
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = storage_.find(key);
  return (it == storage_.end()) ? nullptr : &(it->second->info);
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
    auto port_type = entry->info.type();
    if (port_type == typeid(void))
    {
      port_type = entry->value.type();
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

void Blackboard::clear()
{
  std::unique_lock<std::mutex> lock(mutex_);
  storage_.clear();
}

std::recursive_mutex &Blackboard::entryMutex() const
{
  return entry_mutex_;
}

void Blackboard::createEntry(const std::string &key, const TypeInfo &info)
{
  createEntryImpl(key, info);
}

std::shared_ptr<Blackboard::Entry>
Blackboard::createEntryImpl(const std::string& key, const TypeInfo& info)
{
  std::unique_lock<std::mutex> lock(mutex_);
  // This function might be called recursively, when we do remapping, because we move
  // to the top scope to find already existing  entries

  // search if exists already
  auto storage_it = storage_.find(key);
  if(storage_it != storage_.end())
  {
    const auto& prev_info = storage_it->second->info;
    if (prev_info.type() != info.type() &&
        prev_info.isStronglyTyped() &&
        info.isStronglyTyped())
    {
      auto msg = StrCat("Blackboard entry [", key, "]: once declared, the type of a port"
                        " shall not change. Previously declared type [",
                        BT::demangle(prev_info.type()),
                        "], current type [",
                        BT::demangle(info.type()), "]");

      throw LogicError(msg);
    }
    return storage_it->second;
  }

  std::shared_ptr<Entry> entry;

  // manual remapping first
  auto remapping_it = internal_to_external_.find(key);
  if (remapping_it != internal_to_external_.end())
  {
    const auto& remapped_key = remapping_it->second;
    if (auto parent = parent_bb_.lock())
    {
      entry = parent->createEntryImpl(remapped_key, info);
    }
  }
  else if(autoremapping_ && !IsPrivateKey(key))
  {
    if (auto parent = parent_bb_.lock())
    {
      entry = parent->createEntryImpl(key, info);
    }
  }
  else // not remapped, not found. Create locally.
  {
    entry = std::make_shared<Entry>(info);
    // even if empty, let's assign to it a default type
    entry->value = Any(info.type());
  }
  storage_.insert( {key, entry} );
  return entry;
}

}   // namespace BT
