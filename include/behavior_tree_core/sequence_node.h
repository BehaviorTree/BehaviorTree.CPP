#ifndef SEQUENCENODE_H
#define SEQUENCENODE_H

#include "behavior_tree_core/control_node.h"

namespace BT
{
class SequenceNode : public ControlNode
{
public:
    // Constructor
    SequenceNode(std::string name);
    ~SequenceNode() = default;

    // The method that is going to be executed by the thread
    virtual BT::ReturnStatus Tick() override;
};
}

#endif
