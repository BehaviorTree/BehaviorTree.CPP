#ifndef FALLBACKNODE_H
#define FALLBACKNODE_H

#include <control_node.h>

namespace BT
{
    class FallbackNode : public ControlNode
    {
    public:
        // Constructor
        FallbackNode(std::string name);
        ~FallbackNode();
        int DrawType();
        // The method that is going to be executed by the thread
        BT::ReturnStatus Tick();
    };
}

#endif
