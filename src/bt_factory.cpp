/*  Copyright (C) 2018 Davide Faconti -  All Rights Reserved
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

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/shared_library.h"

namespace BT
{
BehaviorTreeFactory::BehaviorTreeFactory()
{
    registerNodeType<FallbackNode>("Fallback");
    registerNodeType<FallbackStarNode>("FallbackStar");
    registerNodeType<SequenceNode>("Sequence");
    registerNodeType<SequenceStarNode>("SequenceStar");
    registerNodeType<ParallelNode>("ParallelNode");

    registerNodeType<InverterNode>("Inverter");
    registerNodeType<RetryNode>("RetryUntilSuccesful");
    registerNodeType<RepeatNode>("Repeat");
    registerNodeType<TimeoutNode>("Timeout");

    registerNodeType<ForceSuccessDecorator>("ForceSuccess");
    registerNodeType<ForceFailureDecorator>("ForceFailure");

    registerNodeType<AlwaysSuccess>("AlwaysSuccess");
    registerNodeType<AlwaysFailure>("AlwaysFailure");
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
    auto it = builders_.find(ID);
    if (it == builders_.end())
    {
        return false;
    }
    builders_.erase(ID);
    return true;
}

void BehaviorTreeFactory::registerBuilder(const TreeNodeManifest& manifest, NodeBuilder builder)
{
    auto it = builders_.find( manifest.registration_ID);
    if (it != builders_.end())
    {
        throw BehaviorTreeException("ID '" + manifest.registration_ID + "' already registered");
    }

    builders_.insert(std::make_pair(manifest.registration_ID, builder));
    manifests_.push_back(manifest);
    sortTreeNodeManifests();
}

void BehaviorTreeFactory::registerSimpleCondition(
    const std::string& ID, const SimpleConditionNode::TickFunctor& tick_functor)
{
    NodeBuilder builder = [tick_functor, ID](const std::string& name, const NodeParameters& params) {
        return std::unique_ptr<TreeNode>(new SimpleConditionNode(name, tick_functor, params));
    };

    TreeNodeManifest manifest = { NodeType::CONDITION, ID, NodeParameters() };
    registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerSimpleAction(const std::string& ID,
                                               const SimpleActionNode::TickFunctor& tick_functor)
{
    NodeBuilder builder = [tick_functor, ID](const std::string& name, const NodeParameters& params) {
        return std::unique_ptr<TreeNode>(new SimpleActionNode(name, tick_functor, params));
    };

    TreeNodeManifest manifest = { NodeType::ACTION, ID, NodeParameters() };
    registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerSimpleDecorator(
    const std::string& ID, const SimpleDecoratorNode::TickFunctor& tick_functor)
{
    NodeBuilder builder = [tick_functor, ID](const std::string& name, const NodeParameters& params) {
        return std::unique_ptr<TreeNode>(new SimpleDecoratorNode(name, tick_functor, params));
    };

    TreeNodeManifest manifest = { NodeType::DECORATOR, ID, NodeParameters() };
    registerBuilder(manifest, builder);
}

void BehaviorTreeFactory::registerFromPlugin(const std::string file_path)
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
        const std::string& ID, const std::string& name,
        const NodeParameters& params,
        const Blackboard::Ptr& blackboard) const
{
    auto it = builders_.find(ID);
    if (it == builders_.end())
    {
        std::cerr << ID << " not included in this list:" << std::endl;
        for (const auto& it: builders_)
        {
            std::cerr << it.first << std::endl;
        }
        throw std::invalid_argument("ID '" + ID + "' not registered");
    }
    std::unique_ptr<TreeNode> node = it->second(name, params);
    node->setRegistrationName(ID);
    node->setBlackboard(blackboard);
    node->initializeOnce();

    return node;
}

const std::map<std::string, NodeBuilder>& BehaviorTreeFactory::builders() const
{
    return builders_;
}

const std::vector<TreeNodeManifest>& BehaviorTreeFactory::manifests() const
{
    return manifests_;
}

const std::set<std::string> &BehaviorTreeFactory::builtinNodes() const
{
    return builtin_IDs_;
}

void BehaviorTreeFactory::sortTreeNodeManifests()
{
    std::sort(manifests_.begin(), manifests_.end(),
              [](const TreeNodeManifest& a, const TreeNodeManifest& b) {
                  int comp = std::strcmp(toStr(a.type), toStr(b.type));
                  if (comp == 0)
                  {
                      return a.registration_ID < b.registration_ID;
                  }
                  return comp < 0;
              });
}

}   // end namespace
