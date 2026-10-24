#include "behaviortree_cpp/bt_factory.h"

#include <chrono>
#include <stdexcept>

#include <gtest/gtest.h>

using BT::NodeStatus;

struct TestError : std::runtime_error
{
  using std::runtime_error::runtime_error;
};

// RUNNING until halted. Optionally throws on its second tick, or from onHalted().
class AsyncMain : public BT::StatefulActionNode
{
public:
  AsyncMain(const std::string& name, const BT::NodeConfig& config, int* halted,
            bool throw_on_running, bool throw_on_halt)
    : StatefulActionNode(name, config)
    , halted_(halted)
    , throw_(throw_on_running)
    , throw_on_halt_(throw_on_halt)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  NodeStatus onStart() override
  {
    return NodeStatus::RUNNING;
  }
  NodeStatus onRunning() override
  {
    if(throw_)
    {
      throw TestError("boom");
    }
    return NodeStatus::RUNNING;
  }
  void onHalted() override
  {
    (*halted_)++;
    if(throw_on_halt_)
    {
      throw TestError("halt boom");
    }
  }

private:
  int* halted_;
  bool throw_;
  bool throw_on_halt_;
};

// RUNNING on start, SUCCESS on the next tick. Counts how often it starts.
class TwoTickMain : public BT::StatefulActionNode
{
public:
  TwoTickMain(const std::string& name, const BT::NodeConfig& config, int* starts)
    : StatefulActionNode(name, config), starts_(starts)
  {}
  static BT::PortsList providedPorts()
  {
    return {};
  }
  NodeStatus onStart() override
  {
    (*starts_)++;
    return NodeStatus::RUNNING;
  }
  NodeStatus onRunning() override
  {
    return NodeStatus::SUCCESS;
  }
  void onHalted() override
  {}

private:
  int* starts_;
};

class FinallyTest : public testing::Test
{
protected:
  BT::BehaviorTreeFactory factory;
  int cleanup_count = 0;
  int main_ticks = 0;
  int main_halted = 0;
  int cleanup_ticks = 0;
  int throw_once_ticks = 0;
  int spin_ticks = 0;
  int two_tick_starts = 0;
  int second_tick_ticks = 0;
  bool gate = true;

  void SetUp() override
  {
    factory.registerSimpleAction("Cleanup", [this](BT::TreeNode&) {
      cleanup_count++;
      return NodeStatus::SUCCESS;
    });
    factory.registerSimpleAction(
        "Throw", [](BT::TreeNode&) -> NodeStatus { throw TestError("boom"); });
    // RUNNING twice, then SUCCESS
    factory.registerSimpleCondition("RunTwice", [this](BT::TreeNode&) {
      return ++main_ticks < 3 ? NodeStatus::RUNNING : NodeStatus::SUCCESS;
    });
    factory.registerNodeType<AsyncMain>("AsyncMain", &main_halted, false, false);
    factory.registerNodeType<AsyncMain>("AsyncThrow", &main_halted, true, false);
    factory.registerNodeType<AsyncMain>("AsyncHaltThrows", &main_halted, false, true);
    factory.registerNodeType<AsyncMain>("AsyncThrowHaltThrows", &main_halted, true, true);
    factory.registerSimpleAction("ThrowInt",
                                 [](BT::TreeNode&) -> NodeStatus { throw 1; });
    factory.registerSimpleAction("ThrowOnce", [this](BT::TreeNode&) {
      if(++throw_once_ticks == 1)
      {
        throw TestError("once");
      }
      return NodeStatus::SUCCESS;
    });
    factory.registerNodeType<TwoTickMain>("TwoTickMain", &two_tick_starts);
    factory.registerSimpleCondition("Spin", [this](BT::TreeNode&) {
      spin_ticks++;
      return NodeStatus::RUNNING;
    });
    // RUNNING on its first tick, then throws on its second
    factory.registerSimpleCondition("RunThenThrow", [this](BT::TreeNode&) -> NodeStatus {
      if(++second_tick_ticks < 2)
      {
        return NodeStatus::RUNNING;
      }
      throw TestError("late boom");
    });
    // RUNNING on its first tick, then FAILURE
    factory.registerSimpleCondition("RunThenFail", [this](BT::TreeNode&) {
      return ++second_tick_ticks < 2 ? NodeStatus::RUNNING : NodeStatus::FAILURE;
    });
    factory.registerSimpleCondition("Gate", [this](BT::TreeNode&) {
      return gate ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    });
    // Cleanup that is RUNNING on its first tick, then SUCCESS
    factory.registerSimpleCondition("AsyncCleanup", [this](BT::TreeNode&) {
      return ++cleanup_ticks < 2 ? NodeStatus::RUNNING : NodeStatus::SUCCESS;
    });
    factory.registerSimpleCondition("AlwaysRunning", [this](BT::TreeNode&) {
      main_ticks++;
      return NodeStatus::RUNNING;
    });
  }

