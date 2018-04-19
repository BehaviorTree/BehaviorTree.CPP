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
        ~ConditionNode() = default;

        // The method used to interrupt the execution of the node
        virtual void Halt() override;

        // Methods used to access the node state without the
        // conditional waiting (only mutual access)
        bool WriteState(ReturnStatus new_state);

        virtual NodeType Type() const override { return CONDITION_NODE; }

    };
}

#endif
