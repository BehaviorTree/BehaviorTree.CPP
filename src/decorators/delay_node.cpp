/*  Contributed by Indraneel on 26/04/2020
*/
#include "behaviortree_cpp_v3/decorators/delay_node.h"
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
DelayNode::DelayNode(const std::string& name, unsigned milliseconds)
  : DecoratorNode(name, {})
  , msec_(milliseconds)
  , read_parameter_from_ports_(false)
  , delay_started_(false)
{
    setRegistrationID("Delay");
}

DelayNode::DelayNode(const std::string& name, const NodeConfiguration& config)
  : DecoratorNode(name, config), msec_(0), read_parameter_from_ports_(true), delay_started_(false)
{
}

void DelayNode::delay(void)
{
    auto now = std::chrono::steady_clock::now();
    delay_mutex.try_lock_until(now + std::chrono::milliseconds(msec_));
}

NodeStatus DelayNode::tick()
{
    if (read_parameter_from_ports_)
    {
        if (!getInput("delay_msec", msec_))
        {
            throw RuntimeError("Missing parameter [delay_msec] in DelayNode");
        }
    }

    if (!delay_started_)
    {
        delay_started_ = true;
        setStatus(NodeStatus::RUNNING);
        if (msec_ > 0)
        {
            std::lock_guard<std::timed_mutex> l(delay_mutex);
            std::thread t(&DelayNode::delay, this);
            t.join();
        }
    }

    delay_started_ = false;
    auto child_status = child()->executeTick();
    return child_status;
}

}   // namespace BT