  NodeStatus run(const std::string& main, const std::string& cleanup = "<Cleanup/>")
  {
    auto tree =
        factory.createTreeFromText(R"(<root BTCPP_format="4"><BehaviorTree><Finally>)" +
                                   main + cleanup + "</Finally></BehaviorTree></root>");
    return tree.tickWhileRunning();
  }
};

TEST_F(FinallyTest, MainSucceeds_CleanupRuns)
{
  EXPECT_EQ(run("<AlwaysSuccess/>"), NodeStatus::SUCCESS);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, MainFails_CleanupRuns)
{
  EXPECT_EQ(run("<AlwaysFailure/>"), NodeStatus::FAILURE);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, CleanupFails_ReturnsFailure)
{
  EXPECT_EQ(run("<AlwaysSuccess/>", "<AlwaysFailure/>"), NodeStatus::FAILURE);
}

TEST_F(FinallyTest, MainThrows_CleanupRunsThenRethrows)
{
  EXPECT_THROW(run("<Throw/>"), BT::RuntimeError);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, NestedMainThrows_CleanupRunsThenRethrows)
{
  EXPECT_THROW(run("<Sequence><AlwaysSuccess/><Throw/></Sequence>"), BT::RuntimeError);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, CleanupThrows_Propagates)
{
  EXPECT_THROW(run("<AlwaysSuccess/>", "<Throw/>"), BT::RuntimeError);
}

TEST_F(FinallyTest, AsyncMain_CleanupRunsOnceAfterMainCompletes)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><RunTwice/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_EQ(cleanup_count, 0);
  EXPECT_EQ(tree.tickOnce(), NodeStatus::SUCCESS);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, HaltWhileMainRunning_CleanupRuns)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysRunning/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(cleanup_count, 1);
  EXPECT_EQ(tree.rootNode()->status(), NodeStatus::IDLE);

  // A second halt on an idle node does not run cleanup again.
  tree.haltTree();
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, TickedAgain_CleanupRunsEachTime)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysSuccess/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickWhileRunning(), NodeStatus::SUCCESS);
  EXPECT_EQ(tree.tickWhileRunning(), NodeStatus::SUCCESS);
  EXPECT_EQ(cleanup_count, 2);
}

TEST_F(FinallyTest, WrongChildCount_RejectedAtLoad)
{
  EXPECT_THROW((void)factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysSuccess/></Finally>
    </BehaviorTree></root>)"),
               BT::RuntimeError);
  EXPECT_THROW((void)factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysSuccess/><Cleanup/><Cleanup/></Finally>
    </BehaviorTree></root>)"),
               BT::RuntimeError);
}

TEST_F(FinallyTest, MainSkipped_CleanupRunsAndReturnsSkipped)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysSuccess _skipIf="true"/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickWhileRunning(), NodeStatus::SKIPPED);
  EXPECT_EQ(cleanup_count, 1);
  // The node must not be left RUNNING, or this halt would run cleanup again.
  tree.haltTree();
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, AsyncCleanup_ReturnsMainStatusWhenCleanupCompletes)
{
  EXPECT_EQ(run("<AlwaysFailure/>", "<AsyncCleanup/>"), NodeStatus::FAILURE);
  EXPECT_EQ(cleanup_ticks, 2);
}

