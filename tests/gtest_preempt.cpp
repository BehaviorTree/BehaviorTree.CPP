#include "behaviortree_cpp/bt_factory.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

namespace BT::test
{

struct Tracker
{
  std::vector<std::string> events;
  std::atomic<int> running{ 0 };
  std::atomic<int> max_running{ 0 };
  std::atomic<int> started_while_running{ 0 };

  void onStart(const std::string& name)
  {
    if(running.load() > 0)
    {
      started_while_running++;
    }
    events.push_back(name + ":start");
    const int now = ++running;
    int previous = max_running.load();
    while(now > previous && !max_running.compare_exchange_weak(previous, now))
    {
    }
  }

  void onHalted(const std::string& name)
  {
    events.push_back(name + ":halted");
    running--;
  }

  bool hasEvent(const std::string& event) const
  {
    return std::find(events.begin(), events.end(), event) != events.end();
  }

  [[nodiscard]] size_t indexOf(const std::string& event) const
  {
    const auto it = std::find(events.begin(), events.end(), event);
    return it == events.end() ? events.size() : size_t(std::distance(events.begin(), it));
  }

  [[nodiscard]] std::string dump() const
  {
    std::string out;
    for(const auto& e : events)
    {
      out += e + " ";
    }
    return out;
  }
};

class ForeverAction : public BT::StatefulActionNode
{
public:
  ForeverAction(const std::string& name, const BT::NodeConfig& config, Tracker* tracker)
    : BT::StatefulActionNode(name, config), tracker_(tracker)
  {}

  static BT::PortsList providedPorts()
  {
    return {};
  }

  NodeStatus onStart() override
  {
    tracker_->onStart(name());
    return NodeStatus::RUNNING;
  }

  NodeStatus onRunning() override
  {
    return NodeStatus::RUNNING;
  }

  void onHalted() override
  {
    tracker_->onHalted(name());
  }

private:
  Tracker* tracker_;
};

class ForeverThreadedAction : public BT::ThreadedAction
{
public:
  ForeverThreadedAction(const std::string& name, const BT::NodeConfig& config,
                        Tracker* tracker)
    : BT::ThreadedAction(name, config), tracker_(tracker)
  {}

  static BT::PortsList providedPorts()
  {
    return {};
  }

protected:
  NodeStatus tick() override
  {
    tracker_->onStart(name());
    while(!isHaltRequested())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    tracker_->onHalted(name());
    return NodeStatus::FAILURE;
  }

private:
  Tracker* tracker_;
};

class FlagCondition : public BT::ConditionNode
{
public:
  FlagCondition(const std::string& name, const BT::NodeConfig& config, std::string key)
    : BT::ConditionNode(name, config), key_(std::move(key))
  {}

  static BT::PortsList providedPorts()
  {
    return {};
  }

  NodeStatus tick() override
  {
    return config().blackboard->get<bool>(key_) ? NodeStatus::SUCCESS :
                                                  NodeStatus::FAILURE;
  }

private:
  std::string key_;
};

static void registerNodes(BehaviorTreeFactory& factory, Tracker& tracker)
{
  factory.registerNodeType<FlagCondition>("BatteryOk", "battery_ok");
  factory.registerBuilder<ForeverAction>(
      "Forever", [&tracker](const std::string& name, const NodeConfig& config) {
        return std::make_unique<ForeverAction>(name, config, &tracker);
      });
  factory.registerBuilder<ForeverThreadedAction>(
      "ForeverThreaded", [&tracker](const std::string& name, const NodeConfig& config) {
        return std::make_unique<ForeverThreadedAction>(name, config, &tracker);
      });
}

template <typename Predicate>
static bool waitFor(Predicate predicate)
{
  for(int i = 0; i < 200; i++)
  {
    if(predicate())
    {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return predicate();
}

TEST(Preempt, plainFallbackStopsTheOldBranchBeforeStartingTheNewOne)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Main">
      <ReactiveSequence>
        <Fallback>
          <BatteryOk/>
          <Preempt>
            <Forever name="GotoCharging"/>
          </Preempt>
        </Fallback>
        <Forever name="GotoKitchen"/>
      </ReactiveSequence>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  Tracker tracker;
  registerNodes(factory, tracker);
  auto tree = factory.createTreeFromText(xml_text);

  auto bb = tree.subtrees.front()->blackboard;
  bb->set("battery_ok", true);

  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  ASSERT_TRUE(tracker.hasEvent("GotoKitchen:start")) << tracker.dump();
  EXPECT_EQ(tracker.running.load(), 1);

  bb->set("battery_ok", false);
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);

  EXPECT_EQ(tracker.started_while_running.load(), 0) << tracker.dump();
  EXPECT_LT(tracker.indexOf("GotoKitchen:halted"), tracker.indexOf("GotoCharging:start"))
      << tracker.dump();
  EXPECT_EQ(tracker.max_running.load(), 1);
  EXPECT_EQ(tracker.running.load(), 1);
}

TEST(Preempt, reactiveFallbackStopsTheOldBranchBeforeStartingTheNewOne)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Main">
      <ReactiveSequence>
        <ReactiveFallback>
          <BatteryOk/>
          <Preempt>
            <Forever name="GotoCharging"/>
          </Preempt>
        </ReactiveFallback>
        <Forever name="GotoKitchen"/>
      </ReactiveSequence>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  Tracker tracker;
  registerNodes(factory, tracker);
  auto tree = factory.createTreeFromText(xml_text);

