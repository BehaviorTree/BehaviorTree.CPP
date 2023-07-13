/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
*  Copyright (C) 2018-2023 Davide Faconti -  All Rights Reserved
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

#include <condition_variable>
#include <exception>
#include <mutex>
#include <map>

#include "behaviortree_cpp/utils/signal.h"
#include "behaviortree_cpp/basic_types.h"
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/utils/strcat.hpp"
#include "behaviortree_cpp/utils/wakeup_signal.hpp"
#include "behaviortree_cpp/scripting/script_parser.hpp"

#ifdef _MSC_VER
#pragma warning(disable : 4127)
#endif

namespace BT
{

/// This information is used mostly by the XMLParser.
struct TreeNodeManifest
{
  NodeType type;
  std::string registration_ID;
  PortsList ports;
  std::string description;
};

using PortsRemapping = std::unordered_map<std::string, std::string>;

enum class PreCond
{
  // order of the enums also tell us the execution order
  FAILURE_IF = 0,
  SUCCESS_IF,
  SKIP_IF,
  WHILE_TRUE,
  COUNT_
};

enum class PostCond
{
  // order of the enums also tell us the execution order
  ON_HALTED = 0,
  ON_FAILURE,
  ON_SUCCESS,
  ALWAYS,
  COUNT_
};

template <> [[nodiscard]]
std::string toStr<BT::PostCond>(const BT::PostCond& status);

template <> [[nodiscard]]
std::string toStr<BT::PreCond>(const BT::PreCond& status);

using ScriptingEnumsRegistry = std::unordered_map<std::string, int>;

struct NodeConfig
{
  NodeConfig()
  {}

  // Pointer to the blackboard used by this node
  Blackboard::Ptr blackboard;
  // List of enums available for scripting
  std::shared_ptr<ScriptingEnumsRegistry> enums;
  // input ports
  PortsRemapping input_ports;
  // output ports
  PortsRemapping output_ports;

  const TreeNodeManifest* manifest = nullptr;

  // Numberic unique identifier
  uint16_t uid = 0;
  // Unique human-readable name, that encapsulate the subtree
  // hierarchy, for instance, given 2 nested trees, it should be:
  //
  //   main_tree/nested_tree/my_action
  std::string path;

  std::map<PreCond, std::string> pre_conditions;
  std::map<PostCond, std::string> post_conditions;
};

// back compatibility
using NodeConfiguration = NodeConfig;


template <typename T>
inline constexpr bool hasNodeNameCtor()
{
  return std::is_constructible<T, const std::string&>::value;
}

template <typename T, typename... ExtraArgs>
inline constexpr bool hasNodeFullCtor()
{
  return std::is_constructible<T, const std::string&, const NodeConfig&,
                               ExtraArgs...>::value;
}

/// Abstract base class for Behavior Tree Nodes
class TreeNode
{
public:
  typedef std::shared_ptr<TreeNode> Ptr;

  /**
     * @brief TreeNode main constructor.
     *
     * @param name     name of the instance, not the type.
     * @param config   information about input/output ports. See NodeConfig
     *
     * Note: If your custom node has ports, the derived class must implement:
     *
     *     static PortsList providedPorts();
     */
  TreeNode(std::string name, NodeConfig config);

  TreeNode(const TreeNode& other) = delete;
  TreeNode& operator=(const TreeNode& other) = delete;

  TreeNode(TreeNode&& other) noexcept;
  TreeNode& operator=(TreeNode&& other) noexcept;

  virtual ~TreeNode();

  /// The method that should be used to invoke tick() and setStatus();
  virtual BT::NodeStatus executeTick();

  void haltNode();

  [[nodiscard]] bool isHalted() const;

  [[nodiscard]] NodeStatus status() const;

  /// Name of the instance, not the type
  [[nodiscard]] const std::string& name() const;

