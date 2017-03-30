#ifndef PARALLEL_NODE_H
#define PARALLEL_NODE_H

#include <control_node.h>

namespace BT
{
class ParallelNode : public ControlNode
{
public:
    // Constructor
    ParallelNode(std::string name, int threshold_M);
    ~ParallelNode();
    int DrawType();
    // The method that is going to be executed by the thread
    BT::ReturnStatus Tick();
    void Halt();

    unsigned int get_threshold_M();
    void set_threshold_M(unsigned int threshold_M);

private:
    unsigned int threshold_M_;
    unsigned int success_childred_num_;
    unsigned int failure_childred_num_;

};
}
#endif // PARALLEL_NODE_H
