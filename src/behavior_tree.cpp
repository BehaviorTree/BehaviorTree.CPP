#include "behavior_tree_core/behavior_tree.h"
#include <cstring>

namespace BT
{
void recursiveVisitor(TreeNode* node, std::function<void(TreeNode*)> visitor)
{
    if (!node)
    {
        throw std::runtime_error("One of the children of a DecoratorNode or ControlNode is nulltr");
    }

    visitor(node);

    if (auto control = dynamic_cast<BT::ControlNode*>(node))
    {
        for (const auto& child : control->children())
        {
            recursiveVisitor(child, visitor);
        }
    }
    else if (auto decorator = dynamic_cast<BT::DecoratorNode*>(node))
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

        if (auto control = dynamic_cast<const BT::ControlNode*>(node))
        {
            for (const auto& child : control->children())
            {
                recursivePrint(indent, child);
            }
        }
        else if (auto decorator = dynamic_cast<const BT::DecoratorNode*>(node))
        {
            recursivePrint(indent, decorator->child());
        }
    };

    std::cout << "----------------" << std::endl;
    recursivePrint(0, root_node);
    std::cout << "----------------" << std::endl;
}

void buildSerializedStatusSnapshot(TreeNode *root_node, SerializedTreeStatus& serialized_buffer)
{
    serialized_buffer.clear();

    auto visitor = [ &serialized_buffer ](const TreeNode *node)
    {
        serialized_buffer.push_back( std::make_pair( node->UID(),
                                                     static_cast<uint8_t>( node->status()) ) );
    };

    recursiveVisitor(root_node, visitor);
}

const char *toStr(const NodeStatus &status, bool colored)
{
    if( ! colored ){
        switch (status)
        {
        case NodeStatus::SUCCESS:
            return "SUCCESS";
        case NodeStatus::FAILURE:
            return "FAILURE";
        case NodeStatus::RUNNING:
            return "RUNNING";
        case NodeStatus::IDLE:
            return "IDLE";
        }
    }
    else{
        switch (status)
        {
        case NodeStatus::SUCCESS:
            return ( "\x1b[32m" "SUCCESS" "\x1b[0m"); // RED
        case NodeStatus::FAILURE:
            return ( "\x1b[31m" "FAILURE" "\x1b[0m"); // GREEN
        case NodeStatus::RUNNING:
            return ( "\x1b[33m" "RUNNING" "\x1b[0m"); // YELLOW
        case NodeStatus::IDLE:
            return  ( "\x1b[36m" "IDLE" "\x1b[0m"); // CYAN
        }
    }
    return "Undefined";
}

const char *toStr(const NodeType &type)
{
    switch (type)
    {
    case NodeType::ACTION:
        return "Action";
    case NodeType::CONDITION:
        return "Condition";
    case NodeType::DECORATOR:
        return "Decorator";
    case NodeType::CONTROL:
        return "Control";
    case NodeType::SUBTREE:
        return "SubTree";
    default:
        return "Undefined";
    }
}

}
