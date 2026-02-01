#include "dummy_nodes.h"

// This function must be implemented in the .cpp file to create
// a plugin that can be loaded at run-time
BT_REGISTER_NODES(factory)
{
  DummyNodes::RegisterNodes(factory);
}

namespace DummyNodes
{

BT::NodeStatus CheckBattery()
{
  std::cout << "[ Battery: OK ]" << std::endl;
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus CheckTemperature()
{
  std::cout << "[ Temperature: OK ]" << std::endl;
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus SayHello()
{
  std::cout << "Robot says: Hello World" << std::endl;
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus GripperInterface::open()
{
  _opened = true;
  std::cout << "GripperInterface::open" << std::endl;
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus GripperInterface::close()
{
  std::cout << "GripperInterface::close" << std::endl;
  _opened = false;
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus ApproachObject::tick()
{
  std::cout << "ApproachObject: " << this->name() << std::endl;
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus SaySomething::tick()
{
  auto msg = getInput<std::string>("message");
  if(!msg)
  {
    throw BT::RuntimeError("missing required input [message]: ", msg.error());
  }

  std::cout << "Robot says: " << msg.value() << std::endl;
  return BT::NodeStatus::SUCCESS;
}

BT::NodeStatus SaySomethingSimple(BT::TreeNode& self)
{
  auto msg = self.getInput<std::string>("message");
  if(!msg)
  {
    throw BT::RuntimeError("missing required input [message]: ", msg.error());
  }

  std::cout << "Robot says: " << msg.value() << std::endl;
  return BT::NodeStatus::SUCCESS;
}

}  // namespace DummyNodes
