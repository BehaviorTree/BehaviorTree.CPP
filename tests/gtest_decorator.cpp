/* Copyright (C) 2018-2019 Davide Faconti, Eurecat - All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "action_test_node.h"
#include "test_helper.hpp"

#include "behaviortree_cpp/bt_factory.h"

#include <gtest/gtest.h>

using BT::NodeStatus;
using std::chrono::milliseconds;

// Timing constants - need generous margins for Windows timer resolution (~15.6ms)
constexpr int DEADLINE_MS = 100;  // Timeout threshold
constexpr auto ACTION_LONG_MS =
    milliseconds(150);  // Action longer than deadline (will timeout)
constexpr auto ACTION_SHORT_MS =
    milliseconds(30);  // Action shorter than deadline (will succeed)

struct DeadlineTest : testing::Test
{
  BT::TimeoutNode root;
  BT::AsyncActionTest action;

  DeadlineTest() : root("deadline", DEADLINE_MS), action("action", ACTION_LONG_MS)
  {
    root.setChild(&action);
  }
  ~DeadlineTest() override = default;
  DeadlineTest(const DeadlineTest&) = delete;
  DeadlineTest& operator=(const DeadlineTest&) = delete;
  DeadlineTest(DeadlineTest&&) = delete;
  DeadlineTest& operator=(DeadlineTest&&) = delete;
};

struct RepeatTest : testing::Test
{
  BT::RepeatNode root;
  BT::SyncActionTest action;

  RepeatTest() : root("repeat", 3), action("action")
  {
    root.setChild(&action);
  }
  ~RepeatTest() override = default;
  RepeatTest(const RepeatTest&) = delete;
  RepeatTest& operator=(const RepeatTest&) = delete;
  RepeatTest(RepeatTest&&) = delete;
  RepeatTest& operator=(RepeatTest&&) = delete;
};

struct RepeatTestAsync : testing::Test
{
  BT::RepeatNode root;
  BT::AsyncActionTest action;

  RepeatTestAsync() : root("repeat", 3), action("action", milliseconds(20))
  {
    root.setChild(&action);
  }
  ~RepeatTestAsync() override = default;
  RepeatTestAsync(const RepeatTestAsync&) = delete;
  RepeatTestAsync& operator=(const RepeatTestAsync&) = delete;
  RepeatTestAsync(RepeatTestAsync&&) = delete;
  RepeatTestAsync& operator=(RepeatTestAsync&&) = delete;
};

struct RetryTest : testing::Test
{
  BT::RetryNode root;
  BT::SyncActionTest action;

  RetryTest() : root("retry", 3), action("action")
  {
    root.setChild(&action);
  }
  ~RetryTest() override = default;
  RetryTest(const RetryTest&) = delete;
  RetryTest& operator=(const RetryTest&) = delete;
  RetryTest(RetryTest&&) = delete;
  RetryTest& operator=(RetryTest&&) = delete;
};

struct TimeoutAndRetry : testing::Test
{
  BT::RetryNode retry;
  BT::TimeoutNode timeout_root;
  BT::SyncActionTest action;

  TimeoutAndRetry() : timeout_root("deadline", 9), retry("retry", 1000), action("action")
  {
    timeout_root.setChild(&retry);
    retry.setChild(&action);
  }
};

/****************TESTS START HERE***************************/

TEST_F(DeadlineTest, DeadlineTriggeredTest)
{
  BT::NodeStatus state = root.executeTick();
  // Action takes longer than deadline, so it should timeout

  ASSERT_EQ(NodeStatus::RUNNING, action.status());
  ASSERT_EQ(NodeStatus::RUNNING, state);

  std::this_thread::sleep_for(ACTION_LONG_MS + milliseconds(20));
  state = root.executeTick();
  ASSERT_EQ(NodeStatus::FAILURE, state);
  ASSERT_EQ(NodeStatus::IDLE, action.status());
}

