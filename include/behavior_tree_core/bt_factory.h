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

#include "behavior_tree_core/action_node.h"
#include "behavior_tree_core/control_node.h"
#include "behavior_tree_core/condition_node.h"
#include "behavior_tree_core/decorator_node.h"

namespace BT
{

typedef std::set<std::string> RequiredParameters;

// We call Parameters the set of Key/Values that canbe read from file and are
// used to parametrize an object. it is up to the user's code to parse the string.
typedef std::map<std::string, std::string> NodeParameters;

// The term "Builder" refers to the Builder Ppattern (https://en.wikipedia.org/wiki/Builder_pattern)
typedef std::function<std::unique_ptr<TreeNode>(const std::string&,  const NodeParameters&)> NodeBuilder;

class BehaviorTreeFactory
{
  public:

    BehaviorTreeFactory();

    bool unregisterBuilder(const std::string& ID);

    template <typename T>
    void registerBuilder(const std::string& ID);

    void registerSimpleAction(const std::string& ID, std::function<BT::ReturnStatus()> tick_functor);

    void registerSimpleDecorator(const std::string& ID, std::function<BT::ReturnStatus(BT::ReturnStatus)> tick_functor);

    std::unique_ptr<TreeNode> instantiateTreeNode(const std::string& ID, const NodeParameters& params);

    const std::map<std::string, NodeBuilder> & builders() const
    {
        return builders_;
    }

private:

    std::map<std::string, NodeBuilder> builders_;
};


//-----------------------------------------------

template<typename T> inline
void BehaviorTreeFactory::registerBuilder(const std::string &ID)
{
    static_assert(std::is_base_of<ActionNode, T>::value ||
                  std::is_base_of<ControlNode, T>::value ||
                  std::is_base_of<DecoratorNode, T>::value ||
                  std::is_base_of<ConditionNode, T>::value,
                  "[registerBuilder]: accepts only classed derived from either ActionNode, DecoratorNode, ControlNode or ConditionNode");

    constexpr bool default_constructable = std::is_constructible<T, const std::string& >::value;
    constexpr bool param_constructable   = std::is_constructible<T, const std::string&, const NodeParameters&>::value;

    static_assert( param_constructable || param_constructable ,
                  "[registerBuilder]: the registered class must have a Constructor with signature:\n\n"
                  "  (const std::string&, const NodeParameters&)\n"
                  " or\n"
                  "  (const std::string&)" );

    auto it = builders_.find(ID);
    if( it != builders_.end())
    {
        throw BehaviorTreeException("ID '" + ID + "' already registered");
    }

    NodeBuilder builder = [default_constructable, ID](const std::string& name,  const NodeParameters& params)
    {
        if( default_constructable && params.empty() )
        {
            return std::unique_ptr<TreeNode>(new T(name) );
        }
        if( !param_constructable && !params.empty() )
        {
            throw BehaviorTreeException("Trying to instantiate a TreeNode that can NOT accept NodeParameters in the Constructor: ["
                                        + ID + " / " + name +"]");
        }
        return std::unique_ptr<TreeNode>(new T(name, params) );

    };

    builders_.insert( std::make_pair(ID,builder) );
}


}  // end namespace
#endif  // BT_FACTORY_H
