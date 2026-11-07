// Fuzz target: bt_tree_exec_fuzzer
//
// Threat model:
// Behavior trees are frequently loaded from untrusted sources (files, network
// APIs, editor payloads). The existing bt_fuzzer covers XML *parsing* only:
// it never ticks a tree, so the whole execution engine (control nodes,
// decorators, tree_node/behavior_tree state machines, ports at runtime,
// substitution rules, wake-up/timer machinery) is dark in production coverage.
//
// This harness generates *structurally valid* XML over the builtin node set,
// creates the tree, and actually drives it: repeated tickRoot() calls until a
// terminal status, halt() while RUNNING, re-tick after halt, SubTree
// references, scripting enums, substitution rules (incl. wildcard filters) and
// a typed blackboard. All generated values are bounded (depth, children,
// Repeat/Retry counts never -1, Sleep/Timeout/Delay/Delayed-Test in the low
// milliseconds) so that no input can make the harness hang or blow memory.
//
// Expected bug classes: state-machine logic violations, use-after-free/dangling
// node handling on halt-while-RUNNING, out-of-bounds child indexing in control
// nodes, numeric parsing of attributes, scripting runtime errors.

#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/xml_parsing.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

namespace
{

constexpr int kMaxDepth = 3;
constexpr unsigned kMaxTicks = 8;

const char* kNames[] = { "n1", "act", "cond", "root_task", "aux" };

// Scripts used by Script/ScriptCondition/Precondition/TestNode. All short,
// terminating and side-effect limited to the blackboard.
const char* kConditionScripts[] = { "var > 0",    "var < 5",      "flag",         "!flag",
                                    "dbl < 10.0", "str == 'abc'", "var == MODE_A" };
const char* kActionScripts[] = { "var = var + 1", "var = 3",         "var = var - 1",
                                 "str = 'fuzz'",  "dbl = var * 0.5", "flag = !flag",
                                 "var = MODE_A",  "flag = var > 2" };
const char* kLiterals[] = { "1", "2", "true", "false", "1.5", "hello", "{var}" };
const char* kStatuses[] = { "SUCCESS", "FAILURE", "SKIPPED" };

std::string pickName(FuzzedDataProvider& fdp)
{
  return fdp.PickValueInArray(kNames);
}

std::string pickScript(FuzzedDataProvider& fdp)
{
  return fdp.PickValueInArray(kActionScripts);
}

std::string pickCondition(FuzzedDataProvider& fdp)
{
  return fdp.PickValueInArray(kConditionScripts);
}

// A condition-like leaf: ScriptCondition / AlwaysSuccess / AlwaysFailure.
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

// Leaf actions. Occasionally blackboard-remap a port ("{key}" syntax).
std::string genLeaf(FuzzedDataProvider& fdp)
{
  switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 8))
  {
    case 0:
      return "<AlwaysSuccess/>";
    case 1:
      return "<AlwaysFailure/>";
    case 2:
      return "<Script code=\"" + pickScript(fdp) + "\"/>";
    case 3:
      return "<ScriptCondition code=\"" + pickCondition(fdp) + "\"/>";
    case 4: {
      if(fdp.ConsumeBool())
      {
        return std::string("<SetBlackboard output_key=\"var2\" value=\"") +
               fdp.PickValueInArray(kLiterals) + "\"/>";
      }
      return "<SetBlackboard output_key=\"var2\" value=\"{var}\"/>";
    }
    case 5:
      return "<Sleep msec=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<unsigned>(1, 5)) + "\"/>";
    case 6:
      return "<UnsetBlackboard key=\"var2\"/>";
    case 7:
      return "<Script code=\"{txt}\"/>";
    default: {
      if(fdp.ConsumeBool())
      {
        return "<Script code=\"" + pickScript(fdp) + "\" name=\"" + pickName(fdp) +
               "\"/>";
      }
      return "<AlwaysSuccess name=\"" + pickName(fdp) + "\"/>";
    }
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
      return "<RunOnce then_skip=\"" + std::string(fdp.ConsumeBool() ? "true" : "false") +
             "\">" + child + "</RunOnce>";
    case 5:
      // never -1: that would be an infinite loop
      return "<Repeat num_cycles=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<int>(0, 4)) + "\">" + child +
             "</Repeat>";
    case 6:
      // never -1
      return "<RetryUntilSuccessful num_attempts=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<int>(1, 4)) + "\">" + child +
             "</RetryUntilSuccessful>";
    case 7:
      return "<Timeout msec=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<unsigned>(1, 50)) + "\">" + child +
             "</Timeout>";
    case 8:
      return "<Delay delay_msec=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<unsigned>(1, 10)) + "\">" + child +
             "</Delay>";
    case 9:
      return "<Precondition if=\"" + pickCondition(fdp) + "\" else=\"" +
             std::string(fdp.PickValueInArray(kStatuses)) + "\">" + child +
             "</Precondition>";
    default:
      return std::string("<LoopInt queue=\"1;2;3\" value=\"{loopv}\" if_empty=\"") +
             fdp.PickValueInArray(kStatuses) + "\">" + child + "</LoopInt>";
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
      return "<SequenceWithMemory>" + genChildrenBody(fdp, depth, n) +
             "</SequenceWithMemory>";
    case 3:
      return "<Fallback>" + genChildrenBody(fdp, depth, n) + "</Fallback>";
    case 4:
      return "<ReactiveFallback>" + genChildrenBody(fdp, depth, n) +
             "</ReactiveFallback>";
    case 5: {
      const size_t children = fdp.ConsumeIntegralInRange<size_t>(1, 3);
      return "<Parallel success_count=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<size_t>(1, children)) +
             "\" failure_count=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<size_t>(1, children)) + "\">" +
             genChildrenBody(fdp, depth, children) + "</Parallel>";
    }
    case 6: {
      const size_t children = fdp.ConsumeIntegralInRange<size_t>(1, 3);
      return "<ParallelAll failure_count=\"" +
             std::to_string(fdp.ConsumeIntegralInRange<size_t>(1, children)) + "\">" +
             genChildrenBody(fdp, depth, children) + "</ParallelAll>";
    }
    case 7:
      // first child must be the condition branch
      return "<IfThenElse>" + genConditionLeaf(fdp) + genNode(fdp, depth + 1) +
             "</IfThenElse>";
    case 8:
      // first child must be the condition branch
      return "<WhileDoElse>" + genConditionLeaf(fdp) + genNode(fdp, depth + 1) +
             "</WhileDoElse>";
    case 9:
      // exactly NUM_CASES children plus the default one
      return "<Switch2 variable=\"{var}\" case_1=\"1\" case_2=\"2\">" +
             genChildrenBody(fdp, depth, 3) + "</Switch2>";
    default:
      // catch_on_halt="true" is not generated yet: TryCatchNode::halt() ticks
      // the catch child, and if that throws, Tree::~Tree() calls
      // std::terminate. Enable it once that upstream bug is fixed.
      return "<TryCatch catch_on_halt=\"false\">" +
             genChildrenBody(fdp, depth, fdp.ConsumeIntegralInRange<size_t>(2, 3)) +
             "</TryCatch>";
  }
}