  /// Blocking function that will sleep until the setStatus() is called with
  /// either RUNNING, FAILURE or SUCCESS.
  [[nodiscard]] BT::NodeStatus waitValidStatus();

  virtual NodeType type() const = 0;

  using StatusChangeSignal = Signal<TimePoint, const TreeNode&, NodeStatus, NodeStatus>;
  using StatusChangeSubscriber = StatusChangeSignal::Subscriber;
  using StatusChangeCallback = StatusChangeSignal::CallableFunction;

  using PreTickCallback =
      std::function<NodeStatus(TreeNode&)>;
  using PostTickCallback =
      std::function<NodeStatus(TreeNode&, NodeStatus)>;

  /**
     * @brief subscribeToStatusChange is used to attach a callback to a status change.
     * When StatusChangeSubscriber goes out of scope (it is a shared_ptr) the callback
     * is unsubscribed automatically.
     *
     * @param callback The callback to be execute when status change.
     *
     * @return the subscriber handle.
     */
  [[nodiscard]] StatusChangeSubscriber subscribeToStatusChange(StatusChangeCallback callback);

  /** This method attaches to the TreeNode a callback with signature:
     *
     *     Optional<NodeStatus> myCallback(TreeNode& node)
     *
     * This callback is executed BEFORE the tick() and, if it returns a valid Optional<NodeStatus>,
     * the actual tick() will NOT be executed and this result will be returned instead.
     *
     * This is useful to inject a "dummy" implementation of the TreeNode at run-time
     */
  void setPreTickFunction(PreTickCallback callback);

  /**
   * This method attaches to the TreeNode a callback with signature:
   *
   *     Optional<NodeStatus> myCallback(TreeNode& node, NodeStatus new_status)
   *
   * This callback is executed AFTER the tick() and, if it returns a valid Optional<NodeStatus>,
   * the value returned by the actual tick() is overriden with this one.
   */
  void setPostTickFunction(PostTickCallback callback);

  /// The unique identifier of this instance of treeNode.
  /// It is assigneld by the factory
  [[nodiscard]] uint16_t UID() const;

  /// Human readable identifier, that includes the hierarchy of Subtrees
  /// See tutorial 10 as an example.
  [[nodiscard]] const std::string& fullPath() const;

  /// registrationName is the ID used by BehaviorTreeFactory to create an instance.
  [[nodiscard]] const std::string& registrationName() const;

  /// Configuration passed at construction time. Can never change after the
  /// creation of the TreeNode instance.
  [[nodiscard]] const NodeConfig& config() const;

  /** Read an input port, which, in practice, is an entry in the blackboard.
   * If the blackboard contains a std::string and T is not a string,
   * convertFromString<T>() is used automatically to parse the text.
   *
   * @param key   the name of the port.
   * @return      false if an error occurs.
   */
  template <typename T>
  Result getInput(const std::string& key, T& destination) const;

  /** Same as bool getInput(const std::string& key, T& destination)
   * but using optional.
   *
   * @param key   the name of the port.
   */
  template <typename T> [[nodiscard]]
  Expected<T> getInput(const std::string& key) const
  {
    T out;
    auto res = getInput(key, out);
    return (res) ? Expected<T>(out) : nonstd::make_unexpected(res.error());
  }

  /**
   * @brief setOutput modifies the content of an Output port
   * @param key    the name of the port.
   * @param value  new value
   * @return       valid Result, is succesfull.
   */
  template <typename T>
  Result setOutput(const std::string& key, const T& value);

