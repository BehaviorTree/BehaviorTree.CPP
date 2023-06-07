#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/utils/safe_any.hpp"
#include "behaviortree_cpp/exceptions.h"
#include "behaviortree_cpp/utils/locked_reference.hpp"

namespace BT
{

/// This type contains a pointer to Any, protected
/// with a locked mutex as long as the object is in scope
using AnyPtrLocked = LockedPtr<Any>;

/**
 * @brief The Blackboard is the mechanism used by BehaviorTrees to exchange
 * typed data.
 */
class Blackboard
{

public:
  typedef std::shared_ptr<Blackboard> Ptr;

protected:
  // This is intentionally protected. Use Blackboard::create instead
  Blackboard(Blackboard::Ptr parent) : parent_bb_(parent)
  {}

public:

  struct Entry
  {
    Any value;
    PortInfo port_info;
    mutable std::mutex entry_mutex;

    Entry(const PortInfo& info) : port_info(info)
    {}

    Entry(Any&& other_any, const PortInfo& info) :
          value(std::move(other_any)), port_info(info)
    {}
  };

  /** Use this static method to create an instance of the BlackBoard
    *   to share among all your NodeTrees.
    */
  static Blackboard::Ptr create(Blackboard::Ptr parent = {})
  {
    return std::shared_ptr<Blackboard>(new Blackboard(parent));
  }

  virtual ~Blackboard() = default;

  [[nodiscard]] const Entry* getEntry(const std::string& key) const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // search first if this port was remapped
    if(!internal_to_external_.empty())
    {
      if (auto parent = parent_bb_.lock())
      {
        auto remapping_it = internal_to_external_.find(key);
        if (remapping_it != internal_to_external_.end())
        {
          return parent->getEntry(remapping_it->second);
        }
      }
    }
    auto it = storage_.find(key);
    return (it == storage_.end()) ? nullptr : it->second.get();
  }

  [[nodiscard]] Entry* getEntry(const std::string& key)
  {
    // "Avoid Duplication in const and Non-const Member Function,"
    // on p. 23, in Item 3 "Use const whenever possible," in Effective C++, 3d ed
    return const_cast<Entry*>( static_cast<const Blackboard &>(*this).getEntry(key));
  }

  [[nodiscard]] AnyPtrLocked getAnyLocked(const std::string& key)
  {
    if(auto entry = getEntry(key))
    {
      return AnyPtrLocked(&entry->value, &entry->entry_mutex);
    }
    return {};
  }

  [[nodiscard]] AnyPtrLocked getAnyLocked(const std::string& key) const
  {
    if(auto entry = getEntry(key))
    {
      return AnyPtrLocked(&entry->value,  const_cast<std::mutex*>(&entry->entry_mutex));
    }
    return {};
  }

  [[deprecated("Use getAnyLocked instead")]]
  const Any* getAny(const std::string& key) const
  {
    return getAnyLocked(key).get();
  }

  [[deprecated("Use getAnyLocked instead")]]
  Any* getAny(const std::string& key)
  {
    return const_cast<Any*>(getAnyLocked(key).get());
  }

  /** Return true if the entry with the given key was found.
   *  Note that this method may throw an exception if the cast to T failed.
   */
  template <typename T> [[nodiscard]]
  bool get(const std::string& key, T& value) const
  {
    if (auto any_ref = getAnyLocked(key))
    {
      value = any_ref.get()->cast<T>();
      return true;
    }
    return false;
  }

  /**
   * Version of get() that throws if it fails.
   */
  template <typename T> [[nodiscard]]
  T get(const std::string& key) const
  {
    if (auto any_ref = getAnyLocked(key))
    {
      return any_ref.get()->cast<T>();
    }
    else
    {
      throw RuntimeError("Blackboard::get() error. Missing key [", key, "]");
    }
  }

  /// Update the entry with the given key
  template <typename T>
  void set(const std::string& key, const T& value)
  {
    std::unique_lock lock(mutex_);

    // search first if this port was remapped.
    // Change the parent_bb_ in that case
    if(!internal_to_external_.empty())
    {
      auto remapping_it = internal_to_external_.find(key);
      if (remapping_it != internal_to_external_.end())
      {
        const auto& remapped_key = remapping_it->second;
        if (auto parent = parent_bb_.lock())
        {
          parent->set(remapped_key, value);
          return;
        }
      }
    }

    // check local storage
    auto it = storage_.find(key);
    if (it == storage_.end())
    {
      // create a new entry
      Any new_value(value);
      PortInfo new_port(PortDirection::INOUT, new_value.type(), {});
      storage_.emplace(key, std::make_unique<Entry>(std::move(new_value), new_port));
    }
    else
    {
      // this is not the first time we set this entry, we need to check
      // if the type is the same or not.
      Entry& entry = *it->second;
      std::scoped_lock lock(entry.entry_mutex);

      Any& previous_any = entry.value;
      const PortInfo& port_info = entry.port_info;

      Any new_value(value);

      // special case: entry exists but it is not strongly typed... yet
      if (!port_info.isStronglyTyped())
      {
        // Use the new type to create a new entry that is strongly typed.
        entry.port_info =
            PortInfo(port_info.direction(), new_value.type(), port_info.converter());
        previous_any = std::move(new_value);
        return;
      }

      std::type_index previous_type = port_info.type();

      // check type mismatch
      if (previous_type != std::type_index(typeid(T)) &&
          previous_type != new_value.type())
      {
        bool mismatching = true;
        if (std::is_constructible<StringView, T>::value)
        {
          Any any_from_string = port_info.parseString(value);
          if (any_from_string.empty() == false)
          {
            mismatching = false;
            new_value = std::move(any_from_string);
          }
        }

        if (mismatching)
        {
          debugMessage();

          throw LogicError("Blackboard::set() failed: once declared, the type of a port "
                           "shall not change. Declared type [",
                           BT::demangle(previous_type), "] != current type [",
                           BT::demangle(typeid(T)), "]");
        }
      }
      previous_any = std::move(new_value);
    }
  }

  void setPortInfo(const std::string &key, const PortInfo& info);

  [[nodiscard]] const PortInfo* portInfo(const std::string& key);

  void addSubtreeRemapping(StringView internal, StringView external);

  void debugMessage() const;

  [[nodiscard]] std::vector<StringView> getKeys(bool include_remapped = true) const;

  void clear()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    storage_.clear();
  }

  [[deprecated("Use getAnyLocked to access safely an Entry")]]
  std::recursive_mutex& entryMutex() const
  {
    return entry_mutex_;
  }

private:
  mutable std::mutex mutex_;
  mutable std::recursive_mutex entry_mutex_;
  std::unordered_map<std::string, std::unique_ptr<Entry>> storage_;
  std::weak_ptr<Blackboard> parent_bb_;
  std::unordered_map<std::string, std::string> internal_to_external_;

};

}   // namespace BT

