#include "test_helper.hpp"

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_observer.h"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using BT::NodeStatus;
using std::chrono::milliseconds;

TEST(Reactive, RunningChildren)
{
  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <ReactiveSequence>
      <Sequence name="first">
        <TestA/>
        <TestB/>
        <TestC/>
      </Sequence>
      <AsyncSequence name="second">
        <TestD/>
        <TestE/>
        <TestF/>
      </AsyncSequence>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BT::BehaviorTreeFactory factory;
  std::array<int, 6> counters{};
  RegisterTestTick(factory, "Test", counters);

  auto tree = factory.createTreeFromText(reactive_xml_text);

  NodeStatus status = NodeStatus::IDLE;

  int count = 0;
  while(!BT::isStatusCompleted(status) && count < 100)
  {
    count++;
    status = tree.tickExactlyOnce();
  }

  ASSERT_NE(100, count);

  ASSERT_EQ(status, NodeStatus::SUCCESS);

  ASSERT_EQ(counters[0], 3);
  ASSERT_EQ(counters[1], 3);
  ASSERT_EQ(counters[2], 3);

  ASSERT_EQ(counters[3], 1);
  ASSERT_EQ(counters[4], 1);
  ASSERT_EQ(counters[5], 1);
}

TEST(Reactive, Issue587)
{
  // TestA should be executed only once, because of the variable "test"

  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="Example_A">
    <Sequence>
      <Script code="test := false"/>
      <ReactiveSequence>
        <RetryUntilSuccessful name="Retry 1" num_attempts="-1" _skipIf="test ">
          <TestA name="Success 1" _onSuccess="test = true"/>
        </RetryUntilSuccessful>
        <RetryUntilSuccessful name="Retry 2" num_attempts="5">
          <AlwaysFailure name="Failure 2"/>
        </RetryUntilSuccessful>
      </ReactiveSequence>
    </Sequence>
  </BehaviorTree>
</root>
)";

  BT::BehaviorTreeFactory factory;
  std::array<int, 2> counters{};
  RegisterTestTick(factory, "Test", counters);

  auto tree = factory.createTreeFromText(reactive_xml_text);
  tree.tickWhileRunning();

  ASSERT_EQ(counters[0], 1);
}

TEST(Reactive, PreTickHooks)
{
  using namespace BT;

  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="Main">
    <ReactiveSequence>
      <AlwaysFailure name="failureA"/>
      <AlwaysFailure name="failureB"/>
      <Sleep msec="100"/>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BehaviorTreeFactory factory;

  auto tree = factory.createTreeFromText(reactive_xml_text);

  TreeNode::PreTickCallback callback = [](TreeNode& node) -> NodeStatus {
    std::cout << node.name() << " callback" << std::endl;
    return NodeStatus::SUCCESS;
  };

  tree.applyVisitor([&](TreeNode* node) -> void {
    if(auto dd = dynamic_cast<BT::AlwaysFailureNode*>(node))
    {
      dd->setPreTickFunction(callback);
    }
  });

  auto ret = tree.tickWhileRunning();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);
}

TEST(Reactive, TestLogging)
{
  using namespace BT;

  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="Main">
    <ReactiveSequence>
      <TestA name="testA"/>
      <AlwaysSuccess name="success"/>
      <Sleep msec="100"/>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BehaviorTreeFactory factory;

  std::array<int, 1> counters{};
  RegisterTestTick(factory, "Test", counters);

  auto tree = factory.createTreeFromText(reactive_xml_text);
  TreeObserver observer(tree);

  auto ret = tree.tickWhileRunning();
  ASSERT_EQ(ret, NodeStatus::SUCCESS);

  int num_ticks = counters[0];
  ASSERT_GE(num_ticks, 5);

  ASSERT_EQ(observer.getStatistics("testA").success_count, num_ticks);
  ASSERT_EQ(observer.getStatistics("success").success_count, num_ticks);
}

