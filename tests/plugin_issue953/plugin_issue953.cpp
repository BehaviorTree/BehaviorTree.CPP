/**
 * Plugin for testing Issue #953: convertFromString in plugins
 *
 * This plugin defines a custom type (Issue953Type) with its convertFromString
 * specialization ONLY in this file. The test executable that loads this plugin
 * does NOT have access to the convertFromString specialization.
 *
 * Before the fix: getInput<Issue953Type>() would fail because the executor
 * couldn't find the convertFromString specialization.
 *
 * After the fix: getInput<Issue953Type>() works because the StringConverter
 * is captured in PortInfo when InputPort<Issue953Type>() is called (here in
 * the plugin), and getInput() uses that stored converter.
 */

#include "behaviortree_cpp/bt_factory.h"

// Custom type defined ONLY in the plugin
struct Issue953Type
{
  int id = 0;
  std::string name;
  double value = 0.0;
};

// convertFromString specialization ONLY in the plugin - not visible to executor
namespace BT
{
template <>
inline Issue953Type convertFromString(StringView str)
{
  // Format: "id;name;value" e.g., "42;test;3.14"
  const auto parts = BT::splitString(str, ';');
  if(parts.size() != 3)
  {
    throw BT::RuntimeError("Invalid Issue953Type format. Expected: id;name;value");
  }

  Issue953Type result;
  result.id = convertFromString<int>(parts[0]);
  result.name = std::string(parts[1]);
  result.value = convertFromString<double>(parts[2]);
  return result;
}
}  // namespace BT

// Action node that uses Issue953Type
class Issue953Action : public BT::SyncActionNode
{
public:
  Issue953Action(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {}

  BT::NodeStatus tick() override
  {
    // This getInput call relies on the stored StringConverter from PortInfo
    // because the executor doesn't have the convertFromString specialization
    auto result = getInput<Issue953Type>("input");
    if(!result)
    {
      std::cerr << "getInput failed: " << result.error() << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    const auto& data = result.value();

    // Store results in output ports so the test can verify them
    setOutput("out_id", data.id);
    setOutput("out_name", data.name);
    setOutput("out_value", data.value);

    return BT::NodeStatus::SUCCESS;
  }

  static BT::PortsList providedPorts()
  {
    // When InputPort<Issue953Type>() is called here (in the plugin),
    // GetAnyFromStringFunctor<Issue953Type>() captures the convertFromString
    // specialization that IS visible in this compilation unit.
    return { BT::InputPort<Issue953Type>("input", "Input in format: id;name;value"),
             BT::OutputPort<int>("out_id", "{out_id}", "Parsed ID"),
             BT::OutputPort<std::string>("out_name", "{out_name}", "Parsed name"),
             BT::OutputPort<double>("out_value", "{out_value}", "Parsed value") };
  }
};

// Register the node when plugin is loaded
BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<Issue953Action>("Issue953Action");
}
