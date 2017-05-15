#ifndef SELECTORSTARNODE_H
#define SELECTORSTARNODE_H

#include <control_node.h>

namespace BT
{
    class SelectorStarNode : public ControlNode
    {
    public:
        // Constructor
        SelectorStarNode(std::string name);
        ~SelectorStarNode();
        int DrawType();
        // The method that is going to be executed by the thread
        void Exec();
    };
}

#endif
