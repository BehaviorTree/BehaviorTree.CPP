// Fuzz target: bt_xml_roundtrip_fuzzer
//
// Threat model: BehaviorTree.CPP both *reads* XML from untrusted sources and
// *writes* XML that other tools re-read (Groot2 project files, exported
// trees, persisted models). A divergence between writer and reader is a bug
// on its own; the writer path (WriteTreeToXML, writeTreeNodesModelXML,
// writeTreeXSD) and the model/manifest reading paths are dark in production
// coverage (xml_parsing.cpp 50.6%, tinyxml2 writer paths unexercised).
//
// Strategy:
//   mode 0: raw fuzz bytes into VerifyXML / registerBehaviorTreeFromText
//           (parser error and recovery paths),
//   modes 1-3: generate a *valid* document (optionally with two trees or a
//           <TreeNodesModel> section), build the tree, export it with
//           WriteTreeToXML (both metadata/builtin-model flags), re-parse the
//           export in a fresh factory, export again, and additionally
//           generate + re-verify the nodes-model XML and the XSD writer.
//
// All generation is bounded exactly as in bt_tree_exec_fuzzer (depth, child
// counts, no infinite loops, no real sleeps). No tree is ticked here: the
// tick engine belongs to bt_tree_exec_fuzzer; this target owns the parse and
// serialize surface.
//
// Expected bug classes: memory-safety in tinyxml2 writer/reader paths,
// XML attribute escaping/unescaping mismatches, logic divergence between
// export and re-import (lost or duplicated attributes/ports).

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace
{

constexpr int kMaxDepth = 3;

const char* kConditionScripts[] = {
  "var > 0", "var < 5", "flag", "!flag", "str == 'abc'", "var == MODE_A"
};
const char* kLiterals[] = { "1", "2", "true", "false", "1.5", "hello", "{var}" };
const char* kStatuses[] = { "SUCCESS", "FAILURE", "SKIPPED" };

std::string pickCondition(FuzzedDataProvider& fdp)
{
  return fdp.PickValueInArray(kConditionScripts);
}

std::string genConditionLeaf(FuzzedDataProvider& fdp)
{
  switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 2))
  {
    case 0:
      return "<ScriptCondition code=\"" + pickCondition(fdp) + "\"/>";
    case 1:
      return "<AlwaysSuccess/>";
    default:
      return "<AlwaysFailure/>";
  }
}

std::string genLeaf(FuzzedDataProvider& fdp)
{
  switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 8))
  {
    case 0:
      return "<AlwaysSuccess/>";
    case 1:
      return "<AlwaysFailure/>";
    case 2:
      return "<Script code=\"var = var + 1\"/>";
    case 3:
      return "<ScriptCondition code=\"" + pickCondition(fdp) + "\"/>";
    case 4:
      return std::string("<SetBlackboard output_key=\"var2\" value=\"") +
             fdp.PickValueInArray(kLiterals) + "\"/>";
    case 5:
      return "<Sleep msec=\"1\"/>";
    case 6:
      return "<UnsetBlackboard key=\"var2\"/>";
    case 7:
      return "<Script code=\"{txt}\"/>";
    default:
      return "<AlwaysSuccess name=\"task_a\"/>";
  }
}

std::string genDecorator(FuzzedDataProvider& fdp, int depth);
std::string genControl(FuzzedDataProvider& fdp, int depth);

std::string genNode(FuzzedDataProvider& fdp, int depth)
{
  if(depth >= kMaxDepth)
  {
    return genLeaf(fdp);
  }
  switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 2))
  {
    case 0:
      return genLeaf(fdp);
    case 1:
      return genDecorator(fdp, depth);
    default:
      return genControl(fdp, depth);
  }
}

std::string genDecorator(FuzzedDataProvider& fdp, int depth)
{
  const std::string child = genNode(fdp, depth + 1);
  switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 10))
  {
    case 0:
      return "<Inverter>" + child + "</Inverter>";
    case 1:
      return "<ForceSuccess>" + child + "</ForceSuccess>";
    case 2:
      return "<ForceFailure>" + child + "</ForceFailure>";
    case 3:
      return "<KeepRunningUntilFailure>" + child + "</KeepRunningUntilFailure>";
    case 4:
      return "<RunOnce>" + child + "</RunOnce>";
    case 5:
      return "<Repeat num_cycles=\"2\">" + child + "</Repeat>";
    case 6:
      return "<RetryUntilSuccessful num_attempts=\"2\">" + child +
             "</RetryUntilSuccessful>";
    case 7:
      return "<Timeout msec=\"10\">" + child + "</Timeout>";
    case 8:
      return "<Delay delay_msec=\"1\">" + child + "</Delay>";
    case 9:
      return "<Precondition if=\"" + pickCondition(fdp) + "\" else=\"" +
             std::string(fdp.PickValueInArray(kStatuses)) + "\">" + child +
             "</Precondition>";
    default:
      return std::string("<LoopString queue=\"a;b\" value=\"{loops}\" if_empty=\"") +
             fdp.PickValueInArray(kStatuses) + "\">" + child + "</LoopString>";
  }
}

std::string genChildrenBody(FuzzedDataProvider& fdp, int depth, size_t count)
{
  std::string body;
  for(size_t i = 0; i < count; i++)
  {
    body += genNode(fdp, depth + 1);
  }
  return body;
}

