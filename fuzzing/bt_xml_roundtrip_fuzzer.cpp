// Fuzz target: bt_xml_roundtrip_fuzzer
//
// Threat model: BehaviorTree.CPP both *reads* XML from untrusted sources and
// *writes* XML that other tools re-read (Groot2 project files, exported
// trees). A divergence between WriteTreeToXML and re-import is a real bug
// (e.g. a dropped SubTree name). Raw parser coverage belongs to bt_fuzzer.
//
// Strategy: generate a valid tree (optionally with a SubTree), create it,
// exported1 = WriteTreeToXML(tree, false, false), re-import in a fresh
// factory (re-import failures abort), exported2 = WriteTreeToXML(tree2,
// false, false), abort if exported1 != exported2.
//
// Generation is bounded (depth, child counts). No tree is ticked here.
// Exceptions from the *generated* input are expected and ignored; exceptions
// from re-import of our own export are not.

#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <cstdint>
#include <cstdlib>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

namespace
{

constexpr int kMaxDepth = 3;

const char* kConditionScripts[] = { "var > 0", "var < 5",      "flag",
                                    "!flag",   "str == 'abc'", "var == MODE_A" };
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
      return "<AsyncSequence>" + genChildrenBody(fdp, depth, n) + "</AsyncSequence>";
    case 3:
      return "<Fallback>" + genChildrenBody(fdp, depth, n) + "</Fallback>";
    case 4:
      return "<ReactiveFallback>" + genChildrenBody(fdp, depth, n) +
             "</ReactiveFallback>";
    case 5:
      return "<AsyncFallback>" + genChildrenBody(fdp, depth, n) + "</AsyncFallback>";
    case 6: {
      const size_t children = fdp.ConsumeIntegralInRange<size_t>(1, 3);
      return "<Parallel success_count=\"1\" failure_count=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<size_t>(1, children)) + "\">" +
             genChildrenBody(fdp, depth, children) + "</Parallel>";
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

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 8)
  {
    return 0;
  }
  FuzzedDataProvider fdp(data, size);

  // Build a valid document: either a single tree or MainTree + Aux SubTree.
  std::string xml;
  if(fdp.ConsumeBool())
  {
    xml = "<root BTCPP_format=\"4\">" + wrapTree("MainTree", genNode(fdp, 0)) +
          "</root>";
  }
  else
  {
    xml = "<root BTCPP_format=\"4\">" +
          wrapTree("MainTree", "<Sequence><SubTree ID=\"Aux\"/>" + genNode(fdp, 0) +
                                   "</Sequence>") +
          wrapTree("Aux", genLeaf(fdp)) + "</root>";
  }

  BT::Tree tree;
  try
  {
    BT::BehaviorTreeFactory factory;
    factory.registerScriptingEnum("MODE_A", 1);
    factory.registerBehaviorTreeFromText(xml);
    tree = factory.createTree("MainTree", BT::Blackboard::create());
  }
  catch(const std::exception&)
  {
    // Generated input that fails to parse/create is expected.
    return 0;
  }

  const std::string exported1 = BT::WriteTreeToXML(tree, false, false);

  // Re-import of our own export must succeed; failure is a real bug.
  BT::Tree tree2;
  try
  {
    BT::BehaviorTreeFactory factory2;
    factory2.registerScriptingEnum("MODE_A", 1);
    factory2.registerBehaviorTreeFromText(exported1);
    tree2 = factory2.createTree("MainTree", BT::Blackboard::create());
  }
  catch(const std::exception&)
  {
    std::abort();
  }

  const std::string exported2 = BT::WriteTreeToXML(tree2, false, false);
  if(exported1 != exported2)
  {
    std::abort();
  }

  return 0;
}
