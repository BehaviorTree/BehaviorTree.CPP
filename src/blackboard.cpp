#include "behaviortree_cpp/blackboard.h"

#include "behaviortree_cpp/json_export.h"

#include <tuple>
#include <unordered_set>

namespace BT
{

namespace
{
bool IsPrivateKey(StringView str)
{
  return str.size() >= 1 && str.data()[0] == '_';
}
}  // namespace

void Blackboard::enableAutoRemapping(bool remapping)
{
  autoremapping_ = remapping;
}

AnyPtrLocked Blackboard::getAnyLocked(const std::string& key)
{
  if(auto entry = getEntry(key))
  {
    return AnyPtrLocked(&entry->value, &entry->entry_mutex);
  }
  return {};
}

AnyPtrLocked Blackboard::getAnyLocked(const std::string& key) const
{
  if(auto entry = getEntry(key))
  {
    return AnyPtrLocked(&entry->value, const_cast<std::mutex*>(&entry->entry_mutex));
  }
  return {};
}

const Any* Blackboard::getAny(const std::string& key) const
{
  return getAnyLocked(key).get();
}

Any* Blackboard::getAny(const std::string& key)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<Any*>(getAnyLocked(key).get());
}

const std::shared_ptr<Blackboard::Entry>
Blackboard::getEntry(const std::string& key) const
{
  // special syntax: "@" will always refer to the root BB
  if(StartWith(key, '@'))
  {
    return rootBlackboard()->getEntry(key.substr(1, key.size() - 1));
  }

  {
    const std::shared_lock<std::shared_mutex> storage_lock(storage_mutex_);
    auto it = storage_.find(key);
    if(it != storage_.end())
    {
      return it->second;
    }
  }
  // not found. Try autoremapping
  if(auto parent = parent_bb_.lock())
  {
    auto remap_it = internal_to_external_.find(key);
    if(remap_it != internal_to_external_.cend())
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

std::shared_ptr<Blackboard::Entry> Blackboard::getEntry(const std::string& key)
{
  return static_cast<const Blackboard&>(*this).getEntry(key);
}

const TypeInfo* Blackboard::entryInfo(const std::string& key)
{
  auto entry = getEntry(key);
  return (!entry) ? nullptr : &(entry->info);
}

void Blackboard::addSubtreeRemapping(StringView internal, StringView external)
{
  internal_to_external_.insert(
      { static_cast<std::string>(internal), static_cast<std::string>(external) });
}

void Blackboard::debugMessage() const
{
  // Lock storage_mutex_ (shared) to prevent iterator invalidation from
  // concurrent modifications (BUG-5 fix).
  const std::shared_lock storage_lock(storage_mutex_);
  for(const auto& [key, entry] : storage_)
  {
    auto port_type = entry->info.type();
    if(port_type == typeid(void))
    {
      port_type = entry->value.type();
    }

    std::cout << key << " (" << BT::demangle(port_type) << ")" << std::endl;
  }

  for(const auto& [from, to] : internal_to_external_)
  {
    std::cout << "[" << from << "] remapped to port of parent tree [" << to << "]";
    // Show the type of the remapped entry from the parent. Issue #408.
    if(auto parent = parent_bb_.lock())
    {
      if(auto entry = parent->getEntry(to))
      {
        auto port_type = entry->info.type();
        if(port_type == typeid(void))
        {
          port_type = entry->value.type();
        }
        std::cout << " (" << BT::demangle(port_type) << ")";
      }
    }
    std::cout << std::endl;
  }
}

std::vector<StringView> Blackboard::getKeys() const
{
  // Lock storage_mutex_ (shared) to prevent iterator invalidation and
  // dangling StringView from concurrent modifications (BUG-6 fix).
  const std::shared_lock storage_lock(storage_mutex_);
  if(storage_.empty())
  {
    return {};
  }
  std::vector<StringView> out;
  out.reserve(storage_.size());
  for(const auto& entry_it : storage_)
  {
    out.push_back(entry_it.first);
  }
  return out;
}

void Blackboard::clear()
{
  const std::unique_lock storage_lock(storage_mutex_);
  storage_.clear();
}

void Blackboard::createEntry(const std::string& key, const TypeInfo& info)
{
  if(StartWith(key, '@'))
  {
    if(key.find('@', 1) != std::string::npos)
    {
      throw LogicError("Character '@' used multiple times in the key");
    }
    rootBlackboard()->createEntryImpl(key.substr(1, key.size() - 1), info);
  }
  else
  {
    createEntryImpl(key, info);
  }
}

void Blackboard::cloneInto(Blackboard& dst) const
{
  // We must never hold storage_mutex_ while locking entry_mutex, because
  // the script evaluation path (ExprAssignment::evaluate) does the reverse:
  // entry_mutex → storage_mutex_ (via entryInfo → getEntry).
  //
  // Strategy: collect shared_ptrs under storage_mutex_, release it,
  // then copy entry data under entry_mutex only.

  struct CopyTask
  {
    std::string key;
    std::shared_ptr<Entry> src;
    std::shared_ptr<Entry> dst;  // nullptr when a new entry is needed
  };

  std::vector<CopyTask> tasks;
  std::vector<std::string> keys_to_remove;

  // Step 1: snapshot src/dst entries under both storage_mutex_ locks.
  {
    std::shared_lock lk1(storage_mutex_, std::defer_lock);
    std::unique_lock lk2(dst.storage_mutex_, std::defer_lock);
    std::lock(lk1, lk2);

    std::unordered_set<std::string> dst_keys;
    for(const auto& [key, entry] : dst.storage_)
    {
      dst_keys.insert(key);
    }

    for(const auto& [src_key, src_entry] : storage_)
    {
      dst_keys.erase(src_key);
      auto it = dst.storage_.find(src_key);
      if(it != dst.storage_.end())
      {
        tasks.push_back({ src_key, src_entry, it->second });
      }
      else
      {
        tasks.push_back({ src_key, src_entry, nullptr });
      }
    }

    for(const auto& key : dst_keys)
    {
      keys_to_remove.push_back(key);
    }
  }
  // storage_mutex_ locks released here

  // Step 2: copy entry data under entry_mutex only (BUG-3 fix).
  std::vector<std::pair<std::string, std::shared_ptr<Entry>>> new_entries;

  for(auto& task : tasks)
  {
    if(task.dst)
    {
      // overwrite existing entry
      std::scoped_lock entry_locks(task.src->entry_mutex, task.dst->entry_mutex);
      task.dst->string_converter = task.src->string_converter;
      task.dst->value = task.src->value;
      task.dst->info = task.src->info;
      task.dst->sequence_id++;
      task.dst->stamp = std::chrono::steady_clock::now().time_since_epoch();
    }
    else
    {
      // create new entry from src
      std::scoped_lock src_lock(task.src->entry_mutex);
      auto new_entry = std::make_shared<Entry>(task.src->info);
      new_entry->value = task.src->value;
      new_entry->string_converter = task.src->string_converter;
      new_entries.emplace_back(task.key, std::move(new_entry));
    }
  }

  // Step 3: insert new entries and remove stale ones under dst.storage_mutex_.
  if(!new_entries.empty() || !keys_to_remove.empty())
  {
    const std::unique_lock dst_lock(dst.storage_mutex_);
    for(auto& [key, entry] : new_entries)
    {
      dst.storage_.try_emplace(key, std::move(entry));
    }
    for(const auto& key : keys_to_remove)
    {
      dst.storage_.erase(key);
    }
  }
}

Blackboard::Ptr Blackboard::parent()
{
  if(auto parent = parent_bb_.lock())
  {
    return parent;
  }
  return {};
}

std::shared_ptr<Blackboard::Entry> Blackboard::createEntryImpl(const std::string& key,
                                                               const TypeInfo& info)
{
  const std::unique_lock storage_lock(storage_mutex_);
  // This function might be called recursively, when we do remapping, because we move
  // to the top scope to find already existing  entries

  // search if exists already
  auto storage_it = storage_.find(key);
  if(storage_it != storage_.end())
  {
    const auto& prev_info = storage_it->second->info;
    if(prev_info.type() != info.type() && prev_info.isStronglyTyped() &&
       info.isStronglyTyped())
    {
      auto msg = StrCat("Blackboard entry [", key,
                        "]: once declared, the type of a port"
                        " shall not change. Previously declared type [",
                        BT::demangle(prev_info.type()), "], current type [",
                        BT::demangle(info.type()), "]");

      throw LogicError(msg);
    }
    return storage_it->second;
  }

  // manual remapping first
  auto remapping_it = internal_to_external_.find(key);
  if(remapping_it != internal_to_external_.end())
  {
    const auto& remapped_key = remapping_it->second;
    if(auto parent = parent_bb_.lock())
    {
      return parent->createEntryImpl(remapped_key, info);
    }
    throw RuntimeError("Missing parent blackboard");
  }
  // autoremapping second (excluding private keys)
  if(autoremapping_ && !IsPrivateKey(key))
  {
    if(auto parent = parent_bb_.lock())
    {
      return parent->createEntryImpl(key, info);
    }
    throw RuntimeError("Missing parent blackboard");
  }
  // not remapped, not found. Create locally.

  auto entry = std::make_shared<Entry>(info);
  // even if empty, let's assign to it a default type
  entry->value = Any(info.type());
  storage_.insert({ key, entry });
  return entry;
}

nlohmann::json ExportBlackboardToJSON(const Blackboard& blackboard)
{
  nlohmann::json dest;
  for(auto entry_name : blackboard.getKeys())
  {
    const std::string name(entry_name);
    if(auto any_ref = blackboard.getAnyLocked(name))
    {
      if(auto any_ptr = any_ref.get())
      {
        JsonExporter::get().toJson(*any_ptr, dest[name]);
      }
    }
  }
  return dest;
}

void ImportBlackboardFromJSON(const nlohmann::json& json, Blackboard& blackboard)
{
  for(auto it = json.begin(); it != json.end(); ++it)
  {
    if(auto res = JsonExporter::get().fromJson(it.value()))
    {
      auto entry = blackboard.getEntry(it.key());
      if(!entry)
      {
        blackboard.createEntry(it.key(), res->second);
        entry = blackboard.getEntry(it.key());
      }
      // Lock entry_mutex before writing to prevent data races (BUG-4 fix).
      std::scoped_lock lk(entry->entry_mutex);
      entry->value = res->first;
    }
  }
}

Blackboard* BT::Blackboard::rootBlackboard()
{
  auto bb = static_cast<const Blackboard&>(*this).rootBlackboard();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<Blackboard*>(bb);
}

const Blackboard* BT::Blackboard::rootBlackboard() const
{
  const Blackboard* bb = this;
  Blackboard::Ptr prev = parent_bb_.lock();
  while(prev)
  {
    bb = prev.get();
    prev = bb->parent_bb_.lock();
  }
  return bb;
}

}  // namespace BT
