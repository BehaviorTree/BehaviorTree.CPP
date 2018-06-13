#ifndef DECORATORNODE_H
#define DECORATORNODE_H

#include "behavior_tree_core/tree_node.h"

namespace BT
{
class DecoratorNode : public TreeNode
{
  protected:
    TreeNode* child_node_;

  public:
    // Constructor
    DecoratorNode(const std::string& name, const NodeParameters& parameters);

    virtual ~DecoratorNode() override = default;

    // The method used to fill the child vector
    void setChild(TreeNode* child);

    const TreeNode* child() const;
    TreeNode* child();

    // The method used to interrupt the execution of the node
    virtual void halt() override;

    void haltChild();

    virtual NodeType type() const override final
    {
        return NodeType::DECORATOR;
    }
};

}

#endif
