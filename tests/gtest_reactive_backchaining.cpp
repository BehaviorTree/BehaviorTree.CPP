#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_observer.h"

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

namespace BT::test
{

class SimpleCondition : public BT::ConditionNode
{
private:
  std::string port_name_;

public:
  SimpleCondition(const std::string& name, const BT::NodeConfig& config,
                  std::string port_name)
    : BT::ConditionNode(name, config), port_name_(port_name)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  BT::NodeStatus tick() override
  {
    auto val = config().blackboard->get<bool>(port_name_);
    return (val) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
  }
};

//--------------------------
class AsyncTestAction : public BT::StatefulActionNode
{
  int counter_ = 0;
  std::string port_name_;

public:
  AsyncTestAction(const std::string& name, const BT::NodeConfig& config,
                  std::string port_name)
    : BT::StatefulActionNode(name, config), port_name_(port_name)
  {}

  static BT::PortsList providedPorts()
  {
    return {};
  }

  NodeStatus onStart() override
  {
    counter_ = 0;
    return NodeStatus::RUNNING;
  }

  NodeStatus onRunning() override
  {
    if(++counter_ == 2)
    {
      config().blackboard->set<bool>(port_name_, true);
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::RUNNING;
  }
  void onHalted() override
  {}
};

// Records when asynchronous actions are started and halted, and keeps track of
// how many of them are alive at the same time. Used to check that a branch is
// stopped *before* the next one is started (issue #1031).
struct AsyncActionTracker
{
  std::vector<std::string> events;
  int running_actions = 0;
  int max_running_actions = 0;
  int started_while_another_was_running = 0;

  bool hasEvent(const std::string& event) const
  {
    return std::find(events.begin(), events.end(), event) != events.end();
  }

  size_t indexOf(const std::string& event) const
  {
    const auto it = std::find(events.begin(), events.end(), event);
    return it == events.end() ? events.size() : size_t(std::distance(events.begin(), it));
  }
};

class TrackedAsyncAction : public BT::StatefulActionNode
{
public:
  TrackedAsyncAction(const std::string& name, const BT::NodeConfig& config,
                     AsyncActionTracker* tracker)
    : BT::StatefulActionNode(name, config), tracker_(tracker)
  {}

  static BT::PortsList providedPorts()
  {
    return {};
  }

  NodeStatus onStart() override
  {
    if(tracker_->running_actions > 0)
    {
      tracker_->started_while_another_was_running++;
    }
    tracker_->events.push_back(name() + ":start");
    tracker_->running_actions++;
    tracker_->max_running_actions =
        (std::max)(tracker_->max_running_actions, tracker_->running_actions);
    return NodeStatus::RUNNING;
  }

  NodeStatus onRunning() override
  {
    return NodeStatus::RUNNING;
  }

  void onHalted() override
  {
    tracker_->events.push_back(name() + ":halted");
    tracker_->running_actions--;
  }

private:
  AsyncActionTracker* tracker_;
};
//--------------------------

TEST(ReactiveBackchaining, EnsureWarm)
{
  // This test shows the basic structure of a PPA: a fallback
  // of a postcondition and an action to make that
  //  postcondition true.
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="EnsureWarm">
      <ReactiveFallback>
        <IsWarm name="warm"/>
        <ReactiveSequence>
          <IsHoldingJacket name="jacket" />
          <WearJacket name="wear" />
        </ReactiveSequence>
      </ReactiveFallback>
    </BehaviorTree>
  </root>
  )";

  // The final condition of the PPA; the thing that make_warm achieves.
  // For this example, we're only warm after WearJacket returns success.
  BehaviorTreeFactory factory;
  factory.registerNodeType<SimpleCondition>("IsWarm", "is_warm");
  factory.registerNodeType<SimpleCondition>("IsHoldingJacket", "holding_jacket");
  factory.registerNodeType<AsyncTestAction>("WearJacket", "is_warm");

  Tree tree = factory.createTreeFromText(xml_text);
  BT::TreeObserver observer(tree);

  auto& blackboard = tree.subtrees.front()->blackboard;
  blackboard->set("is_warm", false);
  blackboard->set("holding_jacket", true);

  // first tick: not warm, have a jacket: start wearing it
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  EXPECT_FALSE(blackboard->get<bool>("is_warm"));

  // second tick: not warm (still wearing)
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  EXPECT_FALSE(blackboard->get<bool>("is_warm"));

  // third tick: warm (wearing succeeded)
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::SUCCESS);
  EXPECT_TRUE(blackboard->get<bool>("is_warm"));

  // fourth tick: still warm (just the condition ticked)
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::SUCCESS);

  EXPECT_EQ(observer.getStatistics("warm").failure_count, 3);
  EXPECT_EQ(observer.getStatistics("warm").success_count, 1);

  EXPECT_EQ(observer.getStatistics("jacket").transitions_count, 3);
  EXPECT_EQ(observer.getStatistics("jacket").success_count, 3);

  EXPECT_EQ(observer.getStatistics("wear").success_count, 1);
}

