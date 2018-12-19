#ifndef DECORATOR_TIMEOUT_NODE_H
#define DECORATOR_TIMEOUT_NODE_H

#include "behaviortree_cpp/decorator_node.h"
#include <atomic>
#include "timer_queue.h"

namespace BT
{
class TimeoutNode : public DecoratorNode
{
  public:
    TimeoutNode(const std::string& name, unsigned milliseconds);

    TimeoutNode(const std::string& name, const NodeConfiguration& config);

    static const PortsList& providedPorts()
    {
        static PortsList ports = {{"msec"}};
        return ports;
    }

  private:
    static TimerQueue& timer()
    {
        static TimerQueue timer_queue;
        return timer_queue;
    }

    virtual BT::NodeStatus tick() override;

    std::atomic<bool> child_halted_;
    uint64_t timer_id_;

    unsigned msec_;
    bool read_parameter_from_ports_;
};
}

#endif   // DEADLINE_NODE_H
