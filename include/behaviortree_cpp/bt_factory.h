/* Copyright (C) 2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
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

#ifndef BT_FACTORY_H
#define BT_FACTORY_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <algorithm>
#include <set>

#include "behaviortree_cpp/contrib/magic_enum.hpp"
#include "behaviortree_cpp/behavior_tree.h"

namespace BT
{
/// The term "Builder" refers to the Builder Pattern (https://en.wikipedia.org/wiki/Builder_pattern)
using NodeBuilder =
    std::function<std::unique_ptr<TreeNode>(const std::string&, const NodeConfig&)>;

template <typename T, typename... Args>
inline NodeBuilder CreateBuilder(Args... args)
{
  return [=](const std::string& name, const NodeConfig& config) {
    return TreeNode::Instantiate<T, Args...>(name, config, args...);
  };
}

template <typename T>
inline TreeNodeManifest CreateManifest(const std::string& ID,
                                       PortsList portlist = getProvidedPorts<T>())
{
  return {getType<T>(), ID, portlist, {}};
}

#ifdef BT_PLUGIN_EXPORT

#if defined(_WIN32)
  #define BTCPP_EXPORT extern "C" __declspec(dllexport)
#else
  // Unix-like OSes
  #define BTCPP_EXPORT extern "C" __attribute__ ((visibility ("default")))
#endif

#else
  #define BTCPP_EXPORT static
#endif
/* Use this macro to automatically register one or more custom Nodes
* into a factory. For instance:
*
*   BT_REGISTER_NODES(factory)
*   {
*     factory.registerNodeType<MoveBaseAction>("MoveBase");
*   }
*
* IMPORTANT: this function MUST be declared in a cpp file, NOT a header file.
* In your cake, you must add the definition [BT_PLUGIN_EXPORT] with:
*
*   target_compile_definitions(my_plugin_target PRIVATE  BT_PLUGIN_EXPORT )

* See examples in sample_nodes directory.
*/

#define BT_REGISTER_NODES(factory)                                                       \
  BTCPP_EXPORT void BT_RegisterNodesFromPlugin(BT::BehaviorTreeFactory& factory)

constexpr const char* PLUGIN_SYMBOL = "BT_RegisterNodesFromPlugin";

/**
 * @brief Struct used to store a tree.
 * If this object goes out of scope, the tree is destroyed.
 */
class Tree
{
public:
  // a tree can contain multiple subtree.
  struct Subtree
  {
    using Ptr = std::shared_ptr<Subtree>;
    std::vector<TreeNode::Ptr> nodes;
    Blackboard::Ptr blackboard;
    std::string instance_name;
    std::string tree_ID;
  };

  std::vector<Subtree::Ptr> subtrees;

  std::unordered_map<std::string, TreeNodeManifest> manifests;

  Tree()
  {}

  // non-copyable. Only movable
  Tree(const Tree&) = delete;
  Tree& operator=(const Tree&) = delete;

  Tree(Tree&& other)
  {
    (*this) = std::move(other);
  }

  Tree& operator=(Tree&& other)
  {
    subtrees = std::move(other.subtrees);
    manifests = std::move(other.manifests);
    wake_up_ = other.wake_up_;
    return *this;
  }

  void initialize();

  void haltTree()
  {
    if (!rootNode())
    {
      return;
    }
    // the halt should propagate to all the node if the nodes
    // have been implemented correctly
    rootNode()->haltNode();

    //but, just in case.... this should be no-op
    auto visitor = [](BT::TreeNode* node) { node->haltNode(); };
    BT::applyRecursiveVisitor(rootNode(), visitor);

    rootNode()->resetStatus();
  }

  TreeNode* rootNode() const;

  /// Sleep for a certain amount of time.
  /// This sleep could be interrupted by the method
  /// TreeNode::emitWakeUpSignal()
  void sleep(std::chrono::system_clock::duration timeout);

  ~Tree();

  /// Tick the root of the tree once, even if a node invoked
  /// emitWakeUpSignal()
  NodeStatus tickExactlyOnce();

  /**
   * @brief by default, tickOnce() sends a single tick, BUT
   * as long as there is at least one node of the tree
   * invoking TreeNode::emitWakeUpSignal(), it will be ticked again.
   */
  NodeStatus tickOnce();

  /// Call tickOnce until the status is different from RUNNING.
  /// Note that between one tick and the following one,
  /// a Tree::sleep() is used
  NodeStatus
  tickWhileRunning(std::chrono::milliseconds sleep_time = std::chrono::milliseconds(10));

  Blackboard::Ptr rootBlackboard();

  //Call the visitor for each node of the tree.
  void applyVisitor(const std::function<void(const TreeNode*)>& visitor);

  //Call the visitor for each node of the tree.
  void applyVisitor(const std::function<void(TreeNode*)>& visitor);

  uint16_t getUID();


private:
  std::shared_ptr<WakeUpSignal> wake_up_;

  enum TickOption
  {
    EXACTLY_ONCE,
    ONCE_UNLESS_WOKEN_UP,
    WHILE_RUNNING
  };