TEST_F(FinallyTest, AsyncMainThrows_MainHaltedAndCleanupRuns)
{
  EXPECT_THROW(run("<AsyncThrow/>"), BT::RuntimeError);
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, HaltWhileMainRunning_MainHalted)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AsyncMain/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, HaltWhileCleanupRunning_CleanupFinishes)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysSuccess/><AsyncCleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(cleanup_ticks, 2);
  EXPECT_EQ(tree.rootNode()->status(), NodeStatus::IDLE);
}

TEST_F(FinallyTest, HaltWhileMainRunning_AsyncCleanupFinishes)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysRunning/><AsyncCleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(cleanup_ticks, 2);
  EXPECT_EQ(tree.rootNode()->status(), NodeStatus::IDLE);
}

TEST_F(FinallyTest, HaltWhileMainRunning_EveryStepOfAsyncCleanupRuns)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally>
        <AlwaysRunning/>
        <Sequence><AsyncCleanup/><Cleanup/></Sequence>
      </Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(cleanup_ticks, 2);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, HaltWithCleanupThatNeverFinishes_HaltsItAfterTimeout)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally halt_timeout_msec="50"><AlwaysSuccess/><Spin/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  const auto start = std::chrono::steady_clock::now();
  tree.haltTree();
  const auto elapsed = std::chrono::steady_clock::now() - start;
  // The wait stops before a tick that would start after the timeout, up to one 10 ms period early.
  EXPECT_GE(elapsed, std::chrono::milliseconds(40));
  EXPECT_LT(elapsed, std::chrono::seconds(5));
  // At least one tick from the run and one more during the halt.
  EXPECT_GE(spin_ticks, 2);
  EXPECT_EQ(tree.rootNode()->status(), NodeStatus::IDLE);
}

TEST_F(FinallyTest, CleanupThrowsDuringHalt_DoesNotPropagate)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysRunning/><Throw/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_NO_THROW(tree.haltTree());
  EXPECT_EQ(tree.rootNode()->status(), NodeStatus::IDLE);
}

TEST_F(FinallyTest, MainThrowsAndItsHaltThrows_CleanupStillRuns)
{
  EXPECT_THROW(run("<AsyncThrowHaltThrows/>"), BT::RuntimeError);
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, HaltWhereMainHaltThrows_CleanupRunsAndNothingPropagates)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AsyncHaltThrows/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_NO_THROW(tree.haltTree());
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, MainThrowsNonStdException_CleanupRunsThenRethrows)
{
  EXPECT_THROW(run("<ThrowInt/>"), int);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, CleanupThrowsNonStdExceptionDuringHalt_DoesNotPropagate)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysRunning/><ThrowInt/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_NO_THROW(tree.haltTree());
}

TEST_F(FinallyTest, CleanupThrows_EndsTheRunAndHaltDoesNotRetry)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><RunTwice/><ThrowOnce/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_THROW(tree.tickOnce(), BT::RuntimeError);
  tree.haltTree();
  EXPECT_EQ(throw_once_ticks, 1);

  // The next tick starts again with main.
  EXPECT_EQ(tree.tickOnce(), NodeStatus::SUCCESS);
  EXPECT_EQ(main_ticks, 4);
  EXPECT_EQ(throw_once_ticks, 2);
}

TEST_F(FinallyTest, CleanupSkipped_ReturnsMainStatus)
{
  EXPECT_EQ(run("<AlwaysFailure/>", R"(<AlwaysSuccess _skipIf="true"/>)"),
            NodeStatus::FAILURE);
}

