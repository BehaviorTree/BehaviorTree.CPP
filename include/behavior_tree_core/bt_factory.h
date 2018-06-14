/* Copyright (C) 2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018 Davide Faconti -  All Rights Reserved
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
#include <map>
#include <set>
#include <cstring>
#include <algorithm>

#include "behavior_tree_core/behavior_tree.h"

namespace BT
{

struct TreeNodeModel
{
    NodeType type;
    std::string registration_ID;
    NodeParameters required_parameters;
};

class BehaviorTreeFactory
{
  public:
    BehaviorTreeFactory();

    bool unregisterBuilder(const std::string& ID);

    void registerBuilder(const std::string& ID, NodeBuilder builder);

    void registerSimpleAction(const std::string& ID, const std::function<NodeStatus()> &tick_functor);

    void registerSimpleCondition(const std::string& ID, const std::function<NodeStatus()> &tick_functor);

    std::unique_ptr<TreeNode> instantiateTreeNode(const std::string& ID, const std::string& name,
                                                  const NodeParameters& params) const;

    const std::map<std::string, NodeBuilder>& builders() const
    {
        return builders_;
    }

    const std::vector<TreeNodeModel>& models() const
    {
        return treenode_models_;
    }

    template <typename T>
    void registerNodeType(const std::string& ID)
    {
        static_assert(std::is_base_of<ActionNode, T>::value || std::is_base_of<ControlNode, T>::value ||
                          std::is_base_of<DecoratorNode, T>::value || std::is_base_of<ConditionNode, T>::value,
                      "[registerBuilder]: accepts only classed derived from either ActionNode, DecoratorNode, "
                      "ControlNode "
                      "or ConditionNode");

        constexpr bool default_constructable = std::is_constructible<T, std::string>::value;
        constexpr bool param_constructable = std::is_constructible<T, std::string, const NodeParameters&>::value;
        constexpr bool has_static_required_parameters = has_static_method_requiredNodeParameters<T>::value;

        static_assert(default_constructable || param_constructable,
                      "[registerBuilder]: the registered class must have at least one of these two constructors:\n\n"
                      "    (const std::string&, const NodeParameters&) or (const std::string&)");

        static_assert(!(param_constructable && !has_static_required_parameters),
                      "[registerBuilder]: a node that accepts NodeParameters must also implement a static method:\n\n"
                      "    const NodeParameters& requiredNodeParameters(); ");

        registerNodeTypeImpl<T>(ID);
        storeNodeModel<T>(ID);
    }

  private:
    std::map<std::string, NodeBuilder> builders_;
    std::vector<TreeNodeModel> treenode_models_;

    // template specialization + SFINAE + black magic

    // clang-format off
    template <typename T>
    using has_default_constructor = typename std::is_constructible<T, const std::string&>;

    template <typename T>
    using has_params_constructor  = typename std::is_constructible<T, const std::string&, const NodeParameters&>;

    template <typename T, typename = void>
    struct has_static_method_requiredNodeParameters: std::false_type {};

    template <typename T>
    struct has_static_method_requiredNodeParameters<T,
            typename std::enable_if<std::is_same<decltype(T::requiredNodeParameters()), const NodeParameters&>::value>::type>
        : std::true_type {};

    template <typename T>
    typename std::enable_if< has_default_constructor<T>::value && !has_params_constructor<T>::value>::type
    registerNodeTypeImpl(const std::string& ID)
    {
        NodeBuilder builder = [](const std::string& name, const NodeParameters&)
        {
            return std::unique_ptr<TreeNode>(new T(name));
        };
        registerBuilder(ID, builder);
    }

    template <typename T>
    typename std::enable_if< !has_default_constructor<T>::value && has_params_constructor<T>::value>::type
    registerNodeTypeImpl(const std::string& ID)
    {
        NodeBuilder builder = [](const std::string& name, const NodeParameters& params)
        {
            return std::unique_ptr<TreeNode>(new T(name, params));
        };
        registerBuilder(ID, builder);
    }

    template <typename T>
    typename std::enable_if< has_default_constructor<T>::value && has_params_constructor<T>::value>::type
    registerNodeTypeImpl(const std::string& ID)
    {
        NodeBuilder builder = [](const std::string& name, const NodeParameters& params)
        {
            if( params.empty() )
            {
                // call this one that MIGHT use default initialization
                return std::unique_ptr<TreeNode>(new T(name));
            }
            return std::unique_ptr<TreeNode>(new T(name, params));
        };
        registerBuilder(ID, builder);
    }


    template<typename T>
    typename std::enable_if< has_static_method_requiredNodeParameters<T>::value>::type
    storeNodeModel(const std::string& ID)
    {
        treenode_models_.push_back( { getType<T>(), ID, T::requiredNodeParameters()} );
        sortTreeNodeModel();
    }

    template<typename T>
    typename std::enable_if< !has_static_method_requiredNodeParameters<T>::value>::type
    storeNodeModel(const std::string& ID)
    {
        treenode_models_.push_back( { getType<T>(), ID, NodeParameters()} );
        sortTreeNodeModel();
    }

    void sortTreeNodeModel();

    // clang-format on
};

}   // end namespace
#endif   // BT_FACTORY_H
