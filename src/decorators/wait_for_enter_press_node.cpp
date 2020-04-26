/*   Contributed by Indraneel on 26/04/2020
*
*/
#include "behaviortree_cpp_v3/decorators/wait_for_enter_press_node.h"
#include "behaviortree_cpp_v3/action_node.h"

namespace BT
{
WaitForEnterPressNode::WaitForEnterPressNode(const std::string& name, unsigned seconds)
  : DecoratorNode(name, {})
  , sec_(seconds)
  , read_parameter_from_ports_(false)
  , wait_started_(false)
  , timeout(false)
  , keypress(false)
{
    setRegistrationID("WaitForEnterPress");
}

WaitForEnterPressNode::WaitForEnterPressNode(const std::string& name,
                                             const NodeConfiguration& config)
  : DecoratorNode(name, config)
  , sec_(0)
  , read_parameter_from_ports_(true)
  , wait_started_(false)
  , timeout(false)
  , keypress(false)
{
}

void WaitForEnterPressNode::wait(void)
{
    //std::cout<<"Waiting for input key!"<<std::endl;
    std::cin.get();
    //std::cout<<"Received input key!"<<std::endl;
    keypress = true;
    wait_cv_.notify_one();
}

NodeStatus WaitForEnterPressNode::tick()
{
    if (read_parameter_from_ports_)
    {
        if (!getInput("wait_maxsecs", sec_))
        {
            //throw RuntimeError("Missing parameter [wait_maxsecs] in WaitForEnterPressNode");
            std::cout << "WARN: No Max Timeout provided for WaitForEnterPressNode, using infinite "
                         "timeout instead"
                      << std::endl;
            sec_ = -1;   //default value
        }
    }

    if (!wait_started_)
    {
        wait_started_ = true;
        setStatus(NodeStatus::RUNNING);

        if (sec_ > 0)
        {
            std::thread t(&WaitForEnterPressNode::wait, this);
            std::unique_lock<std::mutex> wait_lock(wait_mutex_);
            wait_cv_.wait_for(wait_lock, std::chrono::seconds(sec_),
                              [this] { return keypress || timeout; });
            t.detach();
        }
        else
        {
            std::thread t(&WaitForEnterPressNode::wait, this);
            t.join();
        }
    }

    if (keypress)
    {
        auto child_status = child()->executeTick();
        wait_started_ = false;
        keypress = false;
        return child_status;
    }
    else
    {
        wait_started_ = false;
        return NodeStatus::FAILURE;
    }
}

}   // namespace BT
