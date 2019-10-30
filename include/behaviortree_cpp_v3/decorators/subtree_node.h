#ifndef DECORATOR_SUBTREE_NODE_H
#define DECORATOR_SUBTREE_NODE_H

#include "behaviortree_cpp_v3/decorator_node.h"

namespace BT
{

class DecoratorSubtreeNode : public DecoratorNode
{
  public:
    DecoratorSubtreeNode(const std::string& name);

    virtual ~DecoratorSubtreeNode() override = default;

  private:
    virtual BT::NodeStatus tick() override;

    virtual NodeType type() const override final
    {
        return NodeType::SUBTREE;
    }
};


}

#endif   // DECORATOR_SUBTREE_NODE_H