TEST(Reactive, TwoAsyncNodesInReactiveSequence)
{
  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <ReactiveSequence>
      <AsyncSequence name="first">
        <TestA/>
        <TestB/>
        <TestC/>
      </AsyncSequence>
      <AsyncSequence name="second">
        <TestD/>
        <TestE/>
        <TestF/>
      </AsyncSequence>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BT::BehaviorTreeFactory factory;
  std::array<int, 6> counters{};
  RegisterTestTick(factory, "Test", counters);

  EXPECT_ANY_THROW(auto tree = factory.createTreeFromText(reactive_xml_text));
}

TEST(Reactive, TwoAsyncFallbacksInReactiveSequence)
{
  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <ReactiveSequence>
      <AsyncFallback name="first">
        <TestA/>
        <TestB/>
        <TestC/>
      </AsyncFallback>
      <AsyncFallback name="second">
        <TestD/>
        <TestE/>
        <TestF/>
      </AsyncFallback>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BT::BehaviorTreeFactory factory;
  std::array<int, 6> counters{};
  RegisterTestTick(factory, "Test", counters);

  EXPECT_ANY_THROW(auto tree = factory.createTreeFromText(reactive_xml_text));
}

TEST(Reactive, AsyncSequenceAndAsyncFallbackInReactiveSequence)
{
  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <ReactiveSequence>
      <AsyncSequence name="first">
        <TestA/>
        <TestB/>
      </AsyncSequence>
      <AsyncFallback name="second">
        <TestC/>
        <TestD/>
      </AsyncFallback>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BT::BehaviorTreeFactory factory;
  std::array<int, 6> counters{};
  RegisterTestTick(factory, "Test", counters);

  EXPECT_ANY_THROW(auto tree = factory.createTreeFromText(reactive_xml_text));
}

TEST(Reactive, SingleAsyncChildInReactiveSequenceIsAllowed)
{
  static const char* reactive_xml_text = R"(
<root BTCPP_format="4" >
  <BehaviorTree ID="MainTree">
    <ReactiveSequence>
      <TestA/>
      <AsyncFallback name="only_async">
        <TestB/>
        <TestC/>
      </AsyncFallback>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  BT::BehaviorTreeFactory factory;
  std::array<int, 6> counters{};
  RegisterTestTick(factory, "Test", counters);

  EXPECT_NO_THROW(auto tree = factory.createTreeFromText(reactive_xml_text));
}

// ============ Phase 4: Additional Reactive Tests ============

TEST(Reactive, ReactiveSequence_FirstChildFails)
{
  // When first child fails, ReactiveSequence should return FAILURE immediately
  BT::BehaviorTreeFactory factory;
  std::array<int, 2> counters{};
  RegisterTestTick(factory, "Test", counters);

  static const char* xml_text = R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveSequence>
      <AlwaysFailure/>
      <TestA/>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(counters[0], 0);  // TestA should never be ticked
}

TEST(Reactive, ReactiveSequence_HaltOnConditionChange)
{
  // Test that running children are halted when an earlier condition changes
  BT::BehaviorTreeFactory factory;

  bool condition_result = true;
  int child_tick_count = 0;
  bool child_was_halted = false;

  factory.registerSimpleCondition("DynamicCondition", [&condition_result](BT::TreeNode&) {
    return condition_result ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
  });

  factory.registerBuilder<BT::StatefulActionNode>(
      "TrackingAction", [&child_tick_count, &child_was_halted](
                            const std::string& name, const BT::NodeConfig& config) {
        class TrackingAction : public BT::StatefulActionNode
        {
        public:
          int& tick_count_;
          bool& was_halted_;
          TrackingAction(const std::string& name, const BT::NodeConfig& config,
                         int& tick_count, bool& was_halted)
            : BT::StatefulActionNode(name, config)
            , tick_count_(tick_count)
            , was_halted_(was_halted)
          {}
          NodeStatus onStart() override
          {
            tick_count_++;
            return NodeStatus::RUNNING;
          }
          NodeStatus onRunning() override
          {
            tick_count_++;
            return NodeStatus::RUNNING;
          }
          void onHalted() override
          {
            was_halted_ = true;
          }
        };
        return std::make_unique<TrackingAction>(name, config, child_tick_count,
                                                child_was_halted);
      });

  static const char* xml_text = R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveSequence>
      <DynamicCondition/>
      <TrackingAction/>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick - condition passes, action starts
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);
  ASSERT_GE(child_tick_count, 1);
  ASSERT_FALSE(child_was_halted);

  // Tick again while condition is still true
  status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);

  // Now change condition to false - child should be halted
  condition_result = false;
  status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_TRUE(child_was_halted);
}

