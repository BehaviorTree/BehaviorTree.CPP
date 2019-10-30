#include "behaviortree_cpp_v3/decorators/subtree_node.h"


BT::DecoratorSubtreeNode::DecoratorSubtreeNode(const std::string &name) :
    DecoratorNode(name, {} )
{
    setRegistrationID("SubTree");
}

BT::NodeStatus BT::DecoratorSubtreeNode::tick()
{
    NodeStatus prev_status = status();
    if (prev_status == NodeStatus::IDLE)
    {
        setStatus(NodeStatus::RUNNING);
    }
    return child_node_->executeTick();
}

