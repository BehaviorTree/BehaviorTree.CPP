#ifndef SELECTORNODE_H
#define SELECTORNODE_H

#include <ControlNode.h>

namespace BT
{
    class SelectorNode : public ControlNode
    {
    public:
        // Constructor
        SelectorNode(std::string Name);
        ~SelectorNode();
	int GetType();
        // The method that is going to be executed by the thread
        void Exec();
    };
}

#endif