TEST(Reactive, ReactiveFallback_FirstChildSucceeds)
{
  // When first child succeeds, ReactiveFallback should return SUCCESS immediately
  BT::BehaviorTreeFactory factory;
  std::array<int, 2> counters{};
  RegisterTestTick(factory, "Test", counters);

  static const char* xml_text = R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveFallback>
      <AlwaysSuccess/>
      <TestA/>
    </ReactiveFallback>
  </BehaviorTree>
</root>
)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 0);  // TestA should never be ticked
}

TEST(Reactive, ReactiveFallback_AllChildrenFail)
{
  BT::BehaviorTreeFactory factory;

  static const char* xml_text = R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveFallback>
      <AlwaysFailure/>
      <AlwaysFailure/>
      <AlwaysFailure/>
    </ReactiveFallback>
  </BehaviorTree>
</root>
)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST(Reactive, ReactiveFallback_SecondChildSucceeds)
{
  BT::BehaviorTreeFactory factory;
  std::array<int, 2> counters{};
  RegisterTestTick(factory, "Test", counters);

  static const char* xml_text = R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveFallback>
      <AlwaysFailure/>
      <TestA/>
    </ReactiveFallback>
  </BehaviorTree>
</root>
)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);  // TestA ticked once and succeeded
}

TEST(Reactive, ReactiveSequence_AllChildrenSucceed)
{
  BT::BehaviorTreeFactory factory;
  std::array<int, 3> counters{};
  RegisterTestTick(factory, "Test", counters);

  static const char* xml_text = R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveSequence>
      <TestA/>
      <TestB/>
      <TestC/>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  ASSERT_EQ(counters[0], 1);
  ASSERT_EQ(counters[1], 1);
  ASSERT_EQ(counters[2], 1);
}

TEST(Reactive, ReactiveSequence_ReEvaluatesOnEveryTick)
{
  // Verify that conditions are re-evaluated on every tick in ReactiveSequence
  BT::BehaviorTreeFactory factory;

  int condition_tick_count = 0;
  factory.registerSimpleCondition("CountingCondition",
                                  [&condition_tick_count](BT::TreeNode&) {
                                    condition_tick_count++;
                                    return NodeStatus::SUCCESS;
                                  });

  static const char* xml_text = R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveSequence>
      <CountingCondition/>
      <Sleep msec="50"/>
    </ReactiveSequence>
  </BehaviorTree>
</root>
)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  ASSERT_EQ(status, NodeStatus::SUCCESS);
  // Condition should be ticked multiple times (re-evaluated while Sleep is running)
  ASSERT_GE(condition_tick_count, 2);
}