  /**
   * @brief getLockedPortContent should be used when:
   *
   * - your port contains an object with reference semantic (usually a smart pointer)
   * - you want to modify the object we are pointing to.
   * - you are concerned about thread-safety.
   *
   * For example, if your port has type std::shared_ptr<Foo>,
   * the code below is NOT thread safe:
   *
   *    auto foo_ptr = getInput<std::shared_ptr<Foo>>("port_name");
   *    // modifying the content of foo_ptr is NOT thread-safe
   *
   * What you must do, instead, to guaranty thread-safety, is:
   *
   *    if(auto any_ref = getLockedPortContent("port_name")) {
   *      Any* any = any_ref.get();
   *      auto foo_ptr = any->cast<std::shared_ptr<Foo>>();
   *      // modifying the content of foo_ptr inside this scope IS thread-safe
   *    }
   *
   * It is important to destroy the object AnyPtrLocked, to release the lock.
   *
   * NOTE: this method doesn't work, if the port contains a static string, instead
   * of a blackboard pointer.
   *
   * @param key  the identifier of the port.
   * @return     empty AnyPtrLocked if the blackboard entry doesn't exist or the content
   *             of the port was a static string.
   */
  [[nodiscard]] AnyPtrLocked getLockedPortContent(const std::string& key);

  // function provided mostly for debugging purpose to see the raw value
  // in the port (no remapping and no conversion to a type)
  [[nodiscard]] StringView getRawPortValue(const std::string& key) const;

  /// Check a string and return true if it matches the pattern:  {...}
  [[nodiscard]]
  static bool isBlackboardPointer(StringView str, StringView* stripped_pointer = nullptr);

  [[nodiscard]]
  static StringView stripBlackboardPointer(StringView str);

  [[nodiscard]]
  static Expected<StringView> getRemappedKey(StringView port_name,
                                             StringView remapped_port);

  /// Notify that the tree should be ticked again()
  void emitWakeUpSignal();

  [[nodiscard]]
  bool requiresWakeUp() const;

  /** Used to inject config into a node, even if it doesn't have the proper
     *  constructor
     */
  template <class DerivedT, typename... ExtraArgs>
  static std::unique_ptr<TreeNode> Instantiate(const std::string& name,
                                               const NodeConfig& config,
                                               ExtraArgs... args)
  {
    static_assert(hasNodeFullCtor<DerivedT, ExtraArgs...>() ||
                  hasNodeNameCtor<DerivedT>());

    if constexpr (hasNodeFullCtor<DerivedT, ExtraArgs...>())
    {
      return std::make_unique<DerivedT>(name, config, args...);
    }
    else if constexpr (hasNodeNameCtor<DerivedT>())
    {
      auto node_ptr = new DerivedT(name, args...);
      node_ptr->config() = config;
      return std::unique_ptr<DerivedT>(node_ptr);
    }
  }

protected:
  friend class BehaviorTreeFactory;
  friend class DecoratorNode;
  friend class ControlNode;
  friend class Tree;

  [[nodiscard]] NodeConfig& config();

  /// Method to be implemented by the user
  virtual BT::NodeStatus tick() = 0;

  /// Set the status to IDLE
  void resetStatus();

  // Only BehaviorTreeFactory should call this
  void setRegistrationID(StringView ID);

  void setWakeUpInstance(std::shared_ptr<WakeUpSignal> instance);

  void modifyPortsRemapping(const PortsRemapping& new_remapping);

  /**
     * @brief setStatus changes the status of the node.
     * it will throw if you try to change the status to IDLE, because
     * your parent node should do that, not the user!
     */
  void setStatus(NodeStatus new_status);

  using PreScripts = std::array<ScriptFunction, size_t(PreCond::COUNT_)>;
  using PostScripts = std::array<ScriptFunction, size_t(PostCond::COUNT_)>;

  PreScripts& preConditionsScripts();
  PostScripts& postConditionsScripts();

private:

  struct PImpl;
  std::unique_ptr<PImpl> _p;

  Expected<NodeStatus> checkPreConditions();
  void checkPostConditions(NodeStatus status);

