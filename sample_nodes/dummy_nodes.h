#ifndef SIMPLE_BT_NODES_H
#define SIMPLE_BT_NODES_H

#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/bt_factory.h"

namespace DummyNodes
{

using BT::NodeStatus;

NodeStatus CheckBattery();

NodeStatus CheckTemperature();
NodeStatus SayHello();

class GripperInterface
{
public:
  GripperInterface() : _opened(true)
  {}

  NodeStatus open();

  NodeStatus close();

private:
  bool _opened;
};

//--------------------------------------

// Example of custom SyncActionNode (synchronous action)
// without ports.
class ApproachObject : public BT::SyncActionNode
{
public:
  ApproachObject(const std::string& name) : BT::SyncActionNode(name, {})
  {}

  // You must override the virtual function tick()
  NodeStatus tick() override;
};

// Example of custom SyncActionNode (synchronous action)
// with an input port.
class SaySomething : public BT::SyncActionNode
{
public:
  SaySomething(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  // You must override the virtual function tick()
  NodeStatus tick() override;

  // It is mandatory to define this static method.
  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("message") };
  }
};

//Same as class SaySomething, but to be registered with SimpleActionNode
NodeStatus SaySomethingSimple(BT::TreeNode& self);

// Example os Asynchronous node that use StatefulActionNode as base class
class SleepNode : public BT::StatefulActionNode
{
public:
  SleepNode(const std::string& name, const BT::NodeConfig& config)
    : BT::StatefulActionNode(name, config)
  {}

  static BT::PortsList providedPorts()
  {
    // amount of milliseconds that we want to sleep
    return { BT::InputPort<int>("msec") };
  }

  NodeStatus onStart() override
  {
    int msec = 0;
    getInput("msec", msec);
    if(msec <= 0)
    {
      // no need to go into the RUNNING state
      return NodeStatus::SUCCESS;
    }
    else
    {
      using namespace std::chrono;
      // once the deadline is reached, we will return SUCCESS.
      deadline_ = system_clock::now() + milliseconds(msec);
      return NodeStatus::RUNNING;
    }
  }

  /// method invoked by an action in the RUNNING state.
  NodeStatus onRunning() override
  {
    if(std::chrono::system_clock::now() >= deadline_)
    {
      return NodeStatus::SUCCESS;
    }
    else
    {
      return NodeStatus::RUNNING;
    }
  }

  void onHalted() override
  {
    // nothing to do here...
    std::cout << "SleepNode interrupted" << std::endl;
  }

private:
  std::chrono::system_clock::time_point deadline_;
};

inline void RegisterNodes(BT::BehaviorTreeFactory& factory)
{
  static GripperInterface grip_singleton;

  factory.registerSimpleCondition("CheckBattery", std::bind(CheckBattery));
  factory.registerSimpleCondition("CheckTemperature", std::bind(CheckTemperature));
  factory.registerSimpleAction("SayHello", std::bind(SayHello));
  factory.registerSimpleAction("OpenGripper",
                               std::bind(&GripperInterface::open, &grip_singleton));
  factory.registerSimpleAction("CloseGripper",
                               std::bind(&GripperInterface::close, &grip_singleton));
  factory.registerNodeType<ApproachObject>("ApproachObject");
  factory.registerNodeType<SaySomething>("SaySomething");
}

}  // namespace DummyNodes

#endif  // SIMPLE_BT_NODES_H