  auto bb = tree.subtrees.front()->blackboard;
  bb->set("battery_ok", true);

  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  ASSERT_TRUE(tracker.hasEvent("GotoKitchen:start")) << tracker.dump();

  bb->set("battery_ok", false);
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);

  EXPECT_EQ(tracker.started_while_running.load(), 0) << tracker.dump();
  EXPECT_LT(tracker.indexOf("GotoKitchen:halted"), tracker.indexOf("GotoCharging:start"))
      << tracker.dump();
  EXPECT_EQ(
      std::count(tracker.events.begin(), tracker.events.end(), "GotoKitchen:halted"), 1)
      << tracker.dump();
}

TEST(Preempt, actionInsideSubTree)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4" main_tree_to_execute="Main">
    <BehaviorTree ID="Main">
      <ReactiveSequence>
        <ReactiveFallback>
          <BatteryOk/>
          <SubTree ID="Charge"/>
        </ReactiveFallback>
        <Forever name="GotoKitchen"/>
      </ReactiveSequence>
    </BehaviorTree>

    <BehaviorTree ID="Charge">
      <Preempt>
        <Forever name="GotoCharging"/>
      </Preempt>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  Tracker tracker;
  registerNodes(factory, tracker);
  auto tree = factory.createTreeFromText(xml_text);

  auto bb = tree.subtrees.front()->blackboard;
  bb->set("battery_ok", true);

  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  ASSERT_TRUE(tracker.hasEvent("GotoKitchen:start")) << tracker.dump();

  bb->set("battery_ok", false);
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);

  EXPECT_EQ(tracker.started_while_running.load(), 0) << tracker.dump();
  EXPECT_LT(tracker.indexOf("GotoKitchen:halted"), tracker.indexOf("GotoCharging:start"))
      << tracker.dump();
}

TEST(Preempt, threadedAction)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Main">
      <ReactiveSequence>
        <ReactiveFallback>
          <BatteryOk/>
          <Preempt>
            <ForeverThreaded name="GotoCharging"/>
          </Preempt>
        </ReactiveFallback>
        <ForeverThreaded name="GotoKitchen"/>
      </ReactiveSequence>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  Tracker tracker;
  registerNodes(factory, tracker);
  auto tree = factory.createTreeFromText(xml_text);

  auto bb = tree.subtrees.front()->blackboard;
  bb->set("battery_ok", true);

  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  ASSERT_TRUE(waitFor([&] { return tracker.hasEvent("GotoKitchen:start"); }))
      << tracker.dump();

  bb->set("battery_ok", false);
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  ASSERT_TRUE(waitFor([&] { return tracker.hasEvent("GotoCharging:start"); }))
      << tracker.dump();

  EXPECT_EQ(tracker.started_while_running.load(), 0) << tracker.dump();
  EXPECT_LT(tracker.indexOf("GotoKitchen:halted"), tracker.indexOf("GotoCharging:start"))
      << tracker.dump();
}

TEST(Preempt, parallelInsideTheNewBranchKeepsItsChildrenRunning)
{
  static const char* xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="Main">
      <ReactiveSequence>
        <ReactiveFallback>
          <BatteryOk/>
          <Preempt>
            <Parallel success_count="2" failure_count="2">
              <Forever name="GotoCharging"/>
              <Forever name="OpenDoor"/>
            </Parallel>
          </Preempt>
        </ReactiveFallback>
        <Forever name="GotoKitchen"/>
      </ReactiveSequence>
    </BehaviorTree>
  </root>
  )";

  BehaviorTreeFactory factory;
  Tracker tracker;
  registerNodes(factory, tracker);
  auto tree = factory.createTreeFromText(xml_text);

  auto bb = tree.subtrees.front()->blackboard;
  bb->set("battery_ok", true);

  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);
  ASSERT_TRUE(tracker.hasEvent("GotoKitchen:start")) << tracker.dump();

  bb->set("battery_ok", false);
  EXPECT_EQ(tree.tickExactlyOnce(), NodeStatus::RUNNING);

  EXPECT_LT(tracker.indexOf("GotoKitchen:halted"), tracker.indexOf("GotoCharging:start"))
      << tracker.dump();
  EXPECT_LT(tracker.indexOf("GotoKitchen:halted"), tracker.indexOf("OpenDoor:start"))
      << tracker.dump();
  EXPECT_EQ(tracker.running.load(), 2) << tracker.dump();
  EXPECT_FALSE(tracker.hasEvent("GotoCharging:halted")) << tracker.dump();
  EXPECT_FALSE(tracker.hasEvent("OpenDoor:halted")) << tracker.dump();
}

}  // namespace BT::test