TEST_F(FinallyTest, HaltedByReactiveSequence_CleanupRunsOnce)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <ReactiveSequence>
        <Gate/>
        <Finally><AsyncMain/><Cleanup/></Finally>
      </ReactiveSequence>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_EQ(cleanup_count, 0);
  gate = false;
  EXPECT_EQ(tree.tickOnce(), NodeStatus::FAILURE);
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 1);
  tree.haltTree();
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, InsideHaltedSubTree_CleanupRuns)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4" main_tree_to_execute="Main">
      <BehaviorTree ID="Main"><SubTree ID="Inner"/></BehaviorTree>
      <BehaviorTree ID="Inner">
        <Finally><AsyncMain/><Cleanup/></Finally>
      </BehaviorTree>
    </root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, NestedFinallyAsCleanup_RunsDuringOuterHalt)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally>
        <AsyncMain/>
        <Finally><Cleanup/><Cleanup/></Finally>
      </Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 2);
}

TEST_F(FinallyTest, NestedFinallyAsMain_BothCleanupsRunOnHalt)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally>
        <Finally><AsyncMain/><Cleanup/></Finally>
        <Cleanup/>
      </Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  tree.haltTree();
  EXPECT_EQ(main_halted, 1);
  EXPECT_EQ(cleanup_count, 2);
}

TEST_F(FinallyTest, CleanupFailsDuringHalt_IsReported)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysRunning/><AlwaysFailure/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  testing::internal::CaptureStderr();
  tree.haltTree();
  EXPECT_NE(testing::internal::GetCapturedStderr().find("cleanup returned FAILURE during "
                                                        "halt"),
            std::string::npos);
}

TEST_F(FinallyTest, MainThrows_HaltAfterRethrowDoesNotRerunCleanup)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><Throw/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_THROW(tree.tickOnce(), BT::RuntimeError);
  tree.haltTree();
  EXPECT_EQ(cleanup_count, 1);
}

TEST_F(FinallyTest, MainThrowsWithAsyncCleanup_RethrowsWhenCleanupCompletes)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><Throw/><AsyncCleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_THROW(tree.tickOnce(), BT::RuntimeError);
  EXPECT_EQ(cleanup_ticks, 2);
}

TEST_F(FinallyTest, MainThrows_NextTickStartsFresh)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><ThrowOnce/><Cleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_THROW(tree.tickOnce(), BT::RuntimeError);
  EXPECT_EQ(tree.tickOnce(), NodeStatus::SUCCESS);
  EXPECT_EQ(throw_once_ticks, 2);
  EXPECT_EQ(cleanup_count, 2);
}

TEST_F(FinallyTest, CleanupThrowsAfterRunningDuringHalt_DoesNotPropagate)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysRunning/><RunThenThrow/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_NO_THROW(tree.haltTree());
  EXPECT_EQ(second_tick_ticks, 2);
  EXPECT_EQ(tree.rootNode()->status(), NodeStatus::IDLE);
}

TEST_F(FinallyTest, CleanupFailsAfterRunningDuringHalt_IsReported)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><AlwaysRunning/><RunThenFail/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  testing::internal::CaptureStderr();
  tree.haltTree();
  EXPECT_NE(testing::internal::GetCapturedStderr().find("cleanup returned FAILURE during "
                                                        "halt"),
            std::string::npos);
  EXPECT_EQ(second_tick_ticks, 2);
}

TEST_F(FinallyTest, UnreadableHaltTimeout_FallsBackToDefault)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally halt_timeout_msec="{missing}"><AlwaysRunning/><AsyncCleanup/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  testing::internal::CaptureStderr();
  tree.haltTree();
  EXPECT_NE(testing::internal::GetCapturedStderr().find("cannot read halt_timeout_msec"),
            std::string::npos);
  EXPECT_EQ(cleanup_ticks, 2);
}

TEST_F(FinallyTest, CleanupThrows_StatefulMainRestartsOnNextTick)
{
  auto tree = factory.createTreeFromText(R"(
    <root BTCPP_format="4"><BehaviorTree>
      <Finally><TwoTickMain/><ThrowOnce/></Finally>
    </BehaviorTree></root>)");

  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_THROW(tree.tickOnce(), BT::RuntimeError);

  // Main was reset with the node, so it starts again rather than reporting its old result.
  EXPECT_EQ(tree.tickOnce(), NodeStatus::RUNNING);
  EXPECT_EQ(two_tick_starts, 2);
  EXPECT_EQ(tree.tickOnce(), NodeStatus::SUCCESS);
}
