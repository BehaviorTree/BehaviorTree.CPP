#ifndef DECORATOR_SUBTREE_NODE_H
#define DECORATOR_SUBTREE_NODE_H

#include "behaviortree_cpp/decorator_node.h"

namespace BT
{
/**
 * @brief The SubTreeNode is a way to wrap an entire Subtree,
 * creating a separated BlackBoard.
 * If you want to have data flow through ports, you need to explicitly
 * remap the ports.
 *
 * NOTE: _autoremap will exclude all the ports which name start with underscore '_'
 *
 * Consider this example:

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>

        <Script code="myParam='Hello'" />
        <SubTree ID="Talk" param="{myParam}" />

        <SubTree ID="Talk" param="World" />

        <Script code="param='Auto remapped'" />
        <SubTree ID="Talk" _autoremap="1"  />

        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Talk">
        <SaySomething message="{param}" />
    </BehaviorTree>
</root>

 * You may notice three different approaches to remapping:
 *
 * 1) Subtree: "{param}"  -> Parent: "{myParam}" -> Value: "Hello"
 *    Classical remapping from one port to another, but you need to use the syntax
 *    {myParam} to say that you are remapping the another port.
 *
 * 2) Subtree: "{param}" -> Value: "World"
 *    syntax without {}, in this case param directly point to the __string__ "World".
 *
 * 3) Subtree: "{param}" -> Parent: "{parent}"
 *    Setting to true (or 1) the attribute "_autoremap", we are automatically remapping
 *    each port. Useful to avoid boilerplate.
 */
class SubTreeNode : public DecoratorNode
{
public:
  SubTreeNode(const std::string& name, const NodeConfig& config);

  virtual ~SubTreeNode() override = default;

  static PortsList providedPorts();

  void setSubtreeID(const std::string& ID)
  {
    subtree_id_ = ID;
  }

  const std::string& subtreeID() const
  {
    return subtree_id_;
  }
  virtual BT::NodeStatus tick() override;

  virtual NodeType type() const override final
  {
    return NodeType::SUBTREE;
  }

private:
  std::string subtree_id_;
};

}  // namespace BT

#endif  // DECORATOR_SUBTREE_NODE_H
