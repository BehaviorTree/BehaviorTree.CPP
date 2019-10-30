#ifndef DECORATOR_TIMEOUT_NODE_H
#define DECORATOR_TIMEOUT_NODE_H

#include "behaviortree_cpp_v3/decorator_node.h"
#include <atomic>
#include "timer_queue.h"

namespace BT
{
/**
 * @brief The TimeoutNode will halt() a running child if
 * the latter has been RUNNING for more than a give time.
 * The timeout is in millisecons and it is passed using the port "msec".
 *
 * If timeout is reached it returns FAILURE.
 *
 * Example:
 *
 * <Timeout msec="5000">
 *    <KeepYourBreath/>
 * </Timeout>
 */
class TimeoutNode : public DecoratorNode
{
  public:
    TimeoutNode(const std::string& name, unsigned milliseconds);

    TimeoutNode(const std::string& name, const NodeConfiguration& config);

    ~TimeoutNode() override
    {
        timer_.cancelAll();
    }

    static PortsList providedPorts()
    {
        return { InputPort<unsigned>("msec", "After a certain amount of time, "
                                             "halt() the child if it is still running.") };
    }

  private:
    TimerQueue timer_ ;

    virtual BT::NodeStatus tick() override;

    std::atomic<bool> child_halted_;
    uint64_t timer_id_;

    unsigned msec_;
    bool read_parameter_from_ports_;
    bool timeout_started_;
    std::mutex timeout_mutex_;
};
}

#endif   // DEADLINE_NODE_H