TEST_F(DeadlineTest, DeadlineNotTriggeredTest)
{
  action.setTime(ACTION_SHORT_MS);
  // Action shorter than deadline, should succeed

  BT::NodeStatus state = root.executeTick();

  ASSERT_EQ(NodeStatus::RUNNING, action.status());
  ASSERT_EQ(NodeStatus::RUNNING, state);

  std::this_thread::sleep_for(ACTION_SHORT_MS + milliseconds(20));
  state = root.executeTick();
  ASSERT_EQ(NodeStatus::IDLE, action.status());
  ASSERT_EQ(NodeStatus::SUCCESS, state);
}

TEST_F(RetryTest, RetryTestA)
{
  action.setExpectedResult(NodeStatus::FAILURE);

  root.executeTick();
  ASSERT_EQ(NodeStatus::FAILURE, root.status());
  ASSERT_EQ(3, action.tickCount());

  action.setExpectedResult(NodeStatus::SUCCESS);
  action.resetTicks();

  root.executeTick();
  ASSERT_EQ(NodeStatus::SUCCESS, root.status());
  ASSERT_EQ(1, action.tickCount());
}

TEST_F(RepeatTestAsync, RepeatTestAsync)
{
  action.setExpectedResult(NodeStatus::SUCCESS);

  auto res = root.executeTick();

  while(res == NodeStatus::RUNNING)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    res = root.executeTick();
  }

  ASSERT_EQ(NodeStatus::SUCCESS, root.status());
  ASSERT_EQ(3, action.successCount());
  ASSERT_EQ(0, action.failureCount());

  //-------------------
  action.setExpectedResult(NodeStatus::FAILURE);
  action.resetCounters();

  res = root.executeTick();
  while(res == NodeStatus::RUNNING)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    res = root.executeTick();
  }

  ASSERT_EQ(NodeStatus::FAILURE, root.status());
  ASSERT_EQ(0, action.successCount());
  ASSERT_EQ(1, action.failureCount());
}

TEST_F(RepeatTest, RepeatTestA)
{
  action.setExpectedResult(NodeStatus::FAILURE);

  root.executeTick();
  ASSERT_EQ(NodeStatus::FAILURE, root.status());
  ASSERT_EQ(1, action.tickCount());

  //-------------------
  action.resetTicks();
  action.setExpectedResult(NodeStatus::SUCCESS);

  root.executeTick();
  ASSERT_EQ(NodeStatus::SUCCESS, root.status());
  ASSERT_EQ(3, action.tickCount());
}

// https://github.com/BehaviorTree/BehaviorTree.CPP/issues/57
TEST_F(TimeoutAndRetry, Issue57)
{
  action.setExpectedResult(NodeStatus::FAILURE);

  auto t1 = std::chrono::high_resolution_clock::now();

  while(std::chrono::high_resolution_clock::now() < t1 + std::chrono::seconds(2))
  {
    ASSERT_NE(timeout_root.executeTick(), BT::NodeStatus::IDLE);
    std::this_thread::sleep_for(std::chrono::microseconds(50));
  }
}

TEST(Decorator, RunOnce)
{
  BT::BehaviorTreeFactory factory;
  std::array<int, 2> counters{};
  RegisterTestTick(factory, "Test", counters);

  const std::string xml_text = R"(
    <root BTCPP_format="4" >
       <BehaviorTree>
          <Sequence>
            <RunOnce> <TestA/> </RunOnce>
            <TestB/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  for(int i = 0; i < 5; i++)
  {
    NodeStatus status = tree.tickWhileRunning();
    ASSERT_EQ(status, NodeStatus::SUCCESS);
  }
  // counters[0] contains the number of times TestA was ticked
  ASSERT_EQ(counters[0], 1);
  // counters[1] contains the number of times TestB was ticked
  ASSERT_EQ(counters[1], 5);
}

