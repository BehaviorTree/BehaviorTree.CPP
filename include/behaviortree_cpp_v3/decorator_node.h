#ifndef DECORATORNODE_H
#define DECORATORNODE_H

#include "behaviortree_cpp_v3/tree_node.h"

namespace BT
{
class DecoratorNode : public TreeNode
{
  protected:
    TreeNode* child_node_;

  public:

    DecoratorNode(const std::string& name, const NodeConfiguration& config);

    virtual ~DecoratorNode() override = default;

    void setChild(TreeNode* child);

    const TreeNode* child() const;

    TreeNode* child();

    /// The method used to interrupt the execution of this node
    virtual void halt() override;

    /// Halt() the child node
    void haltChild();

    virtual NodeType type() const override
    {
        return NodeType::DECORATOR;
    }

    NodeStatus executeTick() override;
};

/**
 * @brief The SimpleDecoratorNode provides an easy to use DecoratorNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus functionName(BT::NodeStatus child_status)
 *
 * This avoids the hassle of inheriting from a DecoratorNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method.
 * SimpleDecoratorNode does not support halting, NodeParameters, nor Blackboards.
 */
class SimpleDecoratorNode : public DecoratorNode
{
  public:
    typedef std::function<NodeStatus(NodeStatus, TreeNode&)> TickFunctor;

    // You must provide the function to call when tick() is invoked
    SimpleDecoratorNode(const std::string& name, TickFunctor tick_functor, const NodeConfiguration& config);

    ~SimpleDecoratorNode() override = default;

  protected:
    virtual NodeStatus tick() override;

    TickFunctor tick_functor_;
};
}

#endif
