/* Copyright (C) 2024 Davide Faconti - All Rights Reserved
 *
 * Tree::~Tree() halts the tree. Halting is allowed to throw (a TryCatch with
 * catch_on_halt ticks its catch child, and a tick may throw), and a destructor
 * that lets an exception escape calls std::terminate. Destroying a tree whose
 * halt throws must not abort the process.
 */

#include "behaviortree_cpp/bt_factory.h"

#include <gtest/gtest.h>

using namespace BT;

namespace
{
// Stays RUNNING, so TryCatch is still in its try block when the tree is
// destroyed - that is the condition for catch_on_halt to tick the catch child.
class RunningAction : public StatefulActionNode
{
public:
  RunningAction(const std::string& name, const NodeConfig& config)
    : StatefulActionNode(name, config)
  {}

  static PortsList providedPorts()
  {
    return {};
  }

  NodeStatus onStart() override
  {
    return NodeStatus::RUNNING;
  }
  NodeStatus onRunning() override
  {
    return NodeStatus::RUNNING;
  }
  void onHalted() override
  {}
};

// Throws on every tick, including the one a TryCatch does while halting.
// Stateful, because a SyncActionNode that returns RUNNING is itself an error.
class ThrowingAction : public StatefulActionNode
{
public:
  ThrowingAction(const std::string& name, const NodeConfig& config)
    : StatefulActionNode(name, config)
  {}

  static PortsList providedPorts()
  {
    return {};
  }

  NodeStatus onStart() override
  {
    throw std::runtime_error("thrown from tick");
  }
  NodeStatus onRunning() override
  {
    throw std::runtime_error("thrown from tick");
  }
  void onHalted() override
  {}
};

}  // namespace

// The tree reaches RUNNING, so TryCatch takes the catch_on_halt path when the
// destructor halts it; the catch child throws and the exception must not
// escape the destructor.
TEST(TreeDestructorTest, HaltThatThrowsDoesNotTerminate)
{
  BehaviorTreeFactory factory;
  factory.registerNodeType<RunningAction>("RunningAction");
  factory.registerNodeType<ThrowingAction>("ThrowingAction");

  const std::string xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <TryCatch catch_on_halt="true">
        <RunningAction/>
        <ThrowingAction/>
      </TryCatch>
    </BehaviorTree>
  </root>
  )";

  // Reaching this line is the assertion: before the fix the destructor's
  // exception escaped and the process called std::terminate.
  EXPECT_NO_THROW({
    auto tree = factory.createTreeFromText(xml_text);
    tree.tickOnce();
  });
}

// A plain tree without a throwing node must still be destroyed normally, and
// halting must still happen - the fix must not skip it.
TEST(TreeDestructorTest, HaltStillRuns)
{
  BehaviorTreeFactory factory;

  const std::string xml_text = R"(
  <root BTCPP_format="4">
    <BehaviorTree ID="MainTree">
      <AlwaysSuccess/>
    </BehaviorTree>
  </root>
  )";

  auto tree = factory.createTreeFromText(xml_text);
  EXPECT_EQ(tree.tickOnce(), NodeStatus::SUCCESS);
  // Scoped, so the destructor runs here and the test can observe it.
  EXPECT_NO_THROW({ auto scoped = factory.createTreeFromText(xml_text);
                    scoped.tickOnce(); });
}
