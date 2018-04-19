#include "behavior_tree_core/bt_factory.h"

namespace BT
{
BehaviorTreeFactory::BehaviorTreeFactory()
{
}

bool BehaviorTreeFactory::unregisterBuilder(const std::string &ID)
{
    auto it = builders_.find(ID);
    if( it == builders_.end())
    {
        return false;
    }
    builders_.erase(ID);
    return true;
}

void BehaviorTreeFactory::registerSimpleAction(const std::string &ID,
                                               std::function<ReturnStatus()> tick_functor)
{

}

void BehaviorTreeFactory::registerSimpleDecorator(const std::string &ID,
                                                  std::function<ReturnStatus(ReturnStatus)> tick_functor)
{

}

std::unique_ptr<TreeNode> BehaviorTreeFactory::instantiateTreeNode(const std::string &ID,
                                                                   const NodeParameters &params)
{
    auto it = builders_.find(ID);
    if( it == builders_.end())
    {
        throw BehaviorTreeException("ID '" + ID + "' not registered");
    }
    return it->second(ID, params);
}

}  // end namespace
