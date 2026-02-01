#pragma once

#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/json_export.h"

// Custom type
struct Pose2D
{
  double x, y, theta;
};

// Add this to you main() to register this function into JsonExporter:
//
// BT::JsonExporter::get().addConverter<Pose2D>();

BT_JSON_CONVERTER(Pose2D, pose)
{
  add_field("x", &pose.x);
  add_field("y", &pose.y);
  add_field("theta", &pose.theta);
}

namespace BT
{
// This template specialization is needed only if you want
// to AUTOMATICALLY convert a NodeParameter into a Pose2D
// In other words, implement it if you want to be able to do:
//
//   TreeNode::getInput<Pose2D>(key, ...)
//
template <>
inline Pose2D convertFromString(StringView key)
{
  // three real numbers separated by semicolons
  auto parts = BT::splitString(key, ';');
  if(parts.size() != 3)
  {
    throw BT::RuntimeError("invalid input)");
  }
  else
  {
    Pose2D output;
    output.x = convertFromString<double>(parts[0]);
    output.y = convertFromString<double>(parts[1]);
    output.theta = convertFromString<double>(parts[2]);
    return output;
  }
}
}  // end namespace BT

namespace chr = std::chrono;

// This is an asynchronous operation
class MoveBaseAction : public BT::StatefulActionNode
{
public:
  // Any TreeNode with ports must have a constructor with this signature
  MoveBaseAction(const std::string& name, const BT::NodeConfig& config)
    : StatefulActionNode(name, config)
  {}

  // It is mandatory to define this static method.
  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<Pose2D>("goal") };
  }

  // this function is invoked once at the beginning.
  BT::NodeStatus onStart() override;

  // If onStart() returned RUNNING, we will keep calling
  // this method until it return something different from RUNNING
  BT::NodeStatus onRunning() override;

  // callback to execute if the action was aborted by another node
  void onHalted() override;

private:
  Pose2D _goal;
  chr::system_clock::time_point _completion_time;
};
