#include "behaviortree_cpp_v3/decorators/subtree_node.h"


BT::SubtreeNode::SubtreeNode(const std::string &name) :
    DecoratorNode(name, {} )
{
    setRegistrationID("SubTree");
}

BT::NodeStatus BT::SubtreeNode::tick()
{
    NodeStatus prev_status = status();
    if (prev_status == NodeStatus::IDLE)
    {
        setStatus(NodeStatus::RUNNING);
    }
    return child_node_->executeTick();
}

BT::SubtreeWrapperNode::SubtreeWrapperNode(const std::string &name) :
     DecoratorNode(name, {} )
{
  setRegistrationID("TransparentSubtree");
}

BT::NodeStatus BT::SubtreeWrapperNode::tick()
{
    NodeStatus prev_status = status();
    if (prev_status == NodeStatus::IDLE)
    {
        setStatus(NodeStatus::RUNNING);
    }
    return child_node_->executeTick();
}

