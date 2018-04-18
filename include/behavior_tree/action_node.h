#ifndef BEHAVIORTREECORE_ACTIONNODE_H
#define BEHAVIORTREECORE_ACTIONNODE_H

#include "leaf_node.h"

namespace BT
{

class ActionNode : public LeafNode
{
public:
    // Constructor
    ActionNode(std::string name);
    ~ActionNode() = default;

    // The method that is going to be executed by the thread
    void WaitForTick();

    // Methods used to access the node state without the
    // conditional waiting (only mutual access)
    bool WriteState(ReturnStatus new_state);

    virtual NodeType Type() const override { return ACTION_NODE; }

protected:
    // The thread that will execute the node
    std::thread thread_;

};
}

#endif
