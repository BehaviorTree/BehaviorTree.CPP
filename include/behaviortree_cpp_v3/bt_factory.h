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


#include "behaviortree_cpp_v3/behavior_tree.h"

namespace BT
{

/// The term "Builder" refers to the Builder Pattern (https://en.wikipedia.org/wiki/Builder_pattern)
typedef std::function<std::unique_ptr<TreeNode>(const std::string&, const NodeConfiguration&)>
NodeBuilder;

template <typename T>
using has_default_constructor = typename std::is_constructible<T, const std::string&>;

template <typename T>
using has_params_constructor  = typename std::is_constructible<T, const std::string&, const NodeConfiguration&>;


template <typename T> inline
  NodeBuilder CreateBuilder(typename std::enable_if<has_default_constructor<T>::value &&
                                        has_params_constructor<T>::value >::type* = nullptr)
{
  return [](const std::string& name, const NodeConfiguration& config)
  {
    // Special case. Use default constructor if parameters are empty
    if( config.input_ports.empty() &&
        config.output_ports.empty() &&
        has_default_constructor<T>::value)
    {
      return std::make_unique<T>(name);
    }
    return std::make_unique<T>(name, config);
  };
}

template <typename T> inline
  NodeBuilder CreateBuilder(typename std::enable_if<!has_default_constructor<T>::value &&
                                        has_params_constructor<T>::value >::type* = nullptr)
{
  return [](const std::string& name, const NodeConfiguration& params)
  {
    return std::unique_ptr<TreeNode>(new T(name, params));
  };
}

template <typename T> inline
  NodeBuilder CreateBuilder(typename std::enable_if<has_default_constructor<T>::value &&
                                        !has_params_constructor<T>::value >::type* = nullptr)
{
  return [](const std::string& name, const NodeConfiguration&)
  {
    return std::unique_ptr<TreeNode>(new T(name));
  };
}


template <typename T> inline
TreeNodeManifest CreateManifest(const std::string& ID, PortsList portlist = getProvidedPorts<T>())
{
  return { getType<T>(), ID, portlist };
}


constexpr const char* PLUGIN_SYMBOL = "BT_RegisterNodesFromPlugin";

#ifndef BT_PLUGIN_EXPORT

/* Use this macro to automatically register one or more custom Nodes
into a factory. For instance:

BT_REGISTER_NODES(factory)
{
    factory.registerNodeType<MoveBaseAction>("MoveBase");
}

IMPORTANT: this must funtion MUST be declared in a cpp file, NOT a header file.
See examples for more information about configuring CMake correctly
*/
#define BT_REGISTER_NODES(factory)                                                                 \
        static void BT_RegisterNodesFromPlugin(BT::BehaviorTreeFactory& factory)

#else

#if defined(__linux__) || defined __APPLE__

#define BT_REGISTER_NODES(factory)                                                                 \
    extern "C" void __attribute__((visibility("default")))                                         \
        BT_RegisterNodesFromPlugin(BT::BehaviorTreeFactory& factory)

#elif _WIN32

#define BT_REGISTER_NODES(factory)                                                                 \
    extern "C" void __declspec(dllexport) BT_RegisterNodesFromPlugin(BT::BehaviorTreeFactory& factory)
#endif

#endif


/**
 * @brief Struct used to store a tree.
 * If this object goes out of scope, the tree is destroyed.
 *
 * To tick the tree, simply call:
 *
 *    NodeStatus status = my_tree.tickRoot();
 */
class Tree
{
public:

    std::vector<TreeNode::Ptr> nodes;
    std::vector<Blackboard::Ptr> blackboard_stack;
    std::unordered_map<std::string, TreeNodeManifest> manifests;

    Tree(){}

    // non-copyable. Only movable
    Tree(const Tree& ) = delete;
    Tree& operator=(const Tree&) = delete;

    Tree(Tree&& other)
    {
        (*this) = std::move(other);
    }

    Tree& operator=(Tree&& other)
    {
        nodes = std::move(other.nodes);
        blackboard_stack = std::move(other.blackboard_stack);
        manifests = std::move(other.manifests);
        return *this;
    }

    void haltTree()
    {
        if(!rootNode())
        {
            return;
        }
        // the halt should propagate to all the node if the nodes
        // have been implemented correctly
        rootNode()->halt();
        rootNode()->setStatus(NodeStatus::IDLE);

        //but, just in case.... this should be no-op
        auto visitor = [](BT::TreeNode * node) {
            node->halt();
            node->setStatus(BT::NodeStatus::IDLE);
        };
        BT::applyRecursiveVisitor(rootNode(), visitor);
    }

    TreeNode* rootNode() const
    {
      return nodes.empty() ? nullptr : nodes.front().get();
    }

    NodeStatus tickRoot()
    {
      if(!rootNode())
      {
        throw RuntimeError("Empty Tree");
      }
      NodeStatus ret = rootNode()->executeTick();
      if( ret == NodeStatus::SUCCESS || ret == NodeStatus::FAILURE){
        rootNode()->setStatus(BT::NodeStatus::IDLE);
      }
      return ret;
    }

