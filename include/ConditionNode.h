#ifndef CONDITIONNODE_H
#define CONDITIONNODE_H

#include "LeafNode.h"

namespace BT
{
    class ConditionNode : public LeafNode
    {
    public:
        // Constructor
        ConditionNode(std::string Name);
        ~ConditionNode();

        // The method that is going to be executed by the thread
        virtual void Exec() = 0;

        // The method used to interrupt the execution of the node
        bool Halt();

        // Methods used to access the node state without the
        // conditional waiting (only mutual access)
        bool WriteState(NodeState StateToBeSet);
	int GetType();
    };
}

#endif
