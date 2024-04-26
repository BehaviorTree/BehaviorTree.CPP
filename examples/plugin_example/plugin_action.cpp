#include "custom_type.hpp"
#include "behaviortree_cpp/bt_factory.h"

class PrintVector : public BT::SyncActionNode
{
public:
  PrintVector(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    const auto v = getInput<Vector4D>("value").value();
    printf("x:%f  y:%f  z:%f  w:%f\n", v.x, v.y, v.z, v.w);
    return BT::NodeStatus::SUCCESS;
  }

  // It is mandatory to define this static method.
  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<Vector4D>("value") };
  }
};

// Function used to register PrintVector automatically, when
// loading the plugin.
// Remember that it is mandatory to add to the CMakeLists.txt file this:
//
//    target_compile_definitions(<target_name> PRIVATE  BT_PLUGIN_EXPORT)
//
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<PrintVector>("PrintVector");
}
