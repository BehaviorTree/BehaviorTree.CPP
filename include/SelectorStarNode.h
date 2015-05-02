#ifndef SELECTORSTARNODE_H
#define SELECTORSTARNODE_H

#include <ControlNode.h>

namespace BT
{
    class SelectorStarNode : public ControlNode
    {
    public:
        // Constructor
        SelectorStarNode(std::string Name);
        ~SelectorStarNode();
        int GetType();
        // The method that is going to be executed by the thread
        void Exec();
    };
}

#endif