  NodeStatus tickRoot(TickOption opt, std::chrono::milliseconds sleep_time);

  uint16_t uid_counter_ = 0;
};

class Parser;

/**
 * @brief The BehaviorTreeFactory is used to create instances of a
 * TreeNode at run-time.
 *
 * Some node types are "builtin", whilst other are used defined and need
 * to be registered using a unique ID.
 */
class BehaviorTreeFactory
{
public:
  BehaviorTreeFactory();

  /// Remove a registered ID.
  bool unregisterBuilder(const std::string& ID);

  /** The most generic way to register a NodeBuilder.
    *
    * Throws if you try to register twice a builder with the same
    * registration_ID.
    */
  void registerBuilder(const TreeNodeManifest& manifest, const NodeBuilder& builder);

  template <typename T>
  void registerBuilder(const std::string& ID, const NodeBuilder& builder)
  {
    auto manifest = CreateManifest<T>(ID);
    registerBuilder(manifest, builder);
  }

  /**
    * @brief registerSimpleAction help you register nodes of type SimpleActionNode.
    *
    * @param ID            registration ID
    * @param tick_functor  the callback to be invoked in the tick() method.
    * @param ports         if your SimpleNode requires ports, provide the list here.
    *
    * */
  void registerSimpleAction(const std::string& ID,
                            const SimpleActionNode::TickFunctor& tick_functor,
                            PortsList ports = {});
  /**
    * @brief registerSimpleCondition help you register nodes of type SimpleConditionNode.
    *
    * @param ID            registration ID
    * @param tick_functor  the callback to be invoked in the tick() method.
    * @param ports         if your SimpleNode requires ports, provide the list here.
    *
    * */
  void registerSimpleCondition(const std::string& ID,
                               const SimpleConditionNode::TickFunctor& tick_functor,
                               PortsList ports = {});
  /**
    * @brief registerSimpleDecorator help you register nodes of type SimpleDecoratorNode.
    *
    * @param ID            registration ID
    * @param tick_functor  the callback to be invoked in the tick() method.
    * @param ports         if your SimpleNode requires ports, provide the list here.
    *
    * */
  void registerSimpleDecorator(const std::string& ID,
                               const SimpleDecoratorNode::TickFunctor& tick_functor,
                               PortsList ports = {});

  /**
     * @brief registerFromPlugin load a shared library and execute the function BT_REGISTER_NODES (see macro).
     *
     * @param file_path path of the file
     */
  void registerFromPlugin(const std::string& file_path);

  /**
     * @brief registerFromROSPlugins finds all shared libraries that export ROS plugins for behaviortree_cpp, and calls registerFromPlugin for each library.
     * @throws If not compiled with ROS support or if the library cannot load for any reason
     *
     */
  void registerFromROSPlugins();

  /**
     * @brief registerBehaviorTreeFromFile.
     * Load the definition of an entire behavior tree, but don't instantiate it.
     * You can instantiate it later with:
     *
     *    BehaviorTreeFactory::createTree(tree_id)
     *
     * where "tree_id" come from the XML attribute <BehaviorTree ID="tree_id">
     *
     */
  void registerBehaviorTreeFromFile(const std::string& filename);

  /// Same of registerBehaviorTreeFromFile, but passing the XML text,
  /// instead of the filename.
  void registerBehaviorTreeFromText(const std::string& xml_text);

  /// Returns the ID of the trees registered either with
  /// registerBehaviorTreeFromFile or registerBehaviorTreeFromText.
  std::vector<std::string> registeredBehaviorTrees() const;

  /**
   * @brief Clear previously-registered behavior trees.
   */
  void clearRegisteredBehaviorTrees();

  /**
     * @brief instantiateTreeNode creates an instance of a previously registered TreeNode.
     *
     * @param name     name of this particular instance
     * @param ID       ID used when it was registered
     * @param config   configuration that is passed to the constructor of the TreeNode.
     * @return         new node.
     */
  std::unique_ptr<TreeNode> instantiateTreeNode(const std::string& name,
                                                const std::string& ID,
                                                const NodeConfig& config) const;

  /** registerNodeType where you explicitly pass the list of ports.
   *  Doesn't require the implementation of static method providedPorts()
  */
  template <typename T, typename... ExtraArgs>
  void registerNodeType(const std::string& ID, const PortsList& ports, ExtraArgs... args)
  {
    static_assert(std::is_base_of<ActionNodeBase, T>::value ||
                  std::is_base_of<ControlNode, T>::value ||
                  std::is_base_of<DecoratorNode, T>::value ||
                  std::is_base_of<ConditionNode, T>::value,
                  "[registerNode]: accepts only classed derived from either "
                  "ActionNodeBase, "
                  "DecoratorNode, ControlNode or ConditionNode");

    constexpr bool default_constructable =
        std::is_constructible<T, const std::string&>::value;
    constexpr bool param_constructable =
        std::is_constructible<T, const std::string&, const NodeConfig&,
                              ExtraArgs...>::value;

    // clang-format off
    static_assert(!std::is_abstract<T>::value,
                  "[registerNode]: Some methods are pure virtual. "
                  "Did you override the methods tick() and halt()?");

    static_assert(default_constructable || param_constructable,
       "[registerNode]: the registered class must have at least one of these two constructors:\n"
       "  (const std::string&, const NodeConfig&) or (const std::string&)\n"
       "Check also if the constructor is public!)");
    // clang-format on

    registerBuilder(CreateManifest<T>(ID, ports), CreateBuilder<T>(args...));
  }

