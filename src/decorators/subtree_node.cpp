#include "behaviortree_cpp/decorators/subtree_node.h"

BT::SubTreeNode::SubTreeNode(const std::string& instance_name) :
  DecoratorNode(instance_name, {})
{
  setRegistrationID("SubTree");
}

BT::NodeStatus BT::SubTreeNode::tick()
{
  NodeStatus prev_status = status();
  if (prev_status == NodeStatus::IDLE)
  {
    setStatus(NodeStatus::RUNNING);
  }
  return child_node_->executeTick();
}
