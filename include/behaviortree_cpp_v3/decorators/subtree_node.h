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
 * @brief The TransparentSubtreeNode is a way to wrap an entire Subtree.
 * It does NOT have a separated BlackBoard and does not need ports remapping.
 */
class SubtreeWrapperNode : public DecoratorNode
{
public:
  SubtreeWrapperNode(const std::string& name);

  virtual ~SubtreeWrapperNode() override = default;

private:
  virtual BT::NodeStatus tick() override;

  virtual NodeType type() const override final
  {
    return NodeType::SUBTREE;
  }
};


}

#endif   // DECORATOR_SUBTREE_NODE_H