  /// The method used to interrupt the execution of a RUNNING node.
  /// Only Async nodes that may return RUNNING should implement it.
  virtual void halt() = 0;
};

//-------------------------------------------------------
template <typename T>
inline Result TreeNode::getInput(const std::string& key, T& destination) const
{
  // address the special case where T is an enum
  auto ParseString = [this](const std::string& str) -> T
  {
    if constexpr (std::is_enum_v<T> && !std::is_same_v<T, NodeStatus>)
    {
      auto it = config().enums->find(str);
      // conversion available
      if( it != config().enums->end() )
      {
        return static_cast<T>(it->second);
      }
      else {
        // hopefully str contains a number that can be parsed. May throw
        return static_cast<T>(convertFromString<int>(str));
      }
    }
    else {
      return convertFromString<T>(str);
    }
  };

  auto remap_it = config().input_ports.find(key);
  if (remap_it == config().input_ports.end())
  {
    return nonstd::make_unexpected(StrCat("getInput() failed because "
                                          "NodeConfig::input_ports "
                                          "does not contain the key: [",
                                          key, "]"));
  }

  // special case. Empty port value, we should use the default value,
  // if available in the model.
  // BUT, it the port type is a string, then an empty string might be
  // a valid value
  const std::string& port_value_str = remap_it->second;
  if(port_value_str.empty() && config().manifest)
  {
    const auto& port_manifest = config().manifest->ports.at(key);
    const auto& default_value = port_manifest.defaultValue();
    if(!default_value.empty() && !default_value.isString())
    {
      destination = default_value.cast<T>();
      return {};
    }
  }

  auto remapped_res = getRemappedKey(key, port_value_str);
  try
  {
    // pure string, not a blackboard key
    if (!remapped_res)
    {
      destination = ParseString(port_value_str);
      return {};
    }
    const auto& remapped_key = remapped_res.value();

    if (!config().blackboard)
    {
      return nonstd::make_unexpected("getInput(): trying to access an invalid Blackboard");
    }

    if (auto any_ref = config().blackboard->getAnyLocked(std::string(remapped_key)))
    {
      auto val = any_ref.get();
      if(!val->empty())
      {
        if (!std::is_same_v<T, std::string> &&
            val->type() == typeid(std::string))
        {
          destination = ParseString(val->cast<std::string>());
        }
        else
        {
          destination = val->cast<T>();
        }
        return {};
      }
    }

    return nonstd::make_unexpected(StrCat("getInput() failed because it was unable to "
                                          "find the key [", key, "] remapped to [",
                                          remapped_key, "]"));
  }
  catch (std::exception& err)
  {
    return nonstd::make_unexpected(err.what());
  }
}

template <typename T>
inline Result TreeNode::setOutput(const std::string& key, const T& value)
{
  if (!config().blackboard)
  {
    return nonstd::make_unexpected("setOutput() failed: trying to access a "
                                   "Blackboard(BB) entry, but BB is invalid");
  }

  auto remap_it = config().output_ports.find(key);
  if (remap_it == config().output_ports.end())
  {
    return nonstd::make_unexpected(StrCat("setOutput() failed: "
                                          "NodeConfig::output_ports "
                                          "does not contain the key: [",
                                          key, "]"));
  }
  StringView remapped_key = remap_it->second;
  if (remapped_key == "=")
  {
    remapped_key = key;
  }
  if (isBlackboardPointer(remapped_key))
  {
    remapped_key = stripBlackboardPointer(remapped_key);
  }
  config().blackboard->set(static_cast<std::string>(remapped_key), value);

  return {};
}

// Utility function to fill the list of ports using T::providedPorts();
template <typename T>
inline void assignDefaultRemapping(NodeConfig& config)
{
  for (const auto& it : getProvidedPorts<T>())
  {
    const auto& port_name = it.first;
    const auto direction = it.second.direction();
    if (direction != PortDirection::OUTPUT)
    {
      // PortDirection::{INPUT,INOUT}
      config.input_ports[port_name] = "=";
    }
    if (direction != PortDirection::INPUT)
    {
      // PortDirection::{OUTPUT,INOUT}
      config.output_ports[port_name] = "=";
    }
  }
}

}   // namespace BT
