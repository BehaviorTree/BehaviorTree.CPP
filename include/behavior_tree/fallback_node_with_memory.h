#ifndef FALLBACK_NODE_WITH_MEMORY_H
#define FALLBACK_NODE_WITH_MEMORY_H
#include "behavior_tree/control_node.h"


namespace BT
{
class FallbackNodeWithMemory : public ControlNode
{
public:
    // Constructor
    FallbackNodeWithMemory(std::string name);
    FallbackNodeWithMemory(std::string name, int reset_policy);
    ~FallbackNodeWithMemory() = default;

    // The method that is going to be executed by the thread
    virtual BT::ReturnStatus Tick() override;
    virtual void Halt() override;
private:
    unsigned int current_child_idx_;
    unsigned int reset_policy_;

};
}


#endif // FALLBACK_NODE_WITH_MEMORY_H