std::string wrapTree(const std::string& tree_id, const std::string& body)
{
  return "<BehaviorTree ID=\"" + tree_id + "\">" + body + "</BehaviorTree>";
}

// Drive a created tree: at most kMaxTicks single ticks, optionally halt()
// mid-flight while RUNNING, always halt() at the end, and optionally re-tick
// after halt. Only the bounded tickExactlyOnce()/tickOnce() entry points are
// used: tickWhileRunning() has no internal bound and would hang forever on
// trees like KeepRunningUntilFailure[AlwaysSuccess], which the grammar is
// allowed to generate.
//
// Every tick and halt is wrapped, so that no exception escapes from the Tree
// destructor, which calls haltTree() and would std::terminate (upstream bug).
void safeHaltTree(BT::Tree& tree)
{
  try
  {
    tree.haltTree();
  }
  catch(const std::exception&)
  {
    // a node threw while halting: see the comment above
  }
}

void driveTree(BT::Tree& tree, FuzzedDataProvider& fdp)
{
  const unsigned ticks = fdp.ConsumeIntegralInRange<unsigned>(1, kMaxTicks);
  const unsigned halt_at = fdp.ConsumeIntegralInRange<unsigned>(0, ticks);
  // unused, but consumed so that the seed corpus keeps its meaning
  (void)fdp.ConsumeBool();
  for(unsigned i = 1; i <= ticks; i++)
  {
    BT::NodeStatus status;
    try
    {
      // Only tickExactlyOnce() is safe here: both tickOnce()
      // (ONCE_UNLESS_WOKEN_UP) and tickWhileRunning() run an *internally
      // unbounded* wake-up re-tick loop (Tree::tickRoot's inner
      // `while (wake_up_->waitFor(0))`). With self-re-arming millisecond
      // timers (e.g. Delay[KeepRunningUntilFailure[...]], which the grammar
      // legitimately generates) a single call can spin for tens of seconds
      // and blow the libFuzzer timeout. The wake-up/timer machinery itself is
      // still exercised by the nodes' timers firing during the run.
      status = tree.tickExactlyOnce();
    }
    catch(const std::exception&)
    {
      // node errors on hostile input are expected
      safeHaltTree(tree);
      return;
    }
    if(i == halt_at && status == BT::NodeStatus::RUNNING)
    {
      safeHaltTree(tree);
    }
    if(status != BT::NodeStatus::RUNNING)
    {
      break;
    }
  }
  safeHaltTree(tree);
  if(fdp.ConsumeBool())
  {
    try
    {
      (void)tree.tickExactlyOnce();
    }
    catch(const std::exception&)
    {
      // node errors on hostile input are expected
    }
    safeHaltTree(tree);
  }
}

