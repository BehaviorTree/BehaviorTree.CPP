#include <fuzzer/FuzzedDataProvider.h>
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/xml_parsing.h"
#include <string>

// List of valid node types we can use to construct valid-ish XML
constexpr const char* NODE_TYPES[] = {
  "Sequence",         "Fallback",         "ParallelAll",
  "ReactiveSequence", "ReactiveFallback", "IfThenElse",
  "WhileDoElse",      "Inverter",         "RetryUntilSuccessful",
  "Repeat",           "Timeout",          "Delay",
  "ForceSuccess",     "ForceFailure",     "AlwaysSuccess",
  "AlwaysFailure",    "SetBlackboard",    "SubTree"
};

// Attributes that can be added to nodes
constexpr const char* NODE_ATTRIBUTES[] = { "name",      "ID",         "port_1",
                                            "port_2",    "timeout_ms", "delay_ms",
                                            "threshold", "max_repeats" };

std::string generateFuzzedNodeXML(FuzzedDataProvider& fdp, int depth = 0)
{
  // Prevent stack overflow with max depth
  if(depth > 6)
  {  // Reasonable limit for XML tree depth
    return "<AlwaysSuccess/>";
  }

  std::string xml;
  const std::string node_type = fdp.PickValueInArray(NODE_TYPES);

  xml += "<" + node_type;

  size_t num_attributes = fdp.ConsumeIntegralInRange<size_t>(0, 3);
  for(size_t i = 0; i < num_attributes; i++)
  {
    const std::string attr = fdp.PickValueInArray(NODE_ATTRIBUTES);
    std::string value = fdp.ConsumeRandomLengthString(10);
    xml += " " + attr + "=\"" + value + "\"";
  }

  if(depth > 3 || fdp.ConsumeBool())
  {
    xml += "/>";
  }
  else
  {
    xml += ">";
    // Add some child nodes recursively with depth limit
    size_t num_children = fdp.ConsumeIntegralInRange<size_t>(0, 2);
    for(size_t i = 0; i < num_children; i++)
    {
      xml += generateFuzzedNodeXML(fdp, depth + 1);
    }
    xml += "</" + node_type + ">";
  }

  return xml;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 4)
  {
    return 0;
  }

  FuzzedDataProvider fdp(data, size);
  BT::BehaviorTreeFactory factory;

  try
  {
    // Strategy 1: Test with completely random data
    if(fdp.ConsumeBool())
    {
      std::string random_xml = fdp.ConsumeRandomLengthString(size - 1);
      try
      {
        factory.createTreeFromText(random_xml);
      }
      catch(const std::exception&)
      {}
    }
    // Strategy 2: Generate semi-valid XML
    else
    {
      std::string xml = R"(
                <root BTCPP_format="4">
                    <BehaviorTree ID="MainTree">)";

      size_t num_nodes = fdp.ConsumeIntegralInRange<size_t>(1, 5);
      for(size_t i = 0; i < num_nodes; i++)
      {
        xml += generateFuzzedNodeXML(fdp);
      }

      xml += R"(
                    </BehaviorTree>
                </root>)";

      auto blackboard = BT::Blackboard::create();

      switch(fdp.ConsumeIntegralInRange<int>(0, 2))
      {
        case 0:
          factory.createTreeFromText(xml, blackboard);
          break;
        case 1:
          BT::VerifyXML(xml, {});
          break;
        case 2:
          factory.registerBehaviorTreeFromText(xml);
          if(!factory.registeredBehaviorTrees().empty())
          {
            factory.createTree(factory.registeredBehaviorTrees().front(), blackboard);
          }
          break;
      }
    }
  }
  catch(const std::exception&)
  {}

  return 0;
}
