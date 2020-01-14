/*  Copyright (C) 2018-2019 Davide Faconti, Eurecat -  All Rights Reserved
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

#include "behaviortree_cpp_v3/bt_factory.h"
#include "behaviortree_cpp_v3/utils/shared_library.h"
#include "behaviortree_cpp_v3/xml_parsing.h"

namespace BT
{
BehaviorTreeFactory::BehaviorTreeFactory()
{
    registerNodeType<FallbackNode>("Fallback");
    registerNodeType<SequenceNode>("Sequence");
    registerNodeType<SequenceStarNode>("SequenceStar");
    registerNodeType<ParallelNode>("Parallel");
    registerNodeType<ReactiveSequence>("ReactiveSequence");
    registerNodeType<ReactiveFallback>("ReactiveFallback");

    registerNodeType<InverterNode>("Inverter");
    registerNodeType<RetryNode>("RetryUntilSuccesful");
    registerNodeType<RepeatNode>("Repeat");
    registerNodeType<TimeoutNode>("Timeout");

    registerNodeType<ForceSuccessNode>("ForceSuccess");
    registerNodeType<ForceFailureNode>("ForceFailure");

    registerNodeType<AlwaysSuccessNode>("AlwaysSuccess");
    registerNodeType<AlwaysFailureNode>("AlwaysFailure");
    registerNodeType<SetBlackboard>("SetBlackboard");

    registerNodeType<DecoratorSubtreeNode>("SubTree");

    registerNodeType<BlackboardPreconditionNode<int>>("BlackboardCheckInt");
    registerNodeType<BlackboardPreconditionNode<double>>("BlackboardCheckDouble");
    registerNodeType<BlackboardPreconditionNode<std::string>>("BlackboardCheckString");

    for( const auto& it: builders_)
    {
        builtin_IDs_.insert( it.first );
    }
}

bool BehaviorTreeFactory::unregisterBuilder(const std::string& ID)
{
    if( builtinNodes().count(ID) )
    {
        throw LogicError("You can not remove the builtin registration ID [", ID, "]");
    }
    auto it = builders_.find(ID);
    if (it == builders_.end())
    {
        return false;
    }
    builders_.erase(ID);
    manifests_.erase(ID);
    return true;
}

void BehaviorTreeFactory::registerBuilder(const TreeNodeManifest& manifest, const NodeBuilder& builder)
{
    auto it = builders_.find( manifest.registration_ID);
    if (it != builders_.end())
    {
        throw BehaviorTreeException("ID [", manifest.registration_ID, "] already registered");
    }

    builders_.insert(  {manifest.registration_ID, builder} );
    manifests_.insert( {manifest.registration_ID, manifest} );
}

void BehaviorTreeFactory::registerSimpleCondition(const std::string& ID,
                                                  const SimpleConditionNode::TickFunctor& tick_functor,
                                                  PortsList ports)
{
    NodeBuilder builder = [tick_functor, ID](const std::string& name, const NodeConfiguration& config) {
        return std::make_unique<SimpleConditionNode>(name, tick_functor, config);
    };

    TreeNodeManifest manifest = { NodeType::CONDITION, ID, std::move(ports) };
    registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerSimpleAction(const std::string& ID,
                                               const SimpleActionNode::TickFunctor& tick_functor,
                                               PortsList ports)
{
    NodeBuilder builder = [tick_functor, ID](const std::string& name, const NodeConfiguration& config) {
        return std::make_unique<SimpleActionNode>(name, tick_functor, config);
    };

    TreeNodeManifest manifest = { NodeType::ACTION, ID, std::move(ports) };
    registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerSimpleDecorator(const std::string& ID,
                                                  const SimpleDecoratorNode::TickFunctor& tick_functor,
                                                  PortsList ports)
{
    NodeBuilder builder = [tick_functor, ID](const std::string& name, const NodeConfiguration& config) {
        return std::make_unique<SimpleDecoratorNode>(name, tick_functor, config);
    };

    TreeNodeManifest manifest = { NodeType::DECORATOR, ID, std::move(ports) };
    registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerFromPlugin(const std::string& file_path)
{
    BT::SharedLibrary loader;
    loader.load(file_path);
    typedef void (*Func)(BehaviorTreeFactory&);

    if (loader.hasSymbol(PLUGIN_SYMBOL))
    {
        Func func = (Func)loader.getSymbol(PLUGIN_SYMBOL);
        func(*this);
    }
    else
    {
        std::cout << "ERROR loading library [" << file_path << "]: can't find symbol ["
                  << PLUGIN_SYMBOL << "]" << std::endl;
    }
}

std::unique_ptr<TreeNode> BehaviorTreeFactory::instantiateTreeNode(
        const std::string& name,
        const std::string& ID,
        const NodeConfiguration& config) const
{
    auto it = builders_.find(ID);
    if (it == builders_.end())
    {
        std::cerr << ID << " not included in this list:" << std::endl;
        for (const auto& builder_it: builders_)
        {
            std::cerr << builder_it.first << std::endl;
        }
        throw RuntimeError("BehaviorTreeFactory: ID [", ID, "] not registered");
    }

    std::unique_ptr<TreeNode> node = it->second(name, config);
    node->setRegistrationID( ID );
    return node;
}

const std::unordered_map<std::string, NodeBuilder> &BehaviorTreeFactory::builders() const
{
    return builders_;
}

const std::unordered_map<std::string,TreeNodeManifest>& BehaviorTreeFactory::manifests() const
{
    return manifests_;
}

const std::set<std::string> &BehaviorTreeFactory::builtinNodes() const
{
    return builtin_IDs_;
}

Tree BehaviorTreeFactory::createTreeFromText(const std::string &text,
                                             Blackboard::Ptr blackboard)
{
    XMLParser parser(*this);
    parser.loadFromText(text);
    auto tree = parser.instantiateTree(blackboard);
    tree.manifests = this->manifests();
    return tree;
}

Tree BehaviorTreeFactory::createTreeFromFile(const std::string &file_path,
                                             Blackboard::Ptr blackboard)
{
    XMLParser parser(*this);
    parser.loadFromFile(file_path);
    auto tree = parser.instantiateTree(blackboard);
    tree.manifests = this->manifests();
    return tree;
}

Tree::~Tree()
{
    if (root_node) {
        haltAllActions(root_node);
    }
}

Blackboard::Ptr Tree::rootBlackboard()
{
    if( blackboard_stack.size() > 0)
    {
        return blackboard_stack.front();
    }
    return {};
}


}   // end namespace