  /** registerNodeType is the method to use to register your custom TreeNode.
  *
  *  It accepts only classed derived from either ActionNodeBase, DecoratorNode,
  *  ControlNode or ConditionNode.
  */
  template <typename T, typename... ExtraArgs>
  void registerNodeType(const std::string& ID, ExtraArgs... args)
  {
    constexpr bool param_constructable =
        std::is_constructible<T, const std::string&, const NodeConfig&, ExtraArgs...>::value;
    constexpr bool has_static_ports_list = has_static_method_providedPorts<T>::value;

    // clang-format off
    static_assert(!(param_constructable && !has_static_ports_list),
                  "[registerNode]: you MUST implement the static method:\n"
                  "  PortsList providedPorts();\n");

    static_assert(!(has_static_ports_list && !param_constructable),
                  "[registerNode]: since you have a static method providedPorts(),\n"
                  "you MUST add a constructor with signature:\n"
                  "(const std::string&, const NodeParameters&)\n");
    // clang-format on
    registerNodeType<T>(ID, getProvidedPorts<T>(), args...);
  }

  /// All the builders. Made available mostly for debug purposes.
  const std::unordered_map<std::string, NodeBuilder>& builders() const;

  /// Manifests of all the registered TreeNodes.
  const std::unordered_map<std::string, TreeNodeManifest>& manifests() const;

  /// List of builtin IDs.
  const std::set<std::string>& builtinNodes() const;

  Tree createTreeFromText(const std::string& text,
                          Blackboard::Ptr blackboard = Blackboard::create());

  Tree createTreeFromFile(const std::string& file_path,
                          Blackboard::Ptr blackboard = Blackboard::create());

  Tree createTree(const std::string& tree_name,
                  Blackboard::Ptr blackboard = Blackboard::create());

  /// Add a description to a specific manifest. This description will be added
  /// to <TreeNodesModel> with the function writeTreeNodesModelXML()
  void addDescriptionToManifest(const std::string& node_id,
                                const std::string& description);

  /**
   * @brief Add an Enum to the scripting language.
   * For instance if you do:
   *
   * registerScriptingEnum("THE_ANSWER", 42),
   *
   * You may type this in your scripts:
   *
   * <Script code="myport:=THE_ANSWER" />
   *
   * @param name    string representation of the enum
   * @param value   its value.
   */
  void registerScriptingEnum(StringView name, int value);

  /**
   * @brief registerScriptingEnums is syntactic sugar
   * to automatically register multiple enums. We use
   * https://github.com/Neargye/magic_enum.
   *
   * Please refer to https://github.com/Neargye/magic_enum/blob/master/doc/limitations.md
   * for limitations.
   */
  template <typename EnumType>
  void registerScriptingEnums()
  {
    constexpr auto entries = magic_enum::enum_entries<EnumType>();
    for (const auto& it : entries)
    {
      registerScriptingEnum(it.second, static_cast<int>(it.first));
    }
  }

  void clearSubstitutionRules();

  using SubstitutionRule = std::variant<std::string, TestNodeConfig>;

  /**
   * @brief addSubstitutionRule replace a node with another one when the tree is
   * created.
   * If the rule ia a string, we will use a diferent node type (already registered)
   * instead.
   * If the rule is a TestNodeConfig, a test node with that configuration will be created instead.
   *
   * @param filter   filter used to select the node to sobstitute. The node path is used.
   *                 You may use wildcard matching.
   * @param rule     pass either a string or a TestNodeConfig
   */
  void addSubstitutionRule(StringView filter, SubstitutionRule rule);

  /**
   * @brief loadSubstitutionRuleFromJSON will parse a JSON file to
   * create a set of substitution rules. See Tutorial 11
   * for an example of the syntax.
   *
   * @param json_text  the JSON file as text (BOT the path of the file)
   */
  void loadSubstitutionRuleFromJSON(const std::string& json_text);

  /**
   * @brief substitutionRules return the current substitution rules.
   */
  const std::unordered_map<std::string, SubstitutionRule>&
  substitutionRules() const;

private:
  std::unordered_map<std::string, NodeBuilder> builders_;
  std::unordered_map<std::string, TreeNodeManifest> manifests_;
  std::set<std::string> builtin_IDs_;
  std::unordered_map<std::string, Any> behavior_tree_definitions_;

  std::shared_ptr<std::unordered_map<std::string, int>> scripting_enums_;

  std::shared_ptr<BT::Parser> parser_;

  std::unordered_map<std::string, SubstitutionRule> substitution_rules_;

};

}   // namespace BT

#endif   // BT_FACTORY_H
