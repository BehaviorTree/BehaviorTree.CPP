/* Copyright (C) 2022-2025 Davide Faconti -  All Rights Reserved
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

#pragma once

#include "behaviortree_cpp/contrib/any.hpp"
#include "behaviortree_cpp/contrib/expected.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <typeindex>

namespace BT
{

/**
 * @brief Registry for polymorphic shared_ptr cast relationships.
 *
 * This enables passing shared_ptr<Derived> to ports expecting shared_ptr<Base>
 * without breaking ABI compatibility. Users register inheritance relationships
 * at runtime, and the registry handles upcasting/downcasting transparently.
 *
 * This class is typically owned by BehaviorTreeFactory and passed to Blackboard
 * during tree creation. This avoids global state and makes testing easier.
 *
 * Usage with BehaviorTreeFactory:
 *   BehaviorTreeFactory factory;
 *   factory.registerPolymorphicCast<Cat, Animal>();
 *   factory.registerPolymorphicCast<Sphynx, Cat>();
 *   auto tree = factory.createTreeFromText(xml);
 */
class PolymorphicCastRegistry
{
public:
  using CastFunction = std::function<linb::any(const linb::any&)>;

  PolymorphicCastRegistry() = default;
  ~PolymorphicCastRegistry() = default;

  // Non-copyable, non-movable (contains mutex)
  PolymorphicCastRegistry(const PolymorphicCastRegistry&) = delete;
  PolymorphicCastRegistry& operator=(const PolymorphicCastRegistry&) = delete;
  PolymorphicCastRegistry(PolymorphicCastRegistry&&) = delete;
  PolymorphicCastRegistry& operator=(PolymorphicCastRegistry&&) = delete;

  /**
   * @brief Register a Derived -> Base inheritance relationship.
   *
   * This enables:
   * - Upcasting: shared_ptr<Derived> can be retrieved as shared_ptr<Base>
   * - Downcasting: shared_ptr<Base> can be retrieved as shared_ptr<Derived>
   *   (via dynamic_pointer_cast, may return nullptr if types don't match)
   *
   * @tparam Derived The derived class (must inherit from Base)
   * @tparam Base The base class (must be polymorphic - have virtual functions)
   */
  template <typename Derived, typename Base>
  void registerCast()
  {
    static_assert(std::is_base_of_v<Base, Derived>, "Derived must inherit from Base");
    static_assert(std::is_polymorphic_v<Base>, "Base must be polymorphic (have virtual "
                                               "functions)");

    std::unique_lock<std::shared_mutex> lock(mutex_);

    // Register upcast: Derived -> Base
    auto upcast_key = std::make_pair(std::type_index(typeid(std::shared_ptr<Derived>)),
                                     std::type_index(typeid(std::shared_ptr<Base>)));

    upcasts_[upcast_key] = [](const linb::any& from) -> linb::any {
      auto ptr = linb::any_cast<std::shared_ptr<Derived>>(from);
      return std::static_pointer_cast<Base>(ptr);
    };

    // Register downcast: Base -> Derived (uses dynamic_pointer_cast)
    auto downcast_key = std::make_pair(std::type_index(typeid(std::shared_ptr<Base>)),
                                       std::type_index(typeid(std::shared_ptr<Derived>)));

    downcasts_[downcast_key] = [](const linb::any& from) -> linb::any {
      auto ptr = linb::any_cast<std::shared_ptr<Base>>(from);
      auto derived = std::dynamic_pointer_cast<Derived>(ptr);
      if(!derived)
      {
        throw std::bad_cast();
      }
      return derived;
    };

    // Track inheritance relationship for port compatibility checks
    base_types_[std::type_index(typeid(std::shared_ptr<Derived>))].insert(
        std::type_index(typeid(std::shared_ptr<Base>)));
  }

  /**
   * @brief Check if from_type can be converted to to_type.
   *
   * Returns true if:
   * - from_type == to_type
   * - from_type is a registered derived type of to_type (upcast)
   * - to_type is a registered derived type of from_type (downcast)
   */
  [[nodiscard]] bool isConvertible(std::type_index from_type,
                                   std::type_index to_type) const
  {
    if(from_type == to_type)
    {
      return true;
    }

    std::shared_lock<std::shared_mutex> lock(mutex_);

    // Check direct upcast
    auto upcast_key = std::make_pair(from_type, to_type);
    if(upcasts_.find(upcast_key) != upcasts_.end())
    {
      return true;
    }

    // Check transitive upcast (e.g., Sphynx -> Cat -> Animal)
    if(canUpcastTransitive(from_type, to_type))
    {
      return true;
    }

    // Check downcast
    auto downcast_key = std::make_pair(from_type, to_type);
    if(downcasts_.find(downcast_key) != downcasts_.end())
    {
      return true;
    }

    return false;
  }

  /**
   * @brief Check if from_type can be UPCAST to to_type (not downcast).
   *
   * This is stricter than isConvertible - only allows going from
   * derived to base, not the reverse.
   */
  [[nodiscard]] bool canUpcast(std::type_index from_type, std::type_index to_type) const
  {
    if(from_type == to_type)
    {
      return true;
    }

    std::shared_lock<std::shared_mutex> lock(mutex_);
    return canUpcastTransitive(from_type, to_type);
  }

