#ifndef ACTIONNODE_H
#define ACTIONNODE_H

#include "LeafNode.h"

namespace BT
{

    class ActionNode : public LeafNode
    {
    public:
        // Constructor
        ActionNode(std::string Name);
        ~ActionNode();

        // The method that is going to be executed by the thread
        virtual void Exec() = 0;

        // The method used to interrupt the execution of the node
        virtual bool Halt() = 0;

        // Methods used to access the node state without the
        // conditional waiting (only mutual access)
        bool WriteState(NodeState StateToBeSet);
	int GetType();
    };
}

#endif
