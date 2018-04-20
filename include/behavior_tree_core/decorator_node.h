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
    DecoratorNode(std::string name);
    ~DecoratorNode() = default;

    // The method used to fill the child vector
    void AddChild(TreeNode* child);

    const TreeNode* Child() const;

    // The method used to interrupt the execution of the node
    virtual void Halt() override;

    void HaltChild();

    // Methods used to access the node state without the
    // conditional waiting (only mutual access)
    bool WriteState(NodeStatus new_state);

    virtual NodeType Type() const override final
    {
        return DECORATOR_NODE;
    }
};
}

#endif