TEST(ReactiveBackchaining, EnsureWarmWithEnsureHoldingHacket)
{
  // This test backchains on HoldingHacket => EnsureHoldingHacket to iteratively add reactivity and functionality to the tree.
  // The general structure of the PPA remains the same.
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="EnsureWarm">
      <ReactiveFallback>
        <IsWarm />
        <ReactiveSequence>
          <SubTree ID="EnsureHoldingJacket" />
          <WearJacket />
        </ReactiveSequence>
      </ReactiveFallback>
    </BehaviorTree>

    <BehaviorTree ID="EnsureHoldingJacket">
      <ReactiveFallback>
        <IsHoldingJacket />
        <ReactiveSequence>
          <IsNearCloset />
          <GrabJacket />
        </ReactiveSequence>
      </ReactiveFallback>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  factory.registerNodeType<SimpleCondition>("IsWarm", "is_warm");
  factory.registerNodeType<SimpleCondition>("IsHoldingJacket", "holding_jacket");
  factory.registerNodeType<SimpleCondition>("IsNearCloset", "near_closet");
  factory.registerNodeType<AsyncTestAction>("WearJacket", "is_warm");
  factory.registerNodeType<AsyncTestAction>("GrabJacket", "holding_jacket");

  factory.registerBehaviorTreeFromText(xml_text);
  Tree tree = factory.createTree("EnsureWarm");
  BT::TreeObserver observer(tree);

  tree.subtrees[0]->blackboard->set("is_warm", false);
  tree.subtrees[1]->blackboard->set("holding_jacket", false);
  tree.subtrees[1]->blackboard->set("near_closet", true);

  // first tick: not warm, no jacket, start GrabJacket
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  EXPECT_FALSE(tree.subtrees[0]->blackboard->get<bool>("is_warm"));
  EXPECT_FALSE(tree.subtrees[1]->blackboard->get<bool>("holding_jacket"));
  EXPECT_TRUE(tree.subtrees[1]->blackboard->get<bool>("near_closet"));

  // second tick: still GrabJacket
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);

  // third tick: GrabJacket succeeded, start wearing
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  EXPECT_FALSE(tree.subtrees[0]->blackboard->get<bool>("is_warm"));
  EXPECT_TRUE(tree.subtrees[1]->blackboard->get<bool>("holding_jacket"));

  // fourth tick: still WearingJacket
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);

  // fifth tick: warm (WearingJacket succeeded)
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::SUCCESS);
  EXPECT_TRUE(tree.subtrees[0]->blackboard->get<bool>("is_warm"));

  // sixr tick: still warm (just the condition ticked)
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::SUCCESS);
}

// When the second branch of a reactive fallback takes over, the asynchronous
// action of the first branch must be halted before the new one is started.
// Otherwise both would be alive at the same time and, for instance, two ROS
// actions would run concurrently (issue #1031).
TEST(ReactiveBackchaining, HaltBranchBeforeStartingAnotherAsyncAction)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Main">
      <ReactiveSequence>
        <ReactiveFallback>
          <IsBatteryOk/>
          <GotoCharging/>
        </ReactiveFallback>
        <GotoKitchen/>
      </ReactiveSequence>
    </BehaviorTree>
  </root>
  )";

  AsyncActionTracker tracker;

  BehaviorTreeFactory factory;
  factory.registerNodeType<SimpleCondition>("IsBatteryOk", "battery_ok");
  factory.registerBuilder<TrackedAsyncAction>(
      "GotoCharging", [&tracker](const std::string& name, const BT::NodeConfig& config) {
        return std::make_unique<TrackedAsyncAction>(name, config, &tracker);
      });
  factory.registerBuilder<TrackedAsyncAction>(
      "GotoKitchen", [&tracker](const std::string& name, const BT::NodeConfig& config) {
        return std::make_unique<TrackedAsyncAction>(name, config, &tracker);
      });

  auto tree = factory.createTreeFromText(xml_text);
  auto blackboard = tree.subtrees.front()->blackboard;
  blackboard->set("battery_ok", true);

  // The battery is still fine, so the robot goes to the kitchen.
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  EXPECT_TRUE(tracker.hasEvent("GotoKitchen:start"));
  EXPECT_EQ(tracker.running_actions, 1);

  // The battery runs low while the robot is still moving: the goal must be
  // cancelled before the charging action is started.
  blackboard->set("battery_ok", false);
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);

  EXPECT_EQ(tracker.started_while_another_was_running, 0);
  EXPECT_EQ(tracker.max_running_actions, 1);
  EXPECT_EQ(tracker.running_actions, 1);
  EXPECT_LT(tracker.indexOf("GotoKitchen:halted"), tracker.indexOf("GotoCharging:start"));
}

}  // namespace BT::test
