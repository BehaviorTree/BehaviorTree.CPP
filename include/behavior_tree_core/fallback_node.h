#ifndef FALLBACKNODE_H
#define FALLBACKNODE_H

#include "behavior_tree_core/control_node.h"

namespace BT
{
    class FallbackNode : public ControlNode
    {
    public:
        // Constructor
        FallbackNode(std::string name);
        ~FallbackNode() = default;

        // The method that is going to be executed by the thread
        virtual BT::ReturnStatus Tick() override;
    };
}

#endif
