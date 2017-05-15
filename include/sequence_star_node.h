#ifndef SEQUENCESTARNODE_H
#define SEQUENCESTARNODE_H

#include <control_node.h>

namespace BT
{
    class SequenceStarNode : public ControlNode
    {
    public:
        // Constructor
        SequenceStarNode(std::string name);
        ~SequenceStarNode();
    int DrawType();
        // The method that is going to be executed by the thread
        void Exec();
    };
}

#endif
