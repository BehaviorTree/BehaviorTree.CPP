#include "behaviortree_cpp/decorators/subtree_node.h"

BT::SubTreeNode::SubTreeNode(const std::string& name, const NodeConfig& config)
  : DecoratorNode(name, config)
{
  setRegistrationID("SubTree");
}

BT::PortsList BT::SubTreeNode::providedPorts()
{
  auto port =
      PortInfo(PortDirection::INPUT, typeid(bool), GetAnyFromStringFunctor<bool>());
  port.setDefaultValue(false);
  port.setDescription("If true, all the ports with the same name "
                      "will be remapped");

  return { { "_autoremap", port } };
}

BT::NodeStatus BT::SubTreeNode::tick()
{
  NodeStatus prev_status = status();
  if(prev_status == NodeStatus::IDLE)
  {
    setStatus(NodeStatus::RUNNING);
  }
  const NodeStatus child_status = child_node_->executeTick();
  if(isStatusCompleted(child_status))
  {
    resetChild();
  }

  return child_status;
}
