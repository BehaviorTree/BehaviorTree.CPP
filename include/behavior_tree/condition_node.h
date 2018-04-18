#ifndef CONDITIONNODE_H
#define CONDITIONNODE_H

#include "leaf_node.h"

namespace BT
{
    class ConditionNode : public LeafNode
    {
    public:
        // Constructor
        ConditionNode(std::string name);
        ~ConditionNode();

        // The method that is going to be executed by the thread
        virtual BT::ReturnStatus Tick() = 0;

        // The method used to interrupt the execution of the node
        void Halt();

        // Methods used to access the node state without the
        // conditional waiting (only mutual access)
        bool WriteState(ReturnStatus new_state);
    int DrawType();
    };
}

#endif
