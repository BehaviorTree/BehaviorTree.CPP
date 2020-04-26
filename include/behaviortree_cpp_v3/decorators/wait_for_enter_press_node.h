#ifndef DECORATOR_WAIT_FOR_ENTER_PRESS_NODE_H
#define DECORATOR_WAIT_FOR_ENTER_PRESS_NODE_H

#include "behaviortree_cpp_v3/decorator_node.h"
#include <atomic>
#include "timer_queue.h"

namespace BT
{
/**
 * @brief The Wait for Enter Press node helps get user more control over flow 
 * of the behaviour tree, this node will pause ticking of the child node
 * until the enter key is pressed from the user.
 * 
 * If there is no user input for a long time the node will return failure
 * port "wait_maxsecs" can be used to modify the max wait time of this node
 * which is by default infinite
 *
 *In the meanwhile the status of node will change to running.
 * Example:
 *
 * <WaitForEnterPress wait_maxsecs="15">
 *    <KeepYourBreath/>
 * </WaitForEnterPress>
 */
class WaitForEnterPressNode : public DecoratorNode
{
  public:
    WaitForEnterPressNode(const std::string& name, int seconds);

    WaitForEnterPressNode(const std::string& name, const NodeConfiguration& config);

    ~WaitForEnterPressNode() override
    {
    }

    static PortsList providedPorts()
    {
        return {InputPort<int>("wait_maxsecs", "Max timeout of waiting until enter is pressed "
                                                    "from user")};
    }

  private:
    virtual BT::NodeStatus tick() override;
    void wait(void);

    std::condition_variable wait_cv_;
    int sec_;
    bool read_parameter_from_ports_;
    bool wait_started_;
    std::mutex wait_mutex_;
    bool timeout;
    bool keypress;
};
}   // namespace BT

#endif   // WAIT_FOR_ENTER_PRESS_NODE_H
