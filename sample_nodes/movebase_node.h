#ifndef MOVEBASE_BT_NODES_H
#define MOVEBASE_BT_NODES_H

#include "behaviortree_cpp/behavior_tree.h"

// Custom type
struct Pose2D
{
    double x, y, theta;
};

inline void SleepMS(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

namespace BT
{
// This template specialization is needed only if you want
// to AUTOMATICALLY convert a NodeParameter into a Pose2D
// In other words, implement it if you want to be able to do:
//
//   TreeNode::getParam<Pose2D>(key, ...)
//
template <>
Pose2D convertFromString(StringView key)
{
    // three real numbers separated by semicolons
    auto parts = BT::splitString(key, ';');
    if (parts.size() != 3)
    {
        throw std::runtime_error("invalid input)");
    }
    else
    {
        Pose2D output;
        output.x     = convertFromString<double>(parts[0]);
        output.y     = convertFromString<double>(parts[1]);
        output.theta = convertFromString<double>(parts[2]);
        return output;
    }
}
}

// This is an asynchronous operation that will run in a separate thread.
// It requires the NodeParameter "goal". If the key is not provided, the default
// value "0;0;0" is used instead.

class MoveBaseAction : public BT::AsyncActionNode
{
  public:
    // If your TreeNode requires a NodeParameter, you must define a constructor
    // with this signature.
    MoveBaseAction(const std::string& name, const BT::NodeConfiguration& config)
      : AsyncActionNode(name, config)
    {
    }

    // It is mandatory to define this static method.
    static const BT::PortsList& providedPorts()
    {
        static BT::PortsList ports = {{"goal", BT::PortType::INPUT}};
        return ports;
    }

    BT::NodeStatus tick() override;

    virtual void halt() override;

  private:
    std::atomic_bool _halt_requested;
};

#endif   // MOVEBASE_BT_NODES_H