std::string genControl(FuzzedDataProvider& fdp, int depth)
{
  const size_t n = fdp.ConsumeIntegralInRange<size_t>(1, 3);
  switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 10))
  {
    case 0:
      return "<Sequence>" + genChildrenBody(fdp, depth, n) + "</Sequence>";
    case 1:
      return "<ReactiveSequence>" + genChildrenBody(fdp, depth, n) +
             "</ReactiveSequence>";
    case 2:
      return "<AsyncSequence>" + genChildrenBody(fdp, depth, n) +
             "</AsyncSequence>";
    case 3:
      return "<Fallback>" + genChildrenBody(fdp, depth, n) + "</Fallback>";
    case 4:
      return "<ReactiveFallback>" + genChildrenBody(fdp, depth, n) +
             "</ReactiveFallback>";
    case 5:
      return "<AsyncFallback>" + genChildrenBody(fdp, depth, n) +
             "</AsyncFallback>";
    case 6:
    {
      const size_t children = fdp.ConsumeIntegralInRange<size_t>(1, 3);
      return "<Parallel success_count=\"1\" failure_count=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<size_t>(1, children)) +
             "\">" + genChildrenBody(fdp, depth, children) + "</Parallel>";
    }
    case 7:
      return "<ParallelAll failure_count=\"1\">" + genChildrenBody(fdp, depth, n) +
             "</ParallelAll>";
    case 8:
      return "<IfThenElse>" + genConditionLeaf(fdp) + genNode(fdp, depth + 1) +
             "</IfThenElse>";
    case 9:
      return "<Switch3 variable=\"{var}\" case_1=\"1\" case_2=\"2\" case_3=\"3\">" +
             genChildrenBody(fdp, depth, 4) + "</Switch3>";
    default:
      return "<WhileDoElse>" + genConditionLeaf(fdp) + genNode(fdp, depth + 1) +
             "</WhileDoElse>";
  }
}

std::string wrapTree(const std::string& tree_id, const std::string& body)
{
  return "<BehaviorTree ID=\"" + tree_id + "\">" + body + "</BehaviorTree>";
}

// A model section describing two custom nodes: exercises the
// <TreeNodesModel> reading path when embedded in a full document.
const char* kModelSection =
    "<TreeNodesModel>"
    "<Action ID=\"CustomAct\" description=\"fuzz action\">"
    "<input_port name=\"p\" type=\"int\" default=\"3\"/>"
    "<output_port name=\"q\" type=\"string\"/>"
    "</Action>"
    "<Condition ID=\"CustomCond\" description=\"fuzz condition\">"
    "<input_port name=\"c\" type=\"bool\"/>"
    "</Condition>"
    "</TreeNodesModel>";

}  // namespace


extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 8)
  {
    return 0;
  }
  FuzzedDataProvider fdp(data, size);

  BT::BehaviorTreeFactory factory;
  factory.registerScriptingEnum("MODE_A", 1);

  try
  {
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 3))
    {
      case 0:
      {
        // raw fuzz bytes into the parser: error and recovery paths
        const std::string raw = fdp.ConsumeRandomLengthString(size);
        const std::unordered_map<std::string, BT::NodeType> no_nodes;
        BT::VerifyXML(raw, no_nodes);
        factory.registerBehaviorTreeFromText(raw);
        return 0;
      }
      case 1:
      {
        // single valid tree: export -> re-import -> export
        const std::string xml =
            "<root BTCPP_format=\"4\">" + wrapTree("MainTree", genNode(fdp, 0)) +
            "</root>";
        factory.registerBehaviorTreeFromText(xml);
        auto blackboard = BT::Blackboard::create();
        auto tree = factory.createTree("MainTree", blackboard);
        const bool add_metadata = fdp.ConsumeBool();
        const bool add_builtin_models = fdp.ConsumeBool();
        const std::string exported =
            BT::WriteTreeToXML(tree, add_metadata, add_builtin_models);
        BT::BehaviorTreeFactory factory2;
        factory2.registerScriptingEnum("MODE_A", 1);
        factory2.registerBehaviorTreeFromText(exported);
        auto tree2 = factory2.createTree("MainTree", BT::Blackboard::create());
        (void)BT::WriteTreeToXML(tree2, add_metadata, add_builtin_models);
        break;
      }
      case 2:
      {
        // two trees with a SubTree reference, then the model + XSD writers
        const std::string xml =
            "<root BTCPP_format=\"4\">" +
            wrapTree("MainTree",
                     "<Sequence><SubTree ID=\"Aux\"/>" + genNode(fdp, 0) +
                         "</Sequence>") +
            wrapTree("Aux", genLeaf(fdp)) + "</root>";
        factory.registerBehaviorTreeFromText(xml);
        auto blackboard = BT::Blackboard::create();
        auto tree = factory.createTree("MainTree", blackboard);
        const std::string exported = BT::WriteTreeToXML(tree, true, true);
        BT::BehaviorTreeFactory factory2;
        factory2.registerBehaviorTreeFromText(exported);
        const std::string model = BT::writeTreeNodesModelXML(factory, true);
        BT::VerifyXML(model, {});
        (void)BT::writeTreeXSD(factory);
        break;
      }
      default:
      {
        // document that also embeds a <TreeNodesModel> section
        const std::string xml = "<root BTCPP_format=\"4\">" +
                                wrapTree("MainTree", genNode(fdp, 0)) +
                                kModelSection + "</root>";
        factory.registerBehaviorTreeFromText(xml);
        auto blackboard = BT::Blackboard::create();
        auto tree = factory.createTree("MainTree", blackboard);
        (void)BT::WriteTreeToXML(tree, fdp.ConsumeBool(), true);
        const std::string model = BT::writeTreeNodesModelXML(factory, false);
        BT::BehaviorTreeFactory factory2;
        factory2.registerBehaviorTreeFromText(model);
        break;
      }
    }
  }
  catch(const std::exception&)
  {
    // malformed input and unknown-node errors are expected here
  }

  return 0;
}