// Test for DelayNode with XML: delay_msec port should be honored
TEST(Decorator, DelayWithXML)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4" >
       <BehaviorTree>
          <Delay delay_msec="100">
            <AlwaysSuccess/>
          </Delay>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick should return RUNNING (delay not complete)
  auto start = std::chrono::steady_clock::now();
  NodeStatus status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);

  // Wait for a short time, still should be RUNNING
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);

  // Poll until the delay completes (with timeout for safety)
  while(status == NodeStatus::RUNNING)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    status = tree.tickOnce();
  }
  auto end = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // The child (AlwaysSuccess) should have been executed after the delay
  ASSERT_EQ(status, NodeStatus::SUCCESS);
  // Verify that at least ~200ms have passed (with small tolerance for timing jitter)
  ASSERT_GE(elapsed.count(), 80);
  // Ensure the test didn't take too long (sanity check)
  ASSERT_LE(elapsed.count(), 200);
}

// Tests for ForceFailure decorator
TEST(Decorator, ForceFailure_ChildSuccess)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <ForceFailure>
            <AlwaysSuccess/>
          </ForceFailure>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // ForceFailure should return FAILURE even when child succeeds
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST(Decorator, ForceFailure_ChildFailure)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <ForceFailure>
            <AlwaysFailure/>
          </ForceFailure>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // ForceFailure should return FAILURE when child fails
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

// Tests for ForceSuccess decorator (for completeness)
TEST(Decorator, ForceSuccess_ChildFailure)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <ForceSuccess>
            <AlwaysFailure/>
          </ForceSuccess>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // ForceSuccess should return SUCCESS even when child fails
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(Decorator, ForceSuccess_ChildSuccess)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <ForceSuccess>
            <AlwaysSuccess/>
          </ForceSuccess>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // ForceSuccess should return SUCCESS when child succeeds
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

// Tests for Inverter decorator
TEST(Decorator, Inverter_ChildSuccess)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Inverter>
            <AlwaysSuccess/>
          </Inverter>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // Inverter should return FAILURE when child succeeds
  ASSERT_EQ(status, NodeStatus::FAILURE);
}

TEST(Decorator, Inverter_ChildFailure)
{
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Inverter>
            <AlwaysFailure/>
          </Inverter>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // Inverter should return SUCCESS when child fails
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

TEST(Decorator, Inverter_InSequence)
{
  // Test Inverter behavior within a sequence
  BT::BehaviorTreeFactory factory;

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <Sequence>
            <Inverter>
              <AlwaysFailure/>
            </Inverter>
            <AlwaysSuccess/>
          </Sequence>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);
  auto status = tree.tickWhileRunning();

  // Inverter converts FAILURE to SUCCESS, so sequence continues and succeeds
  ASSERT_EQ(status, NodeStatus::SUCCESS);
}

// Test KeepRunningUntilFailure decorator
TEST(Decorator, KeepRunningUntilFailure)
{
  BT::BehaviorTreeFactory factory;

  int tick_count = 0;
  factory.registerSimpleAction("SuccessThenFail", [&tick_count](BT::TreeNode&) {
    tick_count++;
    if(tick_count < 3)
    {
      return NodeStatus::SUCCESS;
    }
    return NodeStatus::FAILURE;
  });

  const std::string xml_text = R"(
    <root BTCPP_format="4">
       <BehaviorTree>
          <KeepRunningUntilFailure>
            <SuccessThenFail/>
          </KeepRunningUntilFailure>
       </BehaviorTree>
    </root>)";

  auto tree = factory.createTreeFromText(xml_text);

  // First tick - child succeeds, should return RUNNING
  auto status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);
  ASSERT_EQ(tick_count, 1);

  // Second tick - child succeeds again, should return RUNNING
  status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::RUNNING);
  ASSERT_EQ(tick_count, 2);

  // Third tick - child fails, should return FAILURE
  status = tree.tickOnce();
  ASSERT_EQ(status, NodeStatus::FAILURE);
  ASSERT_EQ(tick_count, 3);
}