  /**
   * @brief Attempt to cast the value to the target type.
   *
   * @param from The source any containing a shared_ptr
   * @param from_type The type_index of the stored type
   * @param to_type The target type_index
   * @return The casted any on success, or an error string on failure
   */
  [[nodiscard]] nonstd::expected<linb::any, std::string>
  tryCast(const linb::any& from, std::type_index from_type, std::type_index to_type) const
  {
    if(from_type == to_type)
    {
      return from;
    }

    std::shared_lock<std::shared_mutex> lock(mutex_);

    // Try direct upcast
    auto upcast_key = std::make_pair(from_type, to_type);
    auto upcast_it = upcasts_.find(upcast_key);
    if(upcast_it != upcasts_.end())
    {
      try
      {
        return upcast_it->second(from);
      }
      catch(const std::exception& e)
      {
        return nonstd::make_unexpected(std::string("Direct upcast failed: ") + e.what());
      }
    }

    // Try transitive upcast
    auto transitive_up = applyTransitiveCasts(from, from_type, to_type, upcasts_, true);
    if(transitive_up)
    {
      return transitive_up;
    }

    // Try direct downcast
    auto downcast_key = std::make_pair(from_type, to_type);
    auto downcast_it = downcasts_.find(downcast_key);
    if(downcast_it != downcasts_.end())
    {
      try
      {
        return downcast_it->second(from);
      }
      catch(const std::exception& e)
      {
        return nonstd::make_unexpected(std::string("Downcast failed "
                                                   "(dynamic_pointer_cast returned "
                                                   "null): ") +
                                       e.what());
      }
    }

    // Try transitive downcast
    auto transitive_down =
        applyTransitiveCasts(from, to_type, from_type, downcasts_, false);
    if(transitive_down)
    {
      return transitive_down;
    }

    return nonstd::make_unexpected(std::string("No registered polymorphic conversion "
                                               "available"));
  }

  /**
   * @brief Get all registered base types for a given type.
   */
  [[nodiscard]] std::set<std::type_index> getBaseTypes(std::type_index type) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = base_types_.find(type);
    if(it != base_types_.end())
    {
      return it->second;
    }
    return {};
  }

  /**
   * @brief Clear all registrations (mainly for testing).
   */
  void clear()
  {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    upcasts_.clear();
    downcasts_.clear();
    base_types_.clear();
  }

private:
  // Check if we can upcast from_type to to_type through a chain of registered casts
  [[nodiscard]] bool canUpcastTransitive(std::type_index from_type,
                                         std::type_index to_type) const
  {
    // Depth-first search to find a path from from_type to to_type
    std::set<std::type_index> visited;
    std::vector<std::type_index> queue;
    queue.push_back(from_type);

    while(!queue.empty())
    {
      auto current = queue.back();
      queue.pop_back();

      if(visited.count(current) != 0)
      {
        continue;
      }
      visited.insert(current);

      auto it = base_types_.find(current);
      if(it == base_types_.end())
      {
        continue;
      }

      for(const auto& base : it->second)
      {
        if(base == to_type)
        {
          return true;
        }
        queue.push_back(base);
      }
    }
    return false;
  }

  // Common helper for transitive upcast and downcast.
  //
  // Performs depth-first search from dfs_start through base_types_ edges,
  // looking for dfs_target. When found, reconstructs the path from dfs_target
  // back to dfs_start. If reverse_path is true, reverses it so casts are applied
  // in [dfs_start -> dfs_target] order; otherwise applies in traced order.
  //
  // For upcast:  dfs_start=from_type, dfs_target=to_type, reverse=true,  map=upcasts_
  // For downcast: dfs_start=to_type, dfs_target=from_type, reverse=false, map=downcasts_
  using CastMap = std::map<std::pair<std::type_index, std::type_index>, CastFunction>;

  [[nodiscard]] nonstd::expected<linb::any, std::string> applyTransitiveCasts(
      const linb::any& from, std::type_index dfs_start, std::type_index dfs_target,
      const CastMap& cast_map, bool reverse_path) const
  {
    // Note: std::type_index has no default constructor, so we can't use operator[]
    std::map<std::type_index, std::type_index> parent;
    std::vector<std::type_index> stack;
    stack.push_back(dfs_start);
    parent.insert({ dfs_start, dfs_start });

    while(!stack.empty())
    {
      auto current = stack.back();
      stack.pop_back();

      auto it = base_types_.find(current);
      if(it == base_types_.end())
      {
        continue;
      }

      for(const auto& base : it->second)
      {
        if(parent.find(base) != parent.end())
        {
          continue;
        }
        parent.insert({ base, current });
        if(base == dfs_target)
        {
          // Reconstruct path: trace from dfs_target back to dfs_start
          std::vector<std::type_index> path;
          auto node = dfs_target;
          while(node != dfs_start)
          {
            path.push_back(node);
            node = parent.at(node);
          }
          path.push_back(dfs_start);

          if(reverse_path)
          {
            std::reverse(path.begin(), path.end());
          }

          // Apply casts along the path
          linb::any current_value = from;
          for(size_t i = 0; i + 1 < path.size(); ++i)
          {
            auto cast_key = std::make_pair(path[i], path[i + 1]);
            auto cast_it = cast_map.find(cast_key);
            if(cast_it == cast_map.end())
            {
              return nonstd::make_unexpected(std::string("Transitive cast: missing step "
                                                         "in chain"));
            }
            try
            {
              current_value = cast_it->second(current_value);
            }
            catch(const std::exception& e)
            {
              return nonstd::make_unexpected(std::string("Transitive cast step "
                                                         "failed: ") +
                                             e.what());
            }
          }
          return current_value;
        }
        stack.push_back(base);
      }
    }
    return nonstd::make_unexpected(std::string("No transitive path found"));
  }

  mutable std::shared_mutex mutex_;
  std::map<std::pair<std::type_index, std::type_index>, CastFunction> upcasts_;
  std::map<std::pair<std::type_index, std::type_index>, CastFunction> downcasts_;
  std::map<std::type_index, std::set<std::type_index>> base_types_;
};

}  // namespace BT
