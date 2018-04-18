#ifndef DECORATORNEGATIONNODE_H
#define DECORATORNEGATIONNODE_H

#include "behavior_tree/control_node.h"

namespace BT
{
    class DecoratorNegationNode : public ControlNode
    {
    public:
        // Constructor
        DecoratorNegationNode(std::string name);
        ~DecoratorNegationNode() = default;

        // The method that is going to be executed by the thread
        void Exec();
        void AddChild(TreeNode* child);
    };
}

#endif
