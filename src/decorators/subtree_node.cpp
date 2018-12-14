#include "behaviortree_cpp/decorators/subtree_node.h"


BT::DecoratorSubtreeNode::DecoratorSubtreeNode(const std::string &name) :
  DecoratorNode(name, NodeParameters())
{
    setRegistrationName("SubTree");
}

BT::NodeStatus BT::DecoratorSubtreeNode::tick()
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