    ~Tree();

    Blackboard::Ptr rootBlackboard();

};

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

    /// The most generic way to register your own builder.
    void registerBuilder(const TreeNodeManifest& manifest, const NodeBuilder& builder);

    template <typename T>
    void registerBuilder(const std::string& ID, const NodeBuilder& builder )
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
    void registerFromPlugin(const std::string &file_path);

    /**
     * @brief registerFromROSPlugins finds all shared libraries that export ROS plugins for behaviortree_cpp, and calls registerFromPlugin for each library.
     * @throws If not compiled with ROS support or if the library cannot load for any reason
     *
     */
    void registerFromROSPlugins();

    /**
     * @brief instantiateTreeNode creates an instance of a previously registered TreeNode.
     *
     * @param name     name of this particular instance
     * @param ID       ID used when it was registered
     * @param config   configuration that is passed to the constructor of the TreeNode.
     * @return         new node.
     */
    std::unique_ptr<TreeNode> instantiateTreeNode(const std::string& name, const std::string &ID,
                                                  const NodeConfiguration& config) const;

    /** registerNodeType is the method to use to register your custom TreeNode.
     *
     *  It accepts only classed derived from either ActionNodeBase, DecoratorNode,
     *  ControlNode or ConditionNode.
     */
    template <typename T>
    void registerNodeType(const std::string& ID)
    {
        static_assert(std::is_base_of<ActionNodeBase, T>::value ||
                      std::is_base_of<ControlNode, T>::value ||
                      std::is_base_of<DecoratorNode, T>::value ||
                      std::is_base_of<ConditionNode, T>::value,
                      "[registerNode]: accepts only classed derived from either ActionNodeBase, "
                      "DecoratorNode, ControlNode or ConditionNode");

        static_assert(!std::is_abstract<T>::value,
                      "[registerNode]: Some methods are pure virtual. "
                      "Did you override the methods tick() and halt()?");

        constexpr bool default_constructable = std::is_constructible<T, const std::string&>::value;
        constexpr bool param_constructable =
                std::is_constructible<T, const std::string&, const NodeConfiguration&>::value;
        constexpr bool has_static_ports_list =
                has_static_method_providedPorts<T>::value;

        static_assert(default_constructable || param_constructable,
                      "[registerNode]: the registered class must have at least one of these two "
                      "constructors: "
                      "  (const std::string&, const NodeConfiguration&) or (const std::string&).");

        static_assert(!(param_constructable && !has_static_ports_list),
                      "[registerNode]: you MUST implement the static method: "
                      "  PortsList providedPorts();\n");

        static_assert(!(has_static_ports_list && !param_constructable),
                      "[registerNode]: since you have a static method providedPorts(), "
                      "you MUST add a constructor sign signature (const std::string&, const "
                      "NodeParameters&)\n");

        registerBuilder( CreateManifest<T>(ID), CreateBuilder<T>());
    }

    template <typename T>
    void registerNodeType(const std::string& ID, PortsList ports)
    {
      static_assert(std::is_base_of<ActionNodeBase, T>::value ||
                      std::is_base_of<ControlNode, T>::value ||
                      std::is_base_of<DecoratorNode, T>::value ||
                      std::is_base_of<ConditionNode, T>::value,
                    "[registerNode]: accepts only classed derived from either ActionNodeBase, "
                    "DecoratorNode, ControlNode or ConditionNode");

      static_assert(!std::is_abstract<T>::value,
                    "[registerNode]: Some methods are pure virtual. "
                    "Did you override the methods tick() and halt()?");

      constexpr bool default_constructable = std::is_constructible<T, const std::string&>::value;
      constexpr bool param_constructable =
        std::is_constructible<T, const std::string&, const NodeConfiguration&>::value;
      constexpr bool has_static_ports_list =
        has_static_method_providedPorts<T>::value;

      static_assert(default_constructable || param_constructable,
                    "[registerNode]: the registered class must have at least one of these two "
                    "constructors: (const std::string&, const NodeConfiguration&) or (const std::string&).");

      static_assert(!has_static_ports_list,
                    "[registerNode]: ports are passed to this node explicitly. The static method"
                    "providedPorts() should be removed to avoid ambiguities\n");

      static_assert(param_constructable,
                    "[registerNode]: since this node has ports, "
                    "you MUST add a constructor sign signature (const std::string&, const "
                    "NodeParameters&)\n");

      registerBuilder( CreateManifest<T>(ID, ports), CreateBuilder<T>());
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

private:
    std::unordered_map<std::string, NodeBuilder> builders_;
    std::unordered_map<std::string, TreeNodeManifest> manifests_;
    std::set<std::string> builtin_IDs_;

    // clang-format on
};


}   // end namespace

#endif   // BT_FACTORY_H
