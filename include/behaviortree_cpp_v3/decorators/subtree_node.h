#ifndef DECORATOR_SUBTREE_NODE_H
#define DECORATOR_SUBTREE_NODE_H

#include "behaviortree_cpp_v3/decorator_node.h"

namespace BT
{

/**
 * @brief The SubtreeNode is a way to wrap an entire Subtree,
 * creating a separated BlackBoard.
 * If you want to have data flow through ports, you need to explicitly
 * remap the ports.
 */
class SubtreeNode : public DecoratorNode
{
  public:
    SubtreeNode(const std::string& name);

    virtual ~SubtreeNode() override = default;

  private:
    virtual BT::NodeStatus tick() override;

    virtual NodeType type() const override final
    {
        return NodeType::SUBTREE;
    }
};

/**
 * @brief The SubtreePlus is a new kind of subtree that gives you much more control over remapping:
 *
 * Consider this example:

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>

        <SetBlackboard value="Hello" output_key="myParam" />
        <SubTreePlus ID="Talk" param="{myParam}" />

        <SubTreePlus ID="Talk" param="World" />

        <SetBlackboard value="Auto remapped" output_key="param" />
        <SubTreePlus ID="Talk" __autoremap="1"  />

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
 *    syntax without {}, in this case param directly point to the string "World".
 *
 * 3) Subtree: "{param}" -> Parent: "{parent}"
 *    Setting to true (or 1) the attribute "__autoremap", we are automatically remapping
 *    each port. Usefull to avoid some boilerplate.

 */
class SubtreePlusNode : public DecoratorNode
{
public:
  SubtreePlusNode(const std::string& name);

  virtual ~SubtreePlusNode() override = default;

private:
  virtual BT::NodeStatus tick() override;

  virtual NodeType type() const override final
  {
    return NodeType::SUBTREE;
  }
};



}

#endif   // DECORATOR_SUBTREE_NODE_H