// A small substitution-rules JSON: replaces matched nodes with TestNodes
// (bounded async_delay) and uses a wildcard filter so that the wildcard
// matching code is exercised too.
const char* kSubstitutionJson =
    "{ \"TestNodeConfigs\": {"
    "\"T1\": { \"return_status\": \"SUCCESS\", \"async_delay\": 1, "
    "\"post_script\": \"flag = true\" },"
    "\"T2\": { \"return_status\": \"FAILURE\" },"
    "\"T3\": { \"return_status_script\": \"(var > 2) ? FAILURE : SUCCESS\" } },"
    "\"SubstitutionRules\": {"
    "\"cond\": \"T1\", \"act\": \"T2\", \"n1\": \"T3\", \"*_task\": \"T1\" } }";

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if(size < 16)
  {
    return 0;
  }
  FuzzedDataProvider fdp(data, size);

  BT::BehaviorTreeFactory factory;
  factory.registerScriptingEnum("MODE_A", 1);
  factory.registerScriptingEnum("MODE_B", 2);

  auto blackboard = BT::Blackboard::create();
  blackboard->set("var", 1);
  blackboard->set("dbl", 1.5);
  blackboard->set("str", std::string("abc"));
  blackboard->set("flag", true);
  blackboard->set("txt", std::string("var = 2"));

  try
  {
    switch(fdp.ConsumeIntegralInRange<uint8_t>(0, 2))
    {
      case 0: {
        // plain create + tick
        const std::string xml = "<root BTCPP_format=\"4\">" +
                                wrapTree("MainTree", genNode(fdp, 0)) + "</root>";
        auto tree = factory.createTreeFromText(xml, blackboard);
        driveTree(tree, fdp);
        break;
      }
      case 1: {
        // multiple trees in one document + SubTree reference
        const std::string xml =
            "<root BTCPP_format=\"4\">" +
            wrapTree("MainTree", "<Sequence><SubTree ID=\"AuxTree\"/>" + genNode(fdp, 0) +
                                     "</Sequence>") +
            wrapTree("AuxTree", genLeaf(fdp)) + "</root>";
        factory.registerBehaviorTreeFromText(xml);
        auto tree = factory.createTree("MainTree", blackboard);
        driveTree(tree, fdp);
        break;
      }
      default: {
        // registry + TestNode substitutions (incl. wildcard filter)
        const std::string xml = "<root BTCPP_format=\"4\">" +
                                wrapTree("MainTree", genNode(fdp, 0)) + "</root>";
        factory.registerBehaviorTreeFromText(xml);
        factory.loadSubstitutionRuleFromJSON(kSubstitutionJson);
        const auto& names = factory.registeredBehaviorTrees();
        if(!names.empty())
        {
          auto tree = factory.createTree(names.front(), blackboard);
          driveTree(tree, fdp);
        }
        break;
      }
    }
  }
  catch(const std::exception&)
  {
    // BT raises RuntimeError/LogicError on malformed input: expected here.
  }

  return 0;
}
