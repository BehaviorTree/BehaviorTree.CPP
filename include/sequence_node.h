#ifndef SEQUENCENODE_H
#define SEQUENCENODE_H

#include <control_node.h>

namespace BT
{
class SequenceNode : public ControlNode
{
public:
    // Constructor
    SequenceNode(std::string name);
    ~SequenceNode();
    int DrawType();
    // The method that is going to be executed by the thread
    BT::ReturnStatus Tick();
};
}

#endif
