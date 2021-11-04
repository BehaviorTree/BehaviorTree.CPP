#ifndef DECORATORKEEPTRYINGNODE_H
#define DECORATORKEEPTRYINGNODE_H

#include "behaviortree_cpp_v3/decorator_node.h"

namespace BT
{
/**
 * @brief The KeepTryingNode is used to execute a child several times if it fails.
 *
 * If the child returns SUCCESS, the loop is stopped and this node
 * returns SUCCESS.
 *
 * If the child returns FAILURE, this node will try again up to N times. One retry per tick
 * (N is read from port "num_attempts").
 *
 * Example:
 *
 * <KeepTryingUntilSuccessful num_attempts="3">
 *     <OpenDoor/>
 * </KeepTryingUntilSuccessful>
 */
class KeepTryingNode : public DecoratorNode
{
  public:
    
    KeepTryingNode(const std::string& name, int NTries);

    KeepTryingNode(const std::string& name, const NodeConfiguration& config);

    virtual ~KeepTryingNode() override = default;

    static PortsList providedPorts()
    {
        return { InputPort<int>(NUM_ATTEMPTS,
                               "Execute again a failing child up to N times. "
                               "Use -1 to create an infinite loop.") };
    }

    virtual void halt() override;

  private:
    int max_attempts_;
    int try_index_;

    bool read_parameter_from_ports_;
    static constexpr const char* NUM_ATTEMPTS = "num_attempts";

    virtual BT::NodeStatus tick() override;
};
}

#endif