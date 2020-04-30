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
  , delay_aborted(false)
{
    setRegistrationID("Delay");
}

DelayNode::DelayNode(const std::string& name, const NodeConfiguration& config)
  : DecoratorNode(name, config)
  , msec_(0)
  , read_parameter_from_ports_(true)
  , delay_started_(false)
  , delay_aborted(false)
{
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
        delay_complete = false;
        delay_started_ = true;
        setStatus(NodeStatus::RUNNING);
        if (msec_ > 0)
        {
            timer_id_ = timer_.add(std::chrono::milliseconds(msec_), [this](bool aborted) {
                std::unique_lock<std::mutex> lk(delay_mutex);
                if (!aborted)
                {
                    delay_complete = true;
                }
                else
                {
                    delay_aborted = true;
                }
            });
        }
    }

    std::unique_lock<std::mutex> lk(delay_mutex);

    if (delay_aborted)
    {
        delay_aborted = false;
        delay_started_ = false;
        return NodeStatus::FAILURE;
    }

    else if (delay_complete)
    {
        delay_started_ = false;
        delay_aborted = false;
        auto child_status = child()->executeTick();
        return child_status;
    }
    else
    {
        return NodeStatus::RUNNING;
    }
}

}   // namespace BT
