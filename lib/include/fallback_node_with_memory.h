#ifndef FALLBACK_NODE_WITH_MEMORY_H
#define FALLBACK_NODE_WITH_MEMORY_H
#include <control_node.h>


namespace BT
{
class FallbackNodeWithMemory : public ControlNode
{
public:
    // Constructor
    FallbackNodeWithMemory(std::string name);
    FallbackNodeWithMemory(std::string name, int reset_policy);
    ~FallbackNodeWithMemory();
    int DrawType();
    // The method that is going to be executed by the thread
    BT::ReturnStatus Tick();
    void Halt();
private:
    unsigned int current_child_idx_;
    unsigned int reset_policy_;

};
}


#endif // FALLBACK_NODE_WITH_MEMORY_H