namespace
{
// Logs onStart() and onHalted(); onStart() returns the given status, onRunning() keeps
// running.
class LoggedAction : public BT::StatefulActionNode
{
public:
  LoggedAction(const std::string& name, const BT::NodeConfig& config,
               std::vector<std::string>* log, NodeStatus start_status)
    : BT::StatefulActionNode(name, config), log_(log), start_status_(start_status)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  NodeStatus onStart() override
  {
    log_->push_back(name() + ":start");
    return start_status_;
  }
  NodeStatus onRunning() override
  {
    return NodeStatus::RUNNING;
  }
  void onHalted() override
  {
    log_->push_back(name() + ":halted");
  }

private:
  std::vector<std::string>* log_;
  NodeStatus start_status_;
};

// A coroutine action that completes with SUCCESS on its first tick.
class LoggedCoroCheck : public BT::CoroActionNode
{
public:
  LoggedCoroCheck(const std::string& name, const BT::NodeConfig& config,
                  std::vector<std::string>* log)
    : BT::CoroActionNode(name, config), log_(log)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  NodeStatus tick() override
  {
    log_->push_back(name() + ":start");
    return NodeStatus::SUCCESS;
  }

private:
  std::vector<std::string>* log_;
};

// Keeps all its children running at once, like nav2's PipelineSequence: every tick it
// ticks each child and restarts the ones that completed.
class TickAllChildren : public BT::ControlNode
{
public:
  TickAllChildren(const std::string& name, const BT::NodeConfig& config)
    : BT::ControlNode(name, config)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  NodeStatus tick() override
  {
    setStatus(NodeStatus::RUNNING);
    for(size_t i = 0; i < childrenCount(); i++)
    {
      if(BT::isStatusCompleted(children_nodes_[i]->executeTick()))
      {
        haltChild(i);
      }
    }
    return NodeStatus::RUNNING;
  }
};

long countOf(const std::vector<std::string>& log, const std::string& event)
{
  return std::count(log.begin(), log.end(), event);
}

void registerLoggedNodes(BT::BehaviorTreeFactory& factory, std::vector<std::string>* log)
{
  factory.registerNodeType<LoggedAction>("QuickSuccess", log, NodeStatus::SUCCESS);
  factory.registerNodeType<LoggedAction>("QuickFailure", log, NodeStatus::FAILURE);
  factory.registerNodeType<LoggedAction>("LongAction", log, NodeStatus::RUNNING);
  factory.registerNodeType<LoggedCoroCheck>("CoroCheck", log);
  factory.registerNodeType<TickAllChildren>("TickAllChildren");
}
}  // namespace

// The children before a RUNNING child are re-ticked on every tick. When they complete
// immediately (a check implemented as an action), the RUNNING child must keep running:
// it is not halted and not started again (see #1031 and #1217).
TEST(Reactive, ReactiveSequence_CompletedSiblingsDoNotRestartRunningChild)
{
  BT::BehaviorTreeFactory factory;
  std::vector<std::string> log;
  registerLoggedNodes(factory, &log);

  auto tree = factory.createTreeFromText(R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveSequence>
      <QuickSuccess name="Check"/>
      <CoroCheck name="CoroCheck"/>
      <LongAction name="Long"/>
    </ReactiveSequence>
  </BehaviorTree>
</root>)");

  for(int i = 0; i < 5; i++)
  {
    ASSERT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  }
  EXPECT_EQ(countOf(log, "Check:start"), 5);
  EXPECT_EQ(countOf(log, "CoroCheck:start"), 5);
  EXPECT_EQ(countOf(log, "Long:start"), 1);
  EXPECT_EQ(countOf(log, "Long:halted"), 0);
}

TEST(Reactive, ReactiveFallback_CompletedSiblingsDoNotRestartRunningChild)
{
  BT::BehaviorTreeFactory factory;
  std::vector<std::string> log;
  registerLoggedNodes(factory, &log);

  auto tree = factory.createTreeFromText(R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveFallback>
      <QuickFailure name="Check"/>
      <LongAction name="Long"/>
    </ReactiveFallback>
  </BehaviorTree>
</root>)");

  for(int i = 0; i < 5; i++)
  {
    ASSERT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  }
  EXPECT_EQ(countOf(log, "Check:start"), 5);
  EXPECT_EQ(countOf(log, "Long:start"), 1);
  EXPECT_EQ(countOf(log, "Long:halted"), 0);
}

// A control node may keep several children running at once (nav2's PipelineSequence).
// Starting one of them must not halt the others.
TEST(Reactive, ConcurrentChildrenOfCustomControlAreNotHalted)
{
  BT::BehaviorTreeFactory factory;
  std::vector<std::string> log;
  registerLoggedNodes(factory, &log);

  auto tree = factory.createTreeFromText(R"(
<root BTCPP_format="4">
  <BehaviorTree>
    <ReactiveSequence>
      <QuickSuccess name="Guard"/>
      <TickAllChildren>
        <QuickSuccess name="Replan"/>
        <LongAction name="Follow"/>
      </TickAllChildren>
    </ReactiveSequence>
  </BehaviorTree>
</root>)");

  for(int i = 0; i < 5; i++)
  {
    ASSERT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  }
  EXPECT_EQ(countOf(log, "Replan:start"), 5);
  EXPECT_EQ(countOf(log, "Follow:start"), 1);
  EXPECT_EQ(countOf(log, "Follow:halted"), 0);
}
