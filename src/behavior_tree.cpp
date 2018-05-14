#include "behavior_tree_core/behavior_tree.h"

namespace BT
{
void recursiveVisitor(const TreeNode* node, const std::function<void(const TreeNode*)> visitor)
{
    if (!node)
    {
        throw std::runtime_error("One of the children of a DecoratorNode or ControlNode is nulltr");
    }

    visitor(node);

    auto control = dynamic_cast<const BT::ControlNode*>(node);
    if (control)
    {
        for (const auto& child : control->children())
        {
            recursiveVisitor(child, visitor);
        }
    }
    auto decorator = dynamic_cast<const BT::DecoratorNode*>(node);
    if (decorator)
    {
        recursiveVisitor(decorator->child(), visitor);
    }
}

void printTreeRecursively(const TreeNode* root_node)
{
    std::function<void(int, const BT::TreeNode*)> recursivePrint;

    recursivePrint = [&recursivePrint](unsigned indent, const BT::TreeNode* node) {
        for (unsigned i = 0; i < indent; i++)
        {
            std::cout << "   ";
        }
        if (!node)
        {
            std::cout << "!nullptr!" << std::endl;
            return;
        }
        std::cout << node->name() << std::endl;
        indent++;
        auto control = dynamic_cast<const BT::ControlNode*>(node);
        if (control)
        {
            for (const auto& child : control->children())
            {
                recursivePrint(indent, child);
            }
        }
        auto decorator = dynamic_cast<const BT::DecoratorNode*>(node);
        if (decorator)
        {
            recursivePrint(indent, decorator->child());
        }
    };

    std::cout << "----------------" << std::endl;
    recursivePrint(0, root_node);
    std::cout << "----------------" << std::endl;
}
}
