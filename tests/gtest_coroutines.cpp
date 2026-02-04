#include "behaviortree_cpp/behavior_tree.h"
#include "behaviortree_cpp/decorators/timeout_node.h"

#include <chrono>
#include <future>

#include <gtest/gtest.h>

using namespace std::chrono;

using Millisecond = std::chrono::milliseconds;
using Timepoint = std::chrono::time_point<std::chrono::steady_clock>;

// Timing constants for coroutine tests
// Keep durations short for fast test execution while maintaining reliable relative timing
constexpr auto SHORT_ACTION_DURATION = milliseconds(10);  // Quick action for success path
constexpr auto MEDIUM_ACTION_DURATION = milliseconds(20);  // Medium action duration
constexpr auto LONG_ACTION_DURATION = milliseconds(50);  // Longer action that may timeout
constexpr auto TIMEOUT_DURATION =
    milliseconds(30);  // Timeout threshold (between short and long)
constexpr auto SEQUENCE_TIMEOUT =
    milliseconds(35);  // Timeout for sequence (allows 1 action, not 2)

class SimpleCoroAction : public BT::CoroActionNode
{
public:
  SimpleCoroAction(milliseconds timeout, bool will_fail, const std::string& node_name,
                   const BT::NodeConfig& config)
    : BT::CoroActionNode(node_name, config)
    , will_fail_(will_fail)
    , timeout_(timeout)
    , start_time_(Timepoint::min())
  {}

  virtual void halt() override
  {
    // Cleanup when halted
    start_time_ = Timepoint::min();
    halted_ = true;
    BT::CoroActionNode::halt();
  }

  bool wasHalted()
  {
    return halted_;
  }

  void setRequiredTime(Millisecond ms)
  {
    timeout_ = ms;
  }

protected:
  virtual BT::NodeStatus tick() override
  {
    halted_ = false;

    if(start_time_ == Timepoint::min())
    {
      start_time_ = std::chrono::steady_clock::now();
    }

    while(std::chrono::steady_clock::now() < (start_time_ + timeout_))
    {
      setStatusRunningAndYield();
    }

    halted_ = false;
    start_time_ = Timepoint::min();
    return (will_fail_ ? BT::NodeStatus::FAILURE : BT::NodeStatus::SUCCESS);
  }

public:
  bool will_fail_;

private:
  std::chrono::milliseconds timeout_;
  Timepoint start_time_;
  bool halted_ = false;
};

namespace
{
BT::NodeStatus executeWhileRunning(BT::TreeNode& node)
{
  auto status = node.executeTick();
  while(status == BT::NodeStatus::RUNNING)
  {
    status = node.executeTick();
    std::this_thread::sleep_for(Millisecond(1));
  }
  return status;
}
}  // namespace

TEST(CoroTest, do_action)
{
  BT::NodeConfig node_config_;
  node_config_.blackboard = BT::Blackboard::create();
  BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);
  SimpleCoroAction node(MEDIUM_ACTION_DURATION, false, "Action", node_config_);

  EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(node));
  EXPECT_FALSE(node.wasHalted());

  EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(node)) << "Second call to coro "
                                                                   "action";
  EXPECT_FALSE(node.wasHalted());

  node.will_fail_ = true;
  EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(node)) << "Should execute again "
                                                                   "and return failure";
  EXPECT_FALSE(node.wasHalted());

  EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(node)) << "Shouldn't fail "
                                                                   "because we set "
                                                                   "status to idle";
  EXPECT_FALSE(node.wasHalted());
}

TEST(CoroTest, do_action_timeout)
{
  BT::NodeConfig node_config_;
  node_config_.blackboard = BT::Blackboard::create();
  BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);

  // Action takes longer than timeout -> should fail
  SimpleCoroAction node(LONG_ACTION_DURATION, false, "Action", node_config_);
  BT::TimeoutNode timeout("TimeoutAction", TIMEOUT_DURATION.count());

  timeout.setChild(&node);

  EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(timeout)) << "should timeout";
  EXPECT_TRUE(node.wasHalted());

  // Action takes less than timeout -> should succeed
  node.setRequiredTime(SHORT_ACTION_DURATION);

  EXPECT_EQ(BT::NodeStatus::SUCCESS, executeWhileRunning(timeout));
  EXPECT_FALSE(node.wasHalted());
}

TEST(CoroTest, sequence_child)
{
  BT::NodeConfig node_config_;
  node_config_.blackboard = BT::Blackboard::create();
  BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);

  // Two actions each taking MEDIUM_ACTION_DURATION, but timeout only allows ~1.8x that
  // First action completes, second gets halted by timeout
  SimpleCoroAction actionA(MEDIUM_ACTION_DURATION, false, "action_A", node_config_);
  SimpleCoroAction actionB(MEDIUM_ACTION_DURATION, false, "action_B", node_config_);
  BT::TimeoutNode timeout("timeout", SEQUENCE_TIMEOUT.count());
  BT::SequenceNode sequence("sequence");

  timeout.setChild(&sequence);
  sequence.addChild(&actionA);
  sequence.addChild(&actionB);

  EXPECT_EQ(BT::NodeStatus::FAILURE, executeWhileRunning(timeout)) << "should timeout";
  EXPECT_FALSE(actionA.wasHalted());
  EXPECT_TRUE(actionB.wasHalted());
}

TEST(CoroTest, OtherThreadHalt)
{
  BT::NodeConfig node_config_;
  node_config_.blackboard = BT::Blackboard::create();
  BT::assignDefaultRemapping<SimpleCoroAction>(node_config_);

  SimpleCoroAction actionA(LONG_ACTION_DURATION, false, "action_A", node_config_);
  actionA.executeTick();

  auto handle = std::async(std::launch::async, [&]() { actionA.halt(); });
  handle.wait();
  EXPECT_TRUE(actionA.wasHalted());

  handle = std::async(std::launch::async, [&]() { actionA.executeTick(); });
  handle.wait();
}
