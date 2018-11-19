#ifndef DECORATOR_SUBTREE_NODE_H
#define DECORATOR_SUBTREE_NODE_H

#include "behaviortree_cpp/decorator_node.h"

namespace BT
{
class DecoratorSubtreeNode : public DecoratorNode
{
  public:
    DecoratorSubtreeNode(const std::string& name) : DecoratorNode(name, NodeParameters())
    {
    }

    virtual ~DecoratorSubtreeNode() override = default;

  private:
    virtual BT::NodeStatus tick() override
    {
        NodeStatus prev_status = status();
        if (prev_status == NodeStatus::IDLE)
        {
            setStatus(NodeStatus::RUNNING);
        }
        auto status = child_node_->executeTick();
        setStatus(status);

        // reset child if completed
        if( status == NodeStatus::SUCCESS || status == NodeStatus::FAILURE)
        {
            child_node_->setStatus(NodeStatus::IDLE);
        }
        return status;
    }
};
}

#endif   // DECORATOR_SUBTREE_NODE_H
