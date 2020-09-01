#ifndef DECORATOR_DELAY_NODE_H
#define DECORATOR_DELAY_NODE_H

#include "behaviortree_cpp_v3/decorator_node.h"
#include <atomic>
#include "timer_queue.h"

namespace BT
{
/**
 * @brief The delay node will introduce a delay of a few milliseconds
 * and then tick the child returning the status of the child as it is 
 * upon completion
 * The delay is in milliseconds and it is passed using the port "delay_msec".
 *
 * During the delay the node changes status to RUNNING
 *
 * Example:
 *
 * <Delay delay_msec="5000">
 *    <KeepYourBreath/>
 * </Delay>
 */
class DelayNode : public DecoratorNode
{
  public:
    DelayNode(const std::string& name, unsigned milliseconds);

    DelayNode(const std::string& name, const NodeConfiguration& config);

    ~DelayNode() override
    {
        halt();
    }

    static PortsList providedPorts()
    {
        return {InputPort<unsigned>("delay_msec", "Tick the child after a few milliseconds")};
    }
    void halt() override
    {
        timer_.cancelAll();
        DecoratorNode::halt();
    }

  private:
    TimerQueue timer_;
    uint64_t timer_id_;

    virtual BT::NodeStatus tick() override;

    bool delay_aborted;
    bool delay_complete;
    unsigned msec_;
    bool read_parameter_from_ports_;
    bool delay_started_;
    std::mutex delay_mutex;
};

}   // namespace BT

#endif   // DELAY_NODE_H
