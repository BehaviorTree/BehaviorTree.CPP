#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <iostream>
#include <string>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include "behaviortree_cpp_v3/basic_types.h"
#include "behaviortree_cpp_v3/utils/safe_any.hpp"
#include "behaviortree_cpp_v3/exceptions.h"

namespace BT
{
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
  /** Use this static method to create an instance of the BlackBoard
    *   to share among all your NodeTrees.
    */
  static Blackboard::Ptr create(Blackboard::Ptr parent = {})
  {
    return std::shared_ptr<Blackboard>(new Blackboard(parent));
  }

  virtual ~Blackboard() = default;

  /**
     * @brief The method getAny allow the user to access directly the type
     * erased value.
     *
     * @return the pointer or nullptr if it fails.
     */
  const Any* getAny(const std::string& key) const
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // search first if this port was remapped
    if (auto parent = parent_bb_.lock())
    {
      auto remapping_it = internal_to_external_.find(key);
      if (remapping_it != internal_to_external_.end())
      {
        return parent->getAny(remapping_it->second);
      }
    }
    auto it = storage_.find(key);
    return (it == storage_.end()) ? nullptr : &(it->second.value);
  }

  Any* getAny(const std::string& key)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // search first if this port was remapped
    if (auto parent = parent_bb_.lock())
    {
      auto remapping_it = internal_to_external_.find(key);
      if (remapping_it != internal_to_external_.end())
      {
        return parent->getAny(remapping_it->second);
      }
    }
    auto it = storage_.find(key);
    return (it == storage_.end()) ? nullptr : &(it->second.value);
  }

  /** Return true if the entry with the given key was found.
     *  Note that this method may throw an exception if the cast to T failed.
     */
  template <typename T>
  bool get(const std::string& key, T& value) const
  {
    const Any* val = getAny(key);
    if (val)
    {
      value = val->cast<T>();
    }
    return (bool)val;
  }

  /**
     * Version of get() that throws if it fails.
    */
  template <typename T>
  T get(const std::string& key) const
  {
    const Any* val = getAny(key);
    if (val)
    {
      return val->cast<T>();
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
    std::unique_lock<std::mutex> lock_entry(entry_mutex_);
    std::unique_lock<std::mutex> lock(mutex_);

    // search first if this port was remapped.
    // Change the parent_bb_ in that case
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

    // check local storage
    auto it = storage_.find(key);
    if (it != storage_.end())
    {
      const PortInfo& port_info = it->second.port_info;
      auto& previous_any = it->second.value;
      const auto previous_type = port_info.type();

      Any new_value(value);

      if (previous_type && *previous_type != typeid(T) &&
          *previous_type != new_value.type())
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
                           "shall not change. "
                           "Declared type [",
                           BT::demangle(previous_type), "] != current type [",
                           BT::demangle(typeid(T)), "]");
        }
      }
      previous_any = std::move(new_value);
    }
    else
    {   // create for the first time without any info
      storage_.emplace(key, Entry(Any(value), PortInfo()));
    }
    return;
  }

  void setPortInfo(std::string key, const PortInfo& info);

  const PortInfo* portInfo(const std::string& key);

  void addSubtreeRemapping(StringView internal, StringView external);

  void debugMessage() const;

  std::vector<StringView> getKeys() const;

  void clear()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    storage_.clear();
    internal_to_external_.clear();
  }

  // Lock this mutex before using get() and getAny() and unlock it while you have
  // done using the value.
  std::mutex& entryMutex()
  {
    return entry_mutex_;
  }

private:
  struct Entry
  {
    Any value;
    const PortInfo port_info;

    Entry(const PortInfo& info) : port_info(info)
    {}

    Entry(Any&& other_any, const PortInfo& info) :
      value(std::move(other_any)), port_info(info)
    {}
  };

  mutable std::mutex mutex_;
  mutable std::mutex entry_mutex_;
  std::unordered_map<std::string, Entry> storage_;
  std::weak_ptr<Blackboard> parent_bb_;
  std::unordered_map<std::string, std::string> internal_to_external_;
};

}   // namespace BT

#endif   // BLACKBOARD_H
